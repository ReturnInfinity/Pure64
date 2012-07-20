; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2012 Return Infinity -- see LICENSE.TXT
;
; INIT HDD
; =============================================================================


hdd_setup:

; Probe for an ACHI hard drive controller
	xor ebx, ebx
	xor ecx, ecx
findcontroller:
	cmp bx, 256					; Search up to 256 buses
	je hdd_setup_err_nosata		; No ACHI controller detected
	cmp cx, 256					; Up to 32 devices per bus
	jne findcontroller_1
	add bx, 1					; Next bus
	xor ecx, ecx
findcontroller_1:
	mov dl, 2					; We want the Class/Device code
	call os_pci_read_reg
	add cx, 1					; Increment the device number
	shr rax, 16
	cmp ax, 0xFFFF				; Non-existant device
	je findcontroller
	cmp ax, 0x0106				; Mass storage device, SATA
;	pushf
;	call os_debug_dump_ax
;	push rax
;	mov al, ' '
;	call os_print_newline
;	pop rax
;	popf
	jne findcontroller
	sub cl, 1
	mov dl, 9
	xor eax, eax
	call os_pci_read_reg		; BAR5 (AHCI Base Address Register)
	mov [sata_base], rax

; Basic config of the controller, port 0
	mov rsi, rax				; RSI holds the ABAR
	mov rdi, rsi

; Search the implemented ports for a drive
	mov eax, [rsi+0x0C]			; PI – Ports Implemented
	mov edx, eax
	xor ecx, ecx
	mov ebx, 0x128				; Offset to Port 0 Serial ATA Status
nextport:
	bt edx, 0					; Valid port?
	jnc nodrive
	mov eax, [rsi+rbx]
	cmp eax, 0
	je nodrive
	jmp founddrive

nodrive:
	add ecx, 1
	shr edx, 1
	add ebx, 0x80				; Each port has a 128 byte memory space
	cmp ecx, 32
	je hdd_setup_err_nodisk
	jmp nextport

; Configure the first port found with a drive attached
founddrive:
	mov [drive_port], ecx
	mov rdi, rsi
	add rdi, 0x100				; Offset to port 0
	push rcx					; Save port number
	shl rcx, 7					; Quick multiply by 0x80
	add rdi, rcx
	pop rcx						; Restore port number
	mov rax, 0x70000			; 1024 bytes per port
	stosd						; Offset 00h: PxCLB – Port x Command List Base Address
	xor eax, eax
	stosd						; Offset 04h: PxCLBU – Port x Command List Base Address Upper 32-bits
	mov rax, 0x71000			; 256 or 4096 bytes per port
	stosd						; Offset 08h: PxFB – Port x FIS Base Address
	xor eax, eax
	stosd						; Offset 0Ch: PxFBU – Port x FIS Base Address Upper 32-bits
	stosd						; Offset 10h: PxIS – Port x Interrupt Status
	stosd						; Offset 14h: PxIE – Port x Interrupt Enable

	; Query drive
	mov rdi, 0x200000
	call iddrive
	mov rsi, 0x200000
	mov eax, [rsi+200]		; Max LBA Extended
	shr rax, 11				; rax = rax * 512 / 1048576		MiB
;	shr rax, 21				; rax = rax * 512 / 1073741824	GiB
	mov [hd1_size], eax		; in mebibytes (MiB)
	mov rdi, hdtempstring
	call os_int_to_string

	; Found a bootable drive
	mov byte [cfg_hdd], 0x01

ret

hdd_setup_err_nosata:
hdd_setup_err_nodisk:
	ret

; -----------------------------------------------------------------------------
; iddrive -- Identify a SATA drive
; IN:	RCX = Port # to query
;	RDI = memory location to store details (512 bytes)
; OUT:	Nothing, all registers preserved
iddrive:
	push rdi
	push rsi
	push rcx
	push rax

	shl rcx, 7					; Quick multiply by 0x80
	add rcx, 0x100				; Offset to port 0

	push rdi					; Save the destination memory address
;	push rax					; Save the block number

	mov rsi, [sata_base]

	mov rdi, 0x70000			; command list (1K with 32 entries, 32 bytes each)
	xor eax, eax
	mov eax, 0x00010005 ;4			; 1 PRDTL Entry, Command FIS Length = 16 bytes
	stosd						; DW 0 - Description Information
	xor eax, eax
	stosd						; DW 1 - Command Status
	mov eax, 0x72000
	stosd						; DW 2 - Command Table Base Address
	xor eax, eax
	stosd						; DW 3 - Command Table Base Address Upper
	stosd
	stosd
	stosd
	stosd
	; DW 4 - 7 are reserved

	; command table
	mov rdi, 0x72000			; Build a command table for Port 0
	mov eax, 0x00EC8027			; EC identify, bit 15 set, fis 27 H2D
	stosd						; feature 7:0, command, c, fis
	xor eax, eax
	stosd						; device, lba 23:16, lba 15:8, lba 7:0
	stosd						; feature 15:8, lba 47:40, lba 39:32, lba 31:24
	stosd						; control, ICC, count 15:8, count 7:0
;	stosd						; reserved
	mov rdi, 0x72080
	pop rax						; Restore the destination memory address
	stosd						; Data Base Address
	shr rax, 32
	stosd						; Data Base Address Upper
	xor eax, eax
	stosd						; Reserved
	mov eax, 0x000001FF			; 512 - 1
	stosd						; Description Information

	add rsi, rcx

	mov rdi, rsi
	add rdi, 0x10				; Port x Interrupt Status
	xor eax, eax
	stosd

	mov rdi, rsi
	add rdi, 0x18				; Offset to port 0 Command and Status
	mov eax, [rdi]
	bts eax, 4					; FRE
	bts eax, 0					; ST
	stosd

	mov rdi, rsi
	add rdi, 0x38				; Command Issue
	mov eax, 0x00000001			; Execute Command Slot 0
	stosd

iddrive_poll:
;	mov eax, [rsi+0x10]
;	call os_debug_dump_eax
	mov eax, [rsi+0x38]
;	call os_debug_dump_eax
	cmp eax, 0
	jne iddrive_poll

	mov rdi, rsi
	add rdi, 0x18				; Offset to port 0
	mov eax, [rdi]
	btc eax, 4					; FRE
	btc eax, 0					; ST
	stosd

	pop rax
	pop rcx
	pop rsi
	pop rdi
ret
; -----------------------------------------------------------------------------



; =============================================================================
; EOF
