; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2024 Return Infinity -- see LICENSE.TXT
;
; INIT PIC
; =============================================================================


init_pic:
	; Mask all PIC interrupts
	mov al, 0xFF
	out 0x21, al
	out 0xA1, al

	; Disable NMIs
	in al, 0x70
	or al, 0x80
	out 0x70, al
	in al, 0x71

; Set up RTC
rtc_poll:
	mov al, 0x0A			; Status Register A
	out 0x70, al			; Select the address
	in al, 0x71			; Read the data
	test al, 0x80			; Is there an update in process?
	jne rtc_poll			; If so then keep polling
	mov al, 0x0A			; Status Register A
	out 0x70, al			; Select the address
	mov al, 00100110b		; UIP (0), RTC@32.768KHz (010), Rate@1024Hz (0110)
	out 0x71, al			; Write the data
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	in al, 0x71			; Read the current settings
	push rax			; Save the current Register B settings
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	pop rax				; Restore the Register B settings to AL
	bts ax, 6			; Set Periodic Interrupt Enable (bit 6)
	out 0x71, al			; Write the new settings

	; Remap PIC IRQ's
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

	; Enable specific interrupts
	in al, 0x21
	mov al, 11111001b		; Enable Cascade, Keyboard
	out 0x21, al
	in al, 0xA1
	mov al, 11111110b		; Enable RTC
	out 0xA1, al

	; Acknowledge the RTC
	mov al, 0x0C			; Status Register C
	out 0x70, al			; Select the address
	in al, 0x71			; Read the value to clear any existing interrupt value

	sti				; Enable interrupts

	ret


; =============================================================================
; EOF
