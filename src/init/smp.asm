; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; INIT SMP
; =============================================================================


init_smp:
	; Check if we want the AP's to be enabled.. if not then skip to end
	cmp byte [cfg_smpinit], 1	; Check if SMP should be enabled
	jne noMP			; If not then skip SMP init

	mov edx, [p_BSP]		; Get the BSP APIC ID
	mov esi, IM_DetectedCoreIDs	; List of 32-bit APIC IDs
	xor eax, eax
	xor ecx, ecx
	mov cx, [p_cpu_detected]
smp_send_INIT:
	cmp cx, 0
	je smp_send_INIT_done
	lodsd

	cmp eax, edx			; Is it the BSP?
	je smp_send_INIT_skipcore	; If so, skip

	cmp byte [p_x2APIC], 1
	je smp_send_INIT_x2APIC

smp_send_INIT_APIC:
	; Send 'INIT' IPI to APIC ID in AL
	push rcx			; Save counter
	shl eax, 24
	mov ecx, APIC_ICRH		; Interrupt Command Register (ICR); bits 63-32
	call apic_write
	mov eax, 0x00004500
	mov ecx, APIC_ICRL		; Interrupt Command Register (ICR); bits 31-0
	call apic_write
smp_send_INIT_verify:
	call apic_read
	bt eax, 12			; Verify that the command completed
	jc smp_send_INIT_verify
	pop rcx				; Restore counter
	jmp smp_send_INIT_APIC_done

smp_send_INIT_x2APIC:
	; Send 'INIT' IPI to APIC ID in AL
	push rcx			; Save counter
	mov ecx, APIC_ICR		; Interrupt Command Register (ICR); bits 63-0
	shl rax, 32
	mov ax, 0x4500
	call apic_write
	pop rcx				; Restore counter

smp_send_INIT_APIC_done:

smp_send_INIT_skipcore:
	dec cl
	jmp smp_send_INIT

smp_send_INIT_done:

	; Wait
	mov eax, 500			; 500 microseconds (0.5ms)
	call timer_delay

	mov esi, IM_DetectedCoreIDs
	xor ecx, ecx
	mov cx, [p_cpu_detected]
smp_send_SIPI:
	cmp cx, 0
	je smp_send_SIPI_done
	lodsd

	cmp eax, edx			; Is it the BSP?
	je smp_send_SIPI_skipcore

	cmp byte [p_x2APIC], 1
	je smp_send_SIPI_x2APIC

smp_send_SIPI_APIC:
	; Send 'Startup' IPI to destination using vector 0x08 to specify entry-point is at the memory-address 0x00008000
	push rcx
	shl eax, 24
	mov ecx, APIC_ICRH		; Interrupt Command Register (ICR); bits 63-32
	call apic_write
	mov eax, 0x00004608		; Vector 0x08
	mov ecx, APIC_ICRL		; Interrupt Command Register (ICR); bits 31-0
	call apic_write
smp_send_SIPI_verify:
	call apic_read
	bt eax, 12			; Verify that the command completed
	jc smp_send_SIPI_verify
	pop rcx
	jmp smp_send_SIPI_APIC_done

smp_send_SIPI_x2APIC:
	; Send 'Startup' IPI to destination using vector 0x08 to specify entry-point is at the memory-address 0x00008000
	push rcx
	mov ecx, APIC_ICR		; Interrupt Command Register (ICR); bits 63-0
	shl rax, 32
	mov ax, 0x4608			; Vector 0x08
	call apic_write
	pop rcx

smp_send_SIPI_APIC_done:

smp_send_SIPI_skipcore:
	dec cl
	jmp smp_send_SIPI

smp_send_SIPI_done:

	; Wait for the AP's to finish
	mov eax, 10000		; 10000 microseconds (10ms)
	call timer_delay

noMP:

;	; Calculate base speed of CPU
;	cpuid
;	xor edx, edx
;	xor eax, eax
;	rdtsc
;	push rax
;	mov rax, 1024		; 1024 microseconds (1ms)
;	call timer_delay
;	rdtsc
;	pop rdx
;	sub rax, rdx
;	xor edx, edx
;	mov rcx, 1024
;	div rcx
;	mov [p_cpu_speed], ax

	ret


; =============================================================================
; EOF
