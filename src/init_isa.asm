; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2011 Return Infinity -- see LICENSE.TXT
;
; INIT ISA
; =============================================================================


isa_setup:
	mov edi, 0x00004000		; Clear out memory for the E820 map
	xor eax, eax
	mov ecx, 2048
	rep stosd

	mov al, '1'
	call serial_send_16

; Get the BIOS E820 Memory Map
; use the INT 0x15, eax= 0xE820 BIOS function to get a memory map
; inputs: es:di -> destination buffer for 24 byte entries
; outputs: bp = entry count, trashes all registers except esi
do_e820:
	mov edi, 0x00004000		; location that memory map will be stored to
	xor ebx, ebx			; ebx must be 0 to start
	xor bp, bp			; keep an entry count in bp
	mov edx, 0x0534D4150		; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24			; ask for 24 bytes
	int 0x15
	jc nomemmap			; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150		; Some BIOSes apparently trash this register?
	cmp eax, edx			; on success, eax must have been reset to "SMAP"
	jne nomemmap
	test ebx, ebx			; ebx = 0 implies list is only 1 entry long (worthless)
	je nomemmap
	jmp jmpin
e820lp:
	mov eax, 0xe820			; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24			; ask for 24 bytes again
	int 0x15
	jc memmapend			; carry set means "end of list already reached"
	mov edx, 0x0534D4150		; repair potentially trashed register
jmpin:
	jcxz skipent			; skip any 0 length entries
	cmp cl, 20			; got a 24 byte ACPI 3.X response?
	jbe notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je skipent
notext:
	mov ecx, [es:di + 8]		; get lower dword of memory region length
	test ecx, ecx			; is the qword == 0?
	jne goodent
	mov ecx, [es:di + 12]		; get upper dword of memory region length
	jecxz skipent			; if length qword is 0, skip entry
goodent:
	inc bp				; got a good entry: ++count, move to next storage spot
	add di, 32
skipent:
	test ebx, ebx			; if ebx resets to 0, list is complete
	jne e820lp
nomemmap:
	mov byte [cfg_e820], 0		; No memory map function	
memmapend:
	xor eax, eax			; Create a blank record for termination (32 bytes)
	mov ecx, 8
	rep stosd

	mov al, '2'
	call serial_send_16

; Enable the A20 gate
set_A20:
	in al, 0x64
	test al, 0x02
	jnz set_A20
	mov al, 0xD1
	out 0x64, al
check_A20:
	in al, 0x64
	test al, 0x02
	jnz check_A20
	mov al, 0xDF
	out 0x60, al

	mov al, '3'
	call serial_send_16

; Set PIT Channel 0 to fire at 1000Hz (Divisor = 1193180 / hz)
	mov al, 0x36			; Set Timer
	out 0x43, al
	mov al, 0xA9			; We want 100MHz so 0x2E9B
	out 0x40, al			; 1000MHz would be 0x04A9
	mov al, 0x04
	out 0x40, al

	mov al, '4'
	call serial_send_16

; Set keyboard repeat rate to max
	mov al, 0xf3
	out 0x60, al			; Set Typematic Rate/Delay
	xor al, al
	out 0x60, al			; 30 cps and .25 second delay
	mov al, 0xed
	out 0x60, al			; Set/Reset LEDs
	xor al, al
	out 0x60, al			; all off

	mov al, '5'
	call serial_send_16

; Set up RTC
	mov al, 0x0B
	out 0x70, al
	in al, 0x71
	or al, 00000010b		; Bit 2 (0) Data Mode to BCD, Bit 1 (1) 24 hour mode
	push ax
	mov al, 0x0B
	out 0x70, al
	pop ax
	out 0x71, al

	mov al, '6'
	call serial_send_16

; VBE init
	cmp byte [cfg_vesa], 1		; Check if VESA should be enabled
	jne VBEdone			; If not then skip VESA init

	mov al, '7'
	call serial_send_16

	mov edi, VBEModeInfoBlock	; VBE data will be stored at this address
	mov ax, 0x4F01			; GET SuperVGA MODE INFORMATION - http://www.ctyme.com/intr/rb-0274.htm
	; CX queries the mode, it should be in the form 0x41XX as bit 14 is set for LFB and bit 8 is set for VESA mode
	; 0x4112 is 640x480x24bit	0x4129 is 640x480x32bit
	; 0x4115 is 800x600x24bit	0x412E is 800x600x32bit
	; 0x4118 is 1024x768x24bit	0x4138 is 1024x768x32bit
	; 0x411B is 1280x1024x24bit	0x413D is 1280x1024x32bit
	mov cx, 0x4112			; Put your desired mode here
	mov bx, cx			; Mode is saved to BX for the set command later
	int 0x10

	cmp ax, 0x004F			; Return value in AX should equal 0x004F if supported and sucessful
	jne VBEfail
	cmp byte[VBEModeInfoBlock.BitsPerPixel], 24	; Make sure this matches the number of bits for the mode!
	jne VBEfail			; If set bit mode was unsucessful then bail out

	mov ax, 0x4F02			; SET SuperVGA VIDEO MODE - http://www.ctyme.com/intr/rb-0275.htm
	int 0x10
	cmp ax, 0x004F			; Return value in AX should equal 0x004F if supported and sucessful
	jne VBEfail

	jmp VBEdone

VBEfail:
	mov byte [cfg_vesa], 0		; Clear the VESA config as it was not sucessful
	mov al, 'B'
	call serial_send_16

VBEdone:

	mov al, 'C'
	call serial_send_16

; Remap IRQ's
; As heard on an episode of Jerry Springer.. "It's time to lose the zero (8259 PIC) and get with a hero (IOAPIC)".
; http://osdever.net/tutorials/apicarticle.php
	mov al, 00010001b		; begin PIC 1 initialization
	out 0x20, al
	mov al, 00010001b		; begin PIC 2 initialization
	out 0xA0, al
	mov al, 0x20			; IRQ 0-7: interrupts 20h-27h
	out 0x21, al
	mov al, 0x28			; IRQ 8-15: interrupts 28h-2Fh
	out 0xA1, al
	mov al, 4
	out 0x21, al
	mov al, 2
	out 0xA1, al
	mov al, 1
	out 0x21, al
	out 0xA1, al

	mov al, 'D'
	call serial_send_16

	in al, 0x21
	mov al, 11111110b		; Disable all IRQs except for timer
	out 0x21, al
	in al, 0xA1
	mov al, 11111111b
	out 0xA1, al

	mov al, 'E'
	call serial_send_16

ret


; =============================================================================
; EOF
