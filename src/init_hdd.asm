; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2011 Return Infinity -- see LICENSE.TXT
;
; INIT HDD
; =============================================================================


hdd_setup:
; Read first sector of HDD into memory
	xor rax, rax
	mov rdi, hdbuffer
	push rdi
	mov rcx, 1
	call readsectors
	pop rdi

	cmp byte [cfg_mbr], 0x01	; Did we boot from a MBR drive
	jne hdd_setup_no_mbr		; If not then we already have the correct sector

; Grab the partition offset value for the first partition
	mov eax, [rdi+0x01C6]
	mov [fat16_PartitionOffset], eax

; Read the first sector of the first partition
	mov rdi, hdbuffer
	push rdi
	mov rcx, 1
	call readsectors
	pop rdi	

hdd_setup_no_mbr:
; Get the values we need to start using fat16
	mov ax, [rdi+0x0b]
	mov [fat16_BytesPerSector], ax	; This will probably be 512
	mov al, [rdi+0x0d]
	mov [fat16_SectorsPerCluster], al	; This will be 128 or less (Max cluster size is 64KiB)
	mov ax, [rdi+0x0e]
	mov [fat16_ReservedSectors], ax
	mov [fat16_FatStart], eax
	mov al, [rdi+0x10]
	mov [fat16_Fats], al		; This will probably be 2
	mov ax, [rdi+0x11]
	mov [fat16_RootDirEnts], ax
	mov ax, [rdi+0x16]
	mov [fat16_SectorsPerFat], ax

; Find out how many sectors are on the disk
	xor eax, eax
	mov ax, [rdi+0x13]
	cmp ax, 0x0000
	jne lessthan65536sectors
	mov eax, [rdi+0x20]
lessthan65536sectors:
	mov [fat16_TotalSectors], eax

; Calculate the size in MiB
	xor rax, rax
	mov eax, [fat16_TotalSectors]
	mov [hd1_maxlba], rax
	shr rax, 11			; rax = rax * 512 / 1048576
	mov [hd1_size], eax		; in mebibytes (MiB)

; Create a string of the harddrive size
	mov rdi, hdtempstring
	call os_int_to_string

	xor rax, rax
	xor rbx, rbx
	mov ax, [fat16_SectorsPerFat]
	shl ax, 1			; quick multiply by two
	add ax, [fat16_ReservedSectors]
	mov [fat16_RootStart], eax
	mov bx, [fat16_RootDirEnts]
	shr ebx, 4			; bx = (bx * 32) / 512
	add ebx, eax			; BX now holds the datastart sector number
	mov [fat16_DataStart], ebx

ret


; -----------------------------------------------------------------------------
; readsectors -- Read sectors on the hard drive
; IN:	RAX = starting sector to read
;	RCX = number of sectors to read (1 - 256)
;	RDI = memory location to store sectors
; OUT:	RAX = RAX + number of sectors that were read
;	RCX = number of sectors that were read (0 on error)
;	RDI = RDI + (number of sectors * 512)
;	All other registers preserved
readsectors:
	push rdx
	push rcx
	push rbx
	push rax

	push rcx		; Save RCX for use in the read loop
	mov rbx, rcx		; Store number of sectors to read
	cmp rcx, 256
	jg readsectors_fail	; Over 256? Fail!
	jne readsectors_skip	; Not 256? No need to modify CL
	xor rcx, rcx		; 0 translates to 256
readsectors_skip:

	push rax		; Save RAX since we are about to overwrite it
	mov dx, 0x01F2		; 0x01F2 - Sector count Port 7:0
	mov al, cl		; Read CL sectors
	out dx, al
	pop rax			; Restore RAX which is our sector number
	inc dx			; 0x01F3 - LBA Low Port 7:0
	out dx, al
	inc dx			; 0x01F4 - LBA Mid Port 15:8
	shr rax, 8
	out dx, al
	inc dx			; 0x01F5 - LBA High Port 23:16
	shr rax, 8
	out dx, al
	inc dx			; 0x01F6 - Device Port. Bit 6 set for LBA mode, Bit 4 for device (0 = master, 1 = slave), Bits 3-0 for LBA "Extra High" (27:24)
	shr rax, 8
	and al, 00001111b 	; Clear bits 4-7 just to be safe
	or al, 01000000b	; Turn bit 6 on since we want to use LBA addressing, leave device at 0 (master)
	out dx, al
	inc dx			; 0x01F7 - Command Port
	mov al, 0x20		; Read sector(s). 0x24 if LBA48
	out dx, al

	mov rcx, 4
readsectors_wait:
	in al, dx		; Read status from 0x01F7
	test al, 0x80		; BSY flag set?
	jne readsectors_retry
	test al, 0x08		; DRQ set?
	jne readsectors_dataready
readsectors_retry:
	dec rcx
	jg readsectors_wait
readsectors_nextsector:
	in al, dx		; Read status from 0x01F7
	test al, 0x80		; BSY flag set?
	jne readsectors_nextsector
	test al, 0x21		; ERR or DF set?
	jne readsectors_fail

readsectors_dataready:
	sub dx, 7		; Data port (0x1F0)
	mov rcx, 256		; Read 
	rep insw		; Copy a 512 byte sector to RDI
	add dx, 7		; Set DX back to status register (0x01F7)
	in al, dx		; Delay ~400ns to allow drive to set new values of BSY and DRQ
	in al, dx
	in al, dx
	in al, dx

	dec rbx			; RBX is the "sectors to read" counter
	cmp rbx, 0
	jne readsectors_nextsector

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
	xor rcx, rcx		; Set RCX to 0 since nothing was read
ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
