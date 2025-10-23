; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; INIT PIT
; =============================================================================


init_pit:
	; Enable PIT
	; Base rate is 1193182 Hz
	mov al, 0x34			; Channel 0 (7:6), Access Mode lo/hi (5:4), Mode 2 (3:1), Binary (0)
	out 0x43, al
	; Set divisor to 12 (1193182 Hz / 12 = ~99432 Hz)
	; This gives a precision of ~10 microseconds
	mov al, 0x74
	out 0x40, al			; Set low byte of PIT reload value
	mov al, 0x00
	out 0x40, al			; Set high byte of PIT reload value

	; Create gate for PIT IRQ
	mov edi, 0x20
	mov eax, pit_irq
	call create_gate

	; Enable PIT Interrupt via the PIC
	in al, 0x21
	mov al, 11111110b		; Enable PIT
	out 0x21, al

	ret


; -----------------------------------------------------------------------------
; os_pit_delay -- Delay by X microseconds
; IN:	RAX = Time microseconds
; OUT:	All registers preserved
; Note:	There are 1,000,000 microseconds in a second
;	There are 1,000 milliseconds in a second
os_pit_delay:
	push rdx
	push rcx
	push rbx
	push rax

	; The PIT only gives us a precision of ~10 microseconds
	; We need to divide RAX by 10
	mov ecx, 10
	xor edx, edx
	div rcx

	add rax, [p_Counter_Timer]

os_pit_delay_loop:
	mov rbx, [p_Counter_Timer]
	cmp rax, rbx
	jae os_pit_delay_loop

	pop rax
	pop rbx
	pop rcx
	pop rdx
	ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF