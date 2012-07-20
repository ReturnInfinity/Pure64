; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2012 Return Infinity -- see LICENSE.TXT
;
; BareMetal File System functions
; =============================================================================


; -----------------------------------------------------------------------------
; findfile --
; IN:	RSI(Pointer to file name)
; OUT:	RAX(Staring block number), 0x0 if not found
;		RCX(Number of blocks), 0x0 if not found
findfile:
	push rsi
	push rdi

	mov rdi, 0x200000		; Read to this memory address
	mov rsi, rdi
	xor eax, eax
	call readblock			; Read the first block of the disk

	mov rcx, 64				; BMFS v1 has a max of 64 records
	mov rdi, kernelname
	add rsi, 4096			; Directory is 4K in

findfile_next:
	call os_string_uppercase
	call os_string_compare
	jc findfile_found
	add rsi, 64
	sub rcx, 1
	cmp rcx, 0
	je findfile_error
	jmp findfile_next

findfile_found:
	add rsi, 32
	lodsq					; Starting block number
	xchg rax, rcx
	lodsq					; Reserved blocks
	xchg rax, rcx

findfile_done:
	pop rdi
	pop rsi
ret

findfile_error:
;	mov rsi, findfile_err_msg
;	call os_print_string
	xor eax, eax
	xor ecx, ecx
	jmp findfile_done

;findfile_err_msg:	db 'File not found.', 0
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; readblock -- Read block on the hard drive
; IN:	RAX = starting block to read
;	RDI = memory location to block
; OUT:	Nothing, all registers preserved
readblock:
	push rdi
	push rsi
	push rcx
	push rax

	push rdi					; Save the destination memory address
	push rax					; Save the block number

	xor ecx, ecx
	mov ecx, [drive_port]
	shl rcx, 7					; Quick multiply by 0x80
	add rcx, 0x100				; Offset to port 0

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
	mov eax, 0x00258027			; 25 dma read, bit 15 set, fis 27 H2D
	stosd						; feature 7:0, command, c, fis
	pop rax						; Restore the block number
	shl rax, 12					; Sector start = Block number * 4096
	push rax					; Save the sector start number
	shl rax, 36
	shr rax, 36					; Upper 36 bits cleared
	bts rax, 30					; bit 30 set for LBA
	stosd						; device, lba 23:16, lba 15:8, lba 7:0
	pop rax						; Restore the sector start number
	shr rax, 24
	stosd						; feature 15:8, lba 47:40, lba 39:32, lba 31:24
;	mov eax, 0x40000000			; bit 30 set for LBA
;	stosd						; device, lba 23:16, lba 15:8, lba 7:0
;	mov eax, 0x00000000
;	stosd						; feature 15:8, lba 47:40, lba 39:32, lba 31:24
	mov eax, 0x00001000			; Read 4096 sectors
	stosd						; control, ICC, count 15:8, count 7:0
	mov eax, 0x00000000
	stosd						; reserved
	mov rdi, 0x72080
	pop rax						; Restore the destination memory address
	stosd						; Data Base Address
	shr rax, 32
	stosd						; Data Base Address Upper
;	mov eax, 0x00200000			; store here
;	stosd						; Data Base Address
;	xor eax, eax
;	stosd						; Data Base Address Upper
	stosd						; Reserved
	mov eax, 0x001FFFFF			; 2097152 - 1 (2MiB read)
	stosd						; Description Information

	add rsi, rcx

	mov rdi, rsi
	add rdi, 0x10				; Port x Interrupt Status
	xor eax, eax
	stosd

	mov rdi, rsi
	add rdi, 0x18				; Offset to port 0
	mov eax, [rdi]
	bts eax, 4					; FRE
	bts eax, 0					; ST
	stosd

	mov rdi, rsi
	add rdi, 0x38				; Command Issue
	mov eax, 0x00000001			; Execute Command Slot 0
	stosd

readblock_poll:
	mov eax, [rsi+0x38]
	cmp eax, 0
	jne readblock_poll

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
