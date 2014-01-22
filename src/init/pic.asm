; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2013 Return Infinity -- see LICENSE.TXT
;
; INIT PIC
; =============================================================================


init_pic:
	; Enable specific interrupts
	xor eax, eax
	in al, 0x21
	mov al, 11111001b		; Enable Cascade, Keyboard
	out 0x21, al
	in al, 0xA1
	mov al, 11111110b		; Enable RTC
	out 0xA1, al

	; Set the periodic flag in the RTC
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	in al, 0x71			; Read the current settings
	movzx ebx al
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	or  ebx, 0x40			; Set Periodic(6)
	mov eax, ebx
	out 0x71, al			; Write the new settings

	sti				; Enable interrupts

	; Acknowledge the RTC
	mov al, 0x0C			; Status Register C
	out 0x70, al			; Select the address
	in al, 0x71			; Read the current settings

	ret


; =============================================================================
; EOF
