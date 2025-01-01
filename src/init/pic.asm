; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; INIT PIC
; =============================================================================


init_pic:
	; Initialize and remap PIC IRQ's
	; ICW1
	mov al, 0x11;			; Initialize PIC 1, init (bit 4) and ICW4 (bit 0)
	out 0x20, al
	mov al, 0x11;			; Initialize PIC 2, init (bit 4) and ICW4 (bit 0)
	out 0xA0, al
	; ICW2
	mov al, 0x20			; IRQ 0-7: interrupts 20h-27h
	out 0x21, al
	mov al, 0x28			; IRQ 8-15: interrupts 28h-2Fh
	out 0xA1, al
	; ICW3
	mov al, 4
	out 0x21, al
	mov al, 2
	out 0xA1, al
	; ICW4
	mov al, 1
	out 0x21, al
	mov al, 1
	out 0xA1, al

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

	; Enable specific interrupts
	in al, 0x21
	mov al, 11111001b		; Enable Cascade (HPET Timer 0), Keyboard
	out 0x21, al

	sti				; Enable interrupts

	; Acknowledge the RTC
	mov al, 0x0C			; Status Register C
	out 0x70, al			; Select the address
	in al, 0x71			; Read the value to clear any existing interrupt value

	; Enable PIT
	; Base rate is 1193182 Hz
	mov al, 0x36			; Channel 0 (7:6), Access Mode lo/hi (5:4), Mode 3 (3:1), Binary (0)
	out 0x43, al
	mov al, 0x3C			; 60
	out 0x40, al			; Set low byte of PIT reload value
	mov al, 0x00
	out 0x40, al			; Set high byte of PIT reload value
	; New rate is 19866 Hz (1193182 / 60)
	; 0.050286633812733 milliseconds (ms)
	; 50.28663381273300814 microseconds (us)

	ret


; =============================================================================
; EOF
