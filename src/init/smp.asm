; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2023 Return Infinity -- see LICENSE.TXT
;
; INIT SMP
; =============================================================================


init_smp:
; Check if we want the AP's to be enabled.. if not then skip to end
	cmp byte [cfg_smpinit], 1	; Check if SMP should be enabled
	jne noMP			; If not then skip SMP init

; Start the AP's one by one
	xor eax, eax
	xor edx, edx
	mov rsi, [os_LocalAPICAddress]
	mov eax, [rsi+0x20]		; Add the offset for the APIC ID location
	shr rax, 24			; APIC ID is stored in bits 31:24
	mov dl, al			; Store BSP APIC ID in DL

	mov esi, 0x00005100
	xor eax, eax
	xor ecx, ecx
	mov cx, [cpu_detected]
smp_send_INIT:
	cmp cx, 0
	je smp_send_INIT_done
	lodsb

	cmp al, dl			; Is it the BSP?
	je smp_send_INIT_skipcore

	; Send 'INIT' IPI to APIC ID in AL
	mov rdi, [os_LocalAPICAddress]
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

	mov rax, [os_Counter_RTC]
	add rax, 10
smp_wait1:
	mov rbx, [os_Counter_RTC]
	cmp rax, rbx
	jg smp_wait1

	mov esi, 0x00005100
	xor ecx, ecx
	mov cx, [cpu_detected]
smp_send_SIPI:
	cmp cx, 0
	je smp_send_SIPI_done
	lodsb

	cmp al, dl			; Is it the BSP?
	je smp_send_SIPI_skipcore

	; Send 'Startup' IPI to destination using vector 0x08 to specify entry-point is at the memory-address 0x00008000
	mov rdi, [os_LocalAPICAddress]
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

; Let things settle (Give the AP's some time to finish)
	mov rax, [os_Counter_RTC]
	add rax, 20
smp_wait2:
	mov rbx, [os_Counter_RTC]
	cmp rax, rbx
	jg smp_wait2

; Finish up
noMP:
	lock inc word [cpu_activated]	; BSP adds one here

	xor eax, eax
	mov rsi, [os_LocalAPICAddress]
	add rsi, 0x20			; Add the offset for the APIC ID location
	lodsd				; APIC ID is stored in bits 31:24
	shr rax, 24			; AL now holds the CPU's APIC ID (0 - 255)
	mov [os_BSP], eax		; Store the BSP APIC ID

; Calculate speed of CPU (At this point the RTC is firing at 1024Hz)
	cpuid
	xor edx, edx
	xor eax, eax
	mov rcx, [os_Counter_RTC]
	add rcx, 10
	rdtsc
	push rax
speedtest:
	mov rbx, [os_Counter_RTC]
	cmp rbx, rcx
	jl speedtest
	rdtsc
	pop rdx
	sub rax, rdx
	xor edx, edx
	mov rcx, 10240
	div rcx
	mov [cpu_speed], ax

	cli				; Disable Interrupts

	ret


; =============================================================================
; EOF
