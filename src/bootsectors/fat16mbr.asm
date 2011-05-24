USE16
org 0x7C00

entry:
	cli
	mov [DriveNumber], dl	; BIOS passes drive number in DL
	xor ax, ax
	mov ss, ax
	mov sp, 0x7C00
	mov si, sp
	push ax
	pop es
	push ax
	pop ds
	sti

; Copy MBR sector to 0x0600 and jump there
	cld
	mov di, 0x0600
	mov cx, 0x0100
	repne movsw
	jmp 0x0000:0x0621
; 0x0621

; Print message
	mov si, msg_Load
	call print_string_16

	mov si, 0x07BE
	cmp byte [si], 0x80
	jne NoActivePartition

	mov ecx, [0x07C6]
	mov bx, 0x7C00
	xor esi, esi
	xor edi, edi
	mov si, bx
	mov di, bx
	add eax, ecx
	call readsector
	jmp 0x0000:0x7C00

NoActivePartition:
	mov si, msg_NoPartition
	call print_string_16
	jmp $


;------------------------------------------------------------------------------
; Read a sector from a disk, using LBA
; input:	EAX - 32-bit DOS sector number
;		ES:BX - destination buffer
; output:	ES:BX points one byte after the last byte read
;		EAX - next sector
readsector:
	push dx
	push si
	push di

read_it:
	push eax	; Save the sector number
	mov di, sp	; remember parameter block end

	push byte 0	; other half of the 32 bits at [C]
	push byte 0	; [C] sector number high 32bit
	push eax	; [8] sector number low 32bit
	push es 	; [6] buffer segment
	push bx 	; [4] buffer offset
	push byte 1	; [2] 1 sector (word)
	push byte 16	; [0] size of parameter block (word)

	mov si, sp
	mov dl, [DriveNumber]
	mov ah, 42h	; EXTENDED READ
	int 0x13	; http://hdebruijn.soo.dto.tudelft.nl/newpage/interupt/out-0700.htm#0651

	mov sp, di	; remove parameter block from stack
	pop eax		; Restore the sector number

	jnc read_ok 	; jump if no error

	push ax
	xor ah, ah	; else, reset and retry
	int 0x13
	pop ax
	jmp read_it

read_ok:
	inc eax 		; next sector
	add bx, 512		; Add bytes per sector
	jnc no_incr_es		; if overflow...

incr_es:
	mov dx, es
	add dh, 0x10		; ...add 1000h to ES
	mov es, dx

no_incr_es:
	pop di
	pop si
	pop dx
	ret
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; 16-bit Function to print a sting to the screen
; input:	SI - Address of start of string
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

msg_Load db "Loading... ", 0
msg_NoPartition db "No active partition found"
DriveNumber db 0x00

times 446-$+$$ db 0

tables db "XXXXXXXXXXXXXXXX  DO NOT OVERWRITE THIS AREA!!! XXXXXXXXXXXXXXXX" ; 64 bytes in length

sign dw 0xAA55
