;------------------------------------------------------------------------------
; MBR for use on BMFS. Loads PURE64 from (512-byte) sector 16 and jumps.
;------------------------------------------------------------------------------

USE16
org 0x0600

entry:
	cli
	xor ax, ax
	mov ss, ax
	mov es, ax
	mov ds, ax
	mov sp, 0x7C00
	sti

; Copy MBR sector to 0x0600 and jump there
	cld
	mov si, sp
	mov di, 0x0600
	mov cx, 0x0100
	rep movsw
	jmp 0x0000:load

; Print message
load:
	mov [DriveNumber], dl	; BIOS passes drive number in DL

	mov si, msg_Load
	call print_string_16

	mov eax, 0
	mov ebx, 16			; Start immediately after directory
	mov cx, 0x8000			; Pure64 expects to be loaded at 0x8000
	times 16 call readsector	; Load 8KiB

	mov eax, [0x8000]
	cmp eax, 0xC03166FA
	jne magic_fail

	mov si, msg_LoadDone
	call print_string_16

	jmp 0x0000:0x8000

magic_fail:
	mov si, msg_MagicFail
	call print_string_16
	hlt

;------------------------------------------------------------------------------
; Read a sector from a disk, using LBA
; input:	EAX - High word of 64-bit DOS sector number
;		EBX - Low word of 64-bit DOS sector number
; 		ES:CX - destination buffer
; output:	ES:CX points one byte after the last byte read
; 		EAX - High word of next sector
;		EBX - Low word of sector
readsector:
	push dx
	push si
	push di

read_it:
	push eax	; Save the sector number
	push ebx
	mov di, sp	; remember parameter block end

	push eax	; [C] sector number high 32bit
	push ebx	; [8] sector number low 32bit
	push es		; [6] buffer segment
	push cx		; [4] buffer offset
	push byte 1	; [2] 1 sector (word)
	push byte 16	; [0] size of parameter block (word)

	mov si, sp
	mov dl, [DriveNumber]
	mov ah, 42h	; EXTENDED READ
	int 0x13	; http://hdebruijn.soo.dto.tudelft.nl/newpage/interupt/out-0700.htm#0651

	mov sp, di	; remove parameter block from stack
	pop ebx
	pop eax		; Restore the sector number

	jnc read_ok	; jump if no error

	push ax
	xor ah, ah	; else, reset and retry
	int 0x13
	pop ax
	jmp read_it

read_ok:
	add ebx, 1	; increment next sector with carry
	adc eax, 0
	add cx, 512	; Add bytes per sector
	jnc no_incr_es	; if overflow...

incr_es:
	mov dx, es
	add dh, 0x10	; ...add 1000h to ES
	mov es, dx

no_incr_es:
	pop di
	pop si
	pop dx

	ret
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; 16-bit Function to print a sting to the screen
; input:    SI - Address of start of string
print_string_16:	; Output string in SI to screen
	pusha
	mov ah, 0x0E	; int 0x10 teletype function
.repeat:
	lodsb		; Get char from string
	cmp al, 0
	je .done	; If char is zero, end of string
	int 0x10	; Otherwise, print it
	jmp short .repeat
.done:
	popa
	ret
;------------------------------------------------------------------------------


msg_Load db "Loading PURE64", 0
msg_LoadDone db " - done", 0
msg_MagicFail db " - failed magic number check", 0
DriveNumber db 0x00

times 446-$+$$ db 0

tables db "XXXXXXXXXXXXXXXX  DO NOT OVERWRITE THIS AREA!!! XXXXXXXXXXXXXXXX"	; 64 bytes in length

sign dw 0xAA55
