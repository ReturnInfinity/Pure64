; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; Interrupts
; =============================================================================


; -----------------------------------------------------------------------------
; Default exception handler
exception_gate:
exception_gate_halt:
	cli				; Disable interrupts
	hlt				; Halt the system
	jmp exception_gate_halt
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; Default interrupt handler
interrupt_gate:				; handler for all other interrupts
	iretq
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; Floppy drive interrupt. IRQ 0x06, INT 0x26
; This IRQ runs when floppy drive reads from or writes to whole disk
%ifdef BIOS
align 16
floppy_irq:
	push rdi
	push rbx
	push rax

	mov word [int_done], 1
	mov al, 0x20			; Acknowledge the IRQ
	out 0x20, al

	pop rax
	pop rbx
	pop rdi
	iretq
%endif
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; Spurious interrupt. INT 0xFF
align 16
spurious:				; handler for spurious interrupts
	iretq
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; CPU Exception Gates
exception_gate_00:
	mov al, 0x00
	jmp exception_gate_main

exception_gate_01:
	mov al, 0x01
	jmp exception_gate_main

exception_gate_02:
	mov al, 0x02
	jmp exception_gate_main

exception_gate_03:
	mov al, 0x03
	jmp exception_gate_main

exception_gate_04:
	mov al, 0x04
	jmp exception_gate_main

exception_gate_05:
	mov al, 0x05
	jmp exception_gate_main

exception_gate_06:
	mov al, 0x06
	jmp exception_gate_main

exception_gate_07:
	mov al, 0x07
	jmp exception_gate_main

exception_gate_08:
	mov al, 0x08
	jmp exception_gate_main

exception_gate_09:
	mov al, 0x09
	jmp exception_gate_main

exception_gate_10:
	mov al, 0x0A
	jmp exception_gate_main

exception_gate_11:
	mov al, 0x0B
	jmp exception_gate_main

exception_gate_12:
	mov al, 0x0C
	jmp exception_gate_main

exception_gate_13:
	mov al, 0x0D
	jmp exception_gate_main

exception_gate_14:
	mov al, 0x0E
	jmp exception_gate_main

exception_gate_15:
	mov al, 0x0F
	jmp exception_gate_main

exception_gate_16:
	mov al, 0x10
	jmp exception_gate_main

exception_gate_17:
	mov al, 0x11
	jmp exception_gate_main

exception_gate_18:
	mov al, 0x12
	jmp exception_gate_main

exception_gate_19:
	mov al, 0x13
	jmp exception_gate_main

exception_gate_20:
	mov al, 0x14
	jmp exception_gate_main

exception_gate_21:
	mov al, 0x15
	jmp exception_gate_main

exception_gate_main:
	; Set screen to Red
	mov rdi, [0x00005F00]		; Frame buffer base
	mov rcx, [0x00005F08]		; Frame buffer size
	shr rcx, 2			; Quick divide by 4
	mov eax, 0x00FF0000		; 0x00RRGGBB
	rep stosd
exception_gate_main_hang:
	hlt
	jmp exception_gate_main_hang	; Hang. User must reset machine at this point
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; create_gate
; rax = address of handler
; rdi = gate # to configure
create_gate:
	push rdi
	push rax

	shl rdi, 4			; quickly multiply rdi by 16
	stosw				; store the low word (15:0)
	shr rax, 16
	add rdi, 4			; skip the gate marker
	stosw				; store the high word (31:16)
	shr rax, 16
	stosd				; store the high dword (63:32)

	pop rax
	pop rdi
	ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
