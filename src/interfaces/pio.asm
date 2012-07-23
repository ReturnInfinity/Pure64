; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2012 Return Infinity -- see LICENSE.TXT
;
; INIT HDD
; =============================================================================

; The code below only utilizes the Master drive on the Primary ATA Bus.
; Port IO is hard-coded to Primary : 0x01F0 - 0x01F7, 0x03F6
; Secondary Bus would be 0x0170 - 0x0177, 0x0376

hdd_setup:
; Probe for PATA hard drive
	mov dx, 0x01F0
	mov [ata_base], dx
	add dx, 7			; Primary ATA Regular Status Byte
	in al, dx
	cmp al, 0xFF			; Check for "float" value
	je hdd_setup_try_sata		; No drive detected
	test al, 0xA9			; Is ERR, DRQ, DF, or BSY set?
	jne hdd_setup_err_read
	jmp hdd_setup_size

; Probe for a hard drive controller
hdd_setup_try_sata:
	xor ebx, ebx
	xor ecx, ecx
findcontroller:
	cmp bx, 256			; Search up to 256 buses
	je hdd_setup_err_drive		; No drive detected
	cmp cl, 32			; Up to 32 devices per bus
	jne findcontroller_1
	add bx, 1
	xor ecx, ecx
findcontroller_1:
	mov dl, 2			; We want the Class/Device code
	call os_pci_read_reg
	add cl, 1
	shr rax, 16
	cmp ax, 0x0106			; Mass storage device, SATA
	jne findcontroller
	sub cl, 1
	mov dl, 9
	call os_pci_read_reg		; BAR5 (AHCI Base Address Register)
	mov [sata_base], eax
	call os_debug_dump_eax
	call os_print_newline
	mov rsi, rax
	lodsd
	call os_debug_dump_eax
	call os_print_newline
	bt rax, 18
	jnc legacy_mode_ok
	push rsi
	mov rsi, no_sata_legacy
	call os_print_string
	pop rsi
	jmp $
legacy_mode_ok:
	mov rdi, rsi
	lodsd
	call os_debug_dump_eax
	call os_print_newline
	xor eax, eax
	stosd

;jmp $

	mov dl, 4			; BAR0
	call os_pci_read_reg
	and ax, 0xFFFC			; AX now holds the Base IO Address (clear the low 2 bits)
;   call os_debug_dump_eax
	mov dx, ax
	mov [ata_base], dx
	in al, dx
	cmp al, 0xFF			; Check for "float" value
	je hdd_setup_err_drive		; No drive detected
	test al, 0xA9			; Is ERR, DRQ, DF, or BSY set?
	jne hdd_setup_err_read

hdd_setup_size:
	; Calculate the size in MiB
	mov rdi, 0x200000
	call iddrive
	mov rsi, 0x200000

	mov rax, [rsi+200]		; Max LBA48 sectors, val in word 100-103
	cmp rax, 0
	jnz hdd_got_size

	; no value for max LBA48 sectors, grab LBA28 from word 60-61
	mov eax, [rsi+120]

hdd_got_size:
	shr rax, 11			; rax = rax * 512 / 1048576 	MiB
;	shr rax, 21			; rax = rax * 512 / 1073741824	GiB
	mov [hd1_size], eax		; in mebibytes (MiB)
	mov rdi, hdtempstring
	call os_int_to_string

	; Found a bootable drive
	mov byte [cfg_hdd], 0x01
ret

hdd_setup_err_drive:
	mov rsi, hdd_setup_no_disk
	call os_print_string
	jmp exception_gate_halt

hdd_setup_err_read:
	mov rsi, hdd_setup_read_error
	call os_print_string
	jmp exception_gate_halt

; -----------------------------------------------------------------------------
; readsectors -- Read sectors on the hard drive
; IN:   RAX = starting sector to read
;   RCX = number of sectors to read (1 - 256)
;   RDI = memory location to store sectors
; OUT:  RAX = RAX + number of sectors that were read
;   RCX = number of sectors that were read (0 on error)
;   RDI = RDI + (number of sectors * 512)
;   All other registers preserved
readsectors:
	push rdx
	push rcx
	push rbx
	push rax

	push rcx			; Save RCX for use in the read loop
	mov rbx, rcx			; Store number of sectors to read
	cmp rcx, 0
	je readsectors_fail		; Try to read nothing? Fail!
	cmp rcx, 256
	jg readsectors_fail		; Over 256? Fail!
	jne readsectors_skip		; Not 256? No need to modify CL
	xor rcx, rcx			; 0 translates to 256
