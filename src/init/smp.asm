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

	; Check for APIC support
	mov eax, 1
	cpuid
	bt edx, 9			; APIC bit should be set
	jnc noMP			; If APIC bit is not set then skip SMP init

	; Enable the APIC
	mov ecx, 0x1B			; APIC_BASE MSR
	rdmsr				; Read MSR to EDX:EAX
	bts eax, 11			; EN
	wrmsr				; Write EDX:EAX to MSR

	; Check for x2APIC support
	mov eax, 1
	cpuid
	bt ecx, 21			; x2APIC bit might be set
	jnc init_smp_apic		; If not, continue to APIC SMP init

	; Enable the x2APIC
	mov ecx, 0x1B			; APIC_BASE MSR
	rdmsr				; Read MSR to EDX:EAX
	bts eax, 10			; EXTD
	wrmsr				; Write EDX:EAX to MSR

	mov byte [p_x2APIC], 1		; Set flag for x2APIC support

init_smp_x2apic:
	; Start the AP's one by one
	xor eax, eax
	xor edx, edx
	mov ecx, 0x00000802		; x2APIC Local APIC ID Register MSR
	rdmsr
	mov ebx, eax			; Store BSP APIC ID in EBX
	mov [p_BSP], eax		; Store the BSP APIC ID

	mov esi, IM_DetectedCoreIDs
	xor eax, eax
	xor ecx, ecx
	mov cx, [p_cpu_detected]
init_smp_x2apic_INIT:
	cmp cx, 0
	je init_smp_x2apic_INIT_done
	lodsd

	cmp eax, ebx			; Is it the BSP?
	je init_smp_x2apic_INIT_skipcore

	; Send 'INIT' IPI to APIC ID in EAX
	push rcx
	mov ecx, 0x00000830		; x2APIC Interrupt Command Register (ICR) MSR
	mov edx, eax
	mov eax, 0x00004500
	wrmsr
init_smp_x2apic_INIT_verify:
	rdmsr
	bt eax, 12			; Verify that the command completed
	jc init_smp_x2apic_INIT_verify
	pop rcx

init_smp_x2apic_INIT_skipcore:
	dec cx
	jmp init_smp_x2apic_INIT

init_smp_x2apic_INIT_done:

	; Wait 500 microseconds
	mov eax, 500
	call os_hpet_delay

	mov esi, IM_DetectedCoreIDs
	xor ecx, ecx
	mov cx, [p_cpu_detected]
init_smp_x2apic_SIPI:
	cmp cx, 0
	je init_smp_x2apic_SIPI_done
	lodsd

	cmp eax, ebx			; Is it the BSP?
	je init_smp_x2apic_SIPI_skipcore

	; Send 'Startup' IPI to destination using vector 0x08 to specify entry-point is at the memory-address 0x00008000
	push rcx
	mov ecx, 0x00000830		; x2APIC Interrupt Command Register (ICR) MSR
	mov edx, eax
	mov eax, 0x00004608
	wrmsr
init_smp_x2apic_SIPI_verify:
	rdmsr
	bt eax, 12			; Verify that the command completed
	jc init_smp_x2apic_SIPI_verify
	pop rcx

init_smp_x2apic_SIPI_skipcore:
	dec cx
	jmp init_smp_x2apic_SIPI

init_smp_x2apic_SIPI_done:

	; Wait 10000 microseconds for the AP's to finish
	mov eax, 10000
	call os_hpet_delay

	jmp noMP

init_smp_apic:
	; Start the AP's one by one
	xor eax, eax
	xor edx, edx
	mov rsi, [p_LocalAPICAddress]
	mov eax, [rsi+0x20]		; Add the offset for the APIC ID location
	shr eax, 24			; APIC ID is stored in bits 31:24
	mov ebx, eax			; Store BSP APIC ID in EBX
	mov [p_BSP], eax		; Store the BSP APIC ID

	mov esi, IM_DetectedCoreIDs
	xor eax, eax
	xor ecx, ecx
	mov cx, [p_cpu_detected]
smp_send_INIT:
	cmp cx, 0
	je smp_send_INIT_done
	lodsd

	cmp eax, ebx			; Is it the BSP?
	je smp_send_INIT_skipcore

	; Send 'INIT' IPI to APIC ID in AL
	mov rdi, [p_LocalAPICAddress]
	shl eax, 24
	mov dword [rdi+0x310], eax	; Interrupt Command Register (ICR); bits 63-32
	mov eax, 0x00004500
	mov dword [rdi+0x300], eax	; Interrupt Command Register (ICR); bits 31-0
smp_send_INIT_verify:
	mov eax, [rdi+0x300]		; Interrupt Command Register (ICR); bits 31-0
	bt eax, 12			; Verify that the command completed
	jc smp_send_INIT_verify

smp_send_INIT_skipcore:
	dec cl
	jmp smp_send_INIT

smp_send_INIT_done:

	; Wait 500 microseconds
	mov eax, 500
	call os_hpet_delay

	mov esi, IM_DetectedCoreIDs
	xor ecx, ecx
	mov cx, [p_cpu_detected]
smp_send_SIPI:
	cmp cx, 0
	je smp_send_SIPI_done
	lodsd

	cmp eax, ebx			; Is it the BSP?
	je smp_send_SIPI_skipcore

	; Send 'Startup' IPI to destination using vector 0x08 to specify entry-point is at the memory-address 0x00008000
	mov rdi, [p_LocalAPICAddress]
	shl eax, 24
	mov dword [rdi+0x310], eax	; Interrupt Command Register (ICR); bits 63-32
	mov eax, 0x00004608		; Vector 0x08
	mov dword [rdi+0x300], eax	; Interrupt Command Register (ICR); bits 31-0
smp_send_SIPI_verify:
	mov eax, [rdi+0x300]		; Interrupt Command Register (ICR); bits 31-0
	bt eax, 12			; Verify that the command completed
	jc smp_send_SIPI_verify

smp_send_SIPI_skipcore:
	dec cl
	jmp smp_send_SIPI

smp_send_SIPI_done:

	; Wait 10000 microseconds for the AP's to finish
	mov eax, 10000
	call os_hpet_delay

noMP:
	; Calculate base speed of CPU
	cpuid
	xor edx, edx
	xor eax, eax
	rdtsc
	push rax
	mov rax, 1024
	call os_hpet_delay
	rdtsc
	pop rdx
	sub rax, rdx
	xor edx, edx
	mov rcx, 1024
	div rcx
	mov [p_cpu_speed], ax

	ret


; =============================================================================
; EOF
