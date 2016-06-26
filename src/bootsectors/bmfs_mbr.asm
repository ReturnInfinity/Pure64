; =============================================================================
; Pure64 MBR -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2016 Return Infinity -- see LICENSE.TXT
;
; This Master Boot Record will load Pure64 from a pre-defined location on the
; hard drive without making use of the file system.
;
; In this code we are expecting a BMFS-formatted drive. With BMFS the Pure64
; binary is required to start at sector 16 (8192 bytes from the start). A small
; ckeck is made to make sure Pure64 was loaded by comparing a signiture.
; =============================================================================

%define SECTORS 64      ; Number of sectors to load. 64 sectors = 32768 bytes
%define OFFSET 16       ; Start immediately after directory (offset 8192)
%define ADDRESS 0x8000  ; Pure64 expects to be loaded at 0x8000
%define SEGMENT 0x0000

USE16
org 0x7C00

entry:
	cli				; Disable interrupts
;	xchg bx, bx			; Bochs magic debug
	xor eax, eax
	mov ss, ax
	mov es, ax
	mov ds, ax
	mov sp, 0x7C00
	sti				; Enable interrupts

	mov [DriveNumber], dl		; BIOS passes drive number in DL

	mov si, msg_Load
	call print_string_16

	mov ah, 0x42           ; extended read sectors from drive
	mov si, DAP            ; disk address packet (https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH.3D42h:_Extended_Read_Sectors_From_Drive)
	int 0x13

	mov eax, [0x8000]
	cmp eax, 0xC03166FA		; Match against the Pure64 binary
	jne magic_fail

	mov si, msg_LoadDone
	call print_string_16

	jmp 0x0000:0x8000

magic_fail:
	mov si, msg_MagicFail
	call print_string_16
halt:
	hlt
	jmp halt


;------------------------------------------------------------------------------
; 16-bit function to print a string to the screen
; IN:	SI - Address of start of string
print_string_16:			; Output string in SI to screen
	pusha
	mov ah, 0x0E			; int 0x10 teletype function
.repeat:
	lodsb				; Get char from string
	cmp al, 0
	je .done			; If char is zero, end of string
	int 0x10			; Otherwise, print it
	jmp short .repeat
.done:
	popa
	ret
;------------------------------------------------------------------------------


msg_Load db "BMFS MBR v1.0 - Loading Pure64", 0
msg_LoadDone db " - done.", 13, 10, "Executing...", 0
msg_MagicFail db " - Not found!", 0
DriveNumber db 0x00

DAP:    db 0x10         ; structure size
        db 0x00         ; unused, should be zero
        dw SECTORS      ; number of sectors to be read
        dw ADDRESS      ; offset pointer to memory
        dw SEGMENT      ; segment where system will be loaded
        dq OFFSET       ; block offset on disk

times 446-$+$$ db 0

; False partition table entry required by some BIOS vendors.
db 0x80, 0x00, 0x01, 0x00, 0xEB, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF

times 510-$+$$ db 0

sign dw 0xAA55