readsectors_skip:

	push rax			; Save RAX since we are about to overwrite it
	mov dx, [ata_base]
	;mov dx, 0x01F2			; 0x01F2 - Sector count Port 7:0
	add dx, 2
	mov al, cl			; Read CL sectors
	out dx, al
	pop rax				; Restore RAX which is our sector number
	inc dx				; 0x01F3 - LBA Low Port 7:0
	out dx, al
	inc dx				; 0x01F4 - LBA Mid Port 15:8
	shr rax, 8
	out dx, al
	inc dx				; 0x01F5 - LBA High Port 23:16
	shr rax, 8
	out dx, al
	inc dx				; 0x01F6 - Device Port. Bit 6 set for LBA mode,
					; Bit 4 for device (0 = master, 1 = slave),
					; Bits 3-0 for LBA "Extra High" (27:24)
	shr rax, 8
	and al, 00001111b		; Clear bits 4-7 just to be safe
	or al, 01000000b		; Turn bit 6 on since we want to use
					; LBA addressing, leave device at 0 (master)
	out dx, al
	inc dx				; 0x01F7 - Command Port
	mov al, 0x20			; Read sector(s). 0x24 if LBA48
	out dx, al

	mov rcx, 4
readsectors_wait:
	in al, dx			; Read status from 0x01F7
	test al, 0x80			; BSY flag set?
	jne readsectors_retry
	test al, 0x08			; DRQ set?
	jne readsectors_dataready
readsectors_retry:
	dec rcx
	jg readsectors_wait
readsectors_nextsector:
	in al, dx			; Read status from 0x01F7
	test al, 0x80			; BSY flag set?
	jne readsectors_nextsector
	test al, 0x21			; ERR or DF set?
	jne readsectors_fail

readsectors_dataready:
	sub dx, 7			; Data port (0x1F0)
	mov rcx, 256			; Read
	rep insw			; Copy a 512 byte sector to RDI
	add dx, 7			; Set DX back to status register (0x01F7)
	in al, dx			; Delay ~400ns to allow drive to set
					; new values of BSY and DRQ
	in al, dx
	in al, dx
	in al, dx

	dec rbx				; RBX is the "sectors to read" counter
	cmp rbx, 0
	jne readsectors_nextsector

	test al, 0x21			; ERR or DF set?
	jne readsectors_fail

	pop rcx
	pop rax
	pop rbx
	add rax, rcx
	pop rcx
	pop rdx
ret

readsectors_fail:
	pop rcx
	pop rax
	pop rbx
	pop rcx
	pop rdx
	xor rcx, rcx			; Set RCX to 0 since nothing was read
ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; iddrive -- Identify an ATA drive
; IN:	RDI = memory location to store details (512 bytes)
; OUT:	Nothing, all registers preserved
iddrive:
	push rdx
	push rcx
	push rbx
	push rax

iddrive_skip:
	mov dx, [ata_base]
	add dx, 2			; 0x01F2 - Sector count Port 7:0
	mov al, 0
	out dx, al
	inc dx				; 0x01F3 - LBA Low Port 7:0
	out dx, al
	inc dx				; 0x01F4 - LBA Mid Port 15:8
	out dx, al
	inc dx				; 0x01F5 - LBA High Port 23:16
	out dx, al
	inc dx				; 0x01F6 - Device Port. 0xA0 for master,
					; 0xB0 for slave
	mov al, 0xA0
	out dx, al
	inc dx				; 0x01F7 - Command Port
	mov al, 0xEC
	out dx, al			; IDENTIFY

	mov rcx, 4
iddrive_wait:
	in al, dx			; Read status from 0x01F7
	test al, 0x80			; BSY flag set?
	jne iddrive_retry
	test al, 0x08			; DRQ set?
	jne iddrive_dataready
iddrive_retry:
	dec rcx
	jg iddrive_wait

iddrive_dataready:
	sub dx, 7			; Data port (0x1F0)
	mov rcx, 128			; Read
	rep insw			; Copy the 256-byte ID structure to rdi
	add dx, 7			; Set DX back to status register (0x01F7)
	in al, dx			; Delay ~400ns to allow drive to set
					; new values of BSY and DRQ
	in al, dx
	in al, dx
	in al, dx

	test al, 0x21			; ERR or DF set?
	jne iddrive_fail

	pop rax
	pop rbx
	pop rcx
	pop rdx
ret

iddrive_fail:
	pop rax
	pop rbx
	pop rcx
	pop rdx
ret
; -----------------------------------------------------------------------------


no_sata_legacy: db "SATA Controller only suppports native mode!", 13, 0

; =============================================================================
; EOF
