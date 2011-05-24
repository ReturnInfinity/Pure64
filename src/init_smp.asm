; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2011 Return Infinity -- see LICENSE.TXT
;
; INIT SMP
; =============================================================================

;MP_debugmsg: db 'MP_CODE!', 0


smp_setup:
	sti				; Enable the timer
	mov al, '3'			; Start of MP init
	mov [0x000B809C], al
	mov al, '0'
	mov [0x000B809E], al
	mov al, 'S'
	call serial_send_64

; Step 1: Get APIC Information via ACPI
smp_check_for_acpi:			; Look for the Root System Description Pointer Structure
	mov rsi, 0x00000000000E0000	; We want to start looking here
	mov rbx, 'RSD PTR '		; This in the Signature for the ACPI Structure Table (0x2052545020445352)
searchingforACPI:
	lodsq				; Load a quad word from RSI and store in RAX, then increment RSI by 8
	cmp rax, rbx
	je foundACPI
	cmp rsi, 0x00000000000FFFFF	; Keep looking until we get here
	jge noMP			; We can't find ACPI either.. bail out and default to single cpu mode
	jmp searchingforACPI 

	mov al, '3'			; ACPI tables detected
	mov [0x000B809C], al
	mov al, '2'
	mov [0x000B809E], al

foundACPI:
	jmp init_smp_acpi 

makempgonow:
	mov al, '3'			; ACPI tables parsed
	mov [0x000B809C], al
	mov al, '6'
	mov [0x000B809E], al

; Step 2: Enable Local APIC on BSP
	mov rsi, [os_LocalAPICAddress]
	cmp rsi, 0x00000000
	je noMP				; Skip MP init if we didn't get a valid LAPIC address
	add rsi, 0xf0			; Offset to Spurious Interrupt Register
	mov rdi, rsi
	lodsd
	or eax, 0000000100000000b
	stosd

; Check if we want the AP's to be enabled.. if not then skip to end
;	cmp byte [cfg_smpinit], 1	; Check if SMP should be enabled
;	jne noMP			; If not then skip SMP init

; Step 3: Start the AP's one by one
	xor eax, eax
	xor ecx, ecx
	xor edx, edx
	mov rsi, [os_LocalAPICAddress]
	add rsi, 0x20		; Add the offset for the APIC ID location
	lodsd			; APIC ID is stored in bits 31:24
	shr rax, 24		; AL now holds the BSP CPU's APIC ID
	mov dl, al		; Store BSP APIC ID in DL
	mov rsi, 0x0000000000005800
	xor eax, eax

	mov al, '3'		; Start the AP's
	mov [0x000B809C], al
	mov al, '8'
	mov [0x000B809E], al

nextcore:
	cmp rsi, 0x0000000000005900
	je done
	lodsb
	cmp al, 1		; Is it enabled?
	jne skipcore

	push rax		; Debug - display APIC ID
	mov al, cl
	add al, 48
	call os_print_char
	call serial_send_64
	pop rax

	cmp cl, dl		; Is it the BSP?
	je skipcore

; Broadcast 'INIT' IPI to APIC ID in AL
	mov al, cl
	shl eax, 24
	mov rdi, [os_LocalAPICAddress]
	add rdi, 0x310
	stosd
	mov eax, 0x00004500
	mov rdi, [os_LocalAPICAddress]
	add rdi, 0x300
	stosd
	push rsi
verifyinit:
	mov rsi, [os_LocalAPICAddress]
	add rsi, 0x300
	lodsd
	bt eax, 12			; Verify that the command completed
	jc verifyinit
	pop rsi

	mov rax, [os_Counter]
	add rax, 10
wait1:
	mov rbx, [os_Counter]
	cmp rax, rbx
	jg wait1
	mov al, 'i'
	call serial_send_64

; Broadcast 'Startup' IPI to destination using vector 0x08 to specify entry-point is at the memory-address 0x00008000
	mov al, cl
	shl eax, 24
	mov rdi, [os_LocalAPICAddress]
	add rdi, 0x310
	stosd
	mov eax, 0x00004608		; Vector 0x08
	mov rdi, [os_LocalAPICAddress]
	add rdi, 0x300
	stosd
	push rsi
verifystartup1:
	mov rsi, [os_LocalAPICAddress]
	add rsi, 0x300
	lodsd
	bt eax, 12			; Verify that the command completed
	jc verifystartup1
	pop rsi

	mov rax, [os_Counter]
	add rax, 2
wait2:
	mov rbx, [os_Counter]
	cmp rax, rbx
	jg wait2
	mov al, 's'
	call serial_send_64

skipcore:
	inc cl
	jmp nextcore

done:
	mov al, '3'
	mov [0x000B809C], al
	mov al, 'A'
	mov [0x000B809E], al
	mov al, 'S'
	call serial_send_64	

; Let things settle (Give the AP's some time to finish)
	mov rax, [os_Counter]
	add rax, 10
wait3:
	mov rbx, [os_Counter]
	cmp rax, rbx
	jg wait3

; Step 4: Prepare the IOAPIC
; To be coded...

; Step 5: Finish up

noMP:
	lock
	inc word [cpu_activated]	; BSP adds one here

	xor eax, eax
	mov rsi, [os_LocalAPICAddress]
	add rsi, 0x20			; Add the offset for the APIC ID location
	lodsd				; APIC ID is stored in bits 31:24
	shr rax, 24			; AL now holds the CPU's APIC ID (0 - 255)
	mov rdi, 0x00005700		; The location where the cpu values are stored
	add rdi, rax			; RDI points to infomap CPU area + APIC ID. ex F701 would be APIC ID 1
	mov al, 3			; This is the BSP so bits 0 and 1 are set
	stosb

	mov al, '3'
	mov [0x000B809C], al
	mov al, 'C'
	mov [0x000B809E], al

; Calculate speed of CPU (At this point the timer is firing at 1000Hz)
	cpuid
	xor edx, edx
	xor eax, eax
	mov rcx, [os_Counter]
	add rcx, 10
	rdtsc
	push rax
speedtest:
	mov rbx, [os_Counter]
	cmp rbx, rcx
	jl speedtest
	rdtsc
	pop rdx
	sub rax, rdx
	xor edx, edx
	mov rcx, 10000
	div rcx
	mov [cpu_speed], ax

	mov al, '3'
	mov [0x000B809C], al
	mov al, 'E'
	mov [0x000B809E], al

	cli				; Disable the timer
	
; Set PIT Channel 0 to fire at 100Hz (Divisor = 1193180 / hz)
	mov al, 0x36			; Set Timer
	out 0x43, al
	mov al, 0x9B			; We want 100MHz so 0x2E9B
	out 0x40, al
	mov al, 0x2E
	out 0x40, al

; Disable all IRQs
	in al, 0x21
	mov al, 11111111b
	out 0x21, al
	in al, 0xA1
	mov al, 11111111b
	out 0xA1, al

ret


%include "init_smp_acpi.asm"


; =============================================================================
; EOF
