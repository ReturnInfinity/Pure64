; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; INIT SERIAL
; =============================================================================


init_serial:
	; Disable Interrupts
	mov dx, COM_PORT_INTERRUPT_ENABLE
	mov al, 0			; Disable all interrupts
	out dx, al

	; Enable divisor register for setting baud rate
	mov dx, COM_PORT_LINE_CONTROL
	mov dl, 0x80			; DLB (7 set)
	out dx, al

	; Send the divisor (baud rate will be 115200 / divisor)
	mov dx, COM_PORT_DATA
	mov ax, BAUD_115200
	out dx, al
	mov dx, COM_PORT_DATA+1
	shr ax, 8
	out dx, al

	; Disable divisor register and set values
	mov dx, COM_PORT_LINE_CONTROL
	mov al, 00000111b		; 8 data bits (0-1 set), one stop bit (2 set), no parity (3-5 clear), DLB (7 clear)
	out dx, al

	; Disable modem control
	mov dx, COM_PORT_MODEM_CONTROL
	mov al, 0
	out dx, al

	; Set FIFO
	mov dx, COM_PORT_FIFO_CONTROL
	mov al, 0xC7			; Enable FIFO, clear them, 14-byte threshold
	out dx, al

	ret


; Port Registers
COM_BASE			equ 0x3F8
COM_PORT_DATA			equ COM_BASE + 0
COM_PORT_INTERRUPT_ENABLE	equ COM_BASE + 1
COM_PORT_FIFO_CONTROL		equ COM_BASE + 2
COM_PORT_LINE_CONTROL		equ COM_BASE + 3
COM_PORT_MODEM_CONTROL		equ COM_BASE + 4
COM_PORT_LINE_STATUS		equ COM_BASE + 5
COM_PORT_MODEM_STATUS		equ COM_BASE + 6
COM_PORT_SCRATCH_REGISTER	equ COM_BASE + 7

; Baud Rates
BAUD_115200			equ 1
BAUD_57600			equ 2
BAUD_9600			equ 12
BAUD_300			equ 384


; =============================================================================
; EOF