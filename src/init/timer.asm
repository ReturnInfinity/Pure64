; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; INIT TIMER
; =============================================================================


init_timer:
	; Check for hypervisor presence
	mov eax, 1
	cpuid
	bt ecx, 31			; HV - hypervisor present
	jnc init_timer_phys		; If bit is clear then jump to phys init

	; Check for hypervisor type
	mov eax, 0x40000000
	cpuid
	cmp ebx, 0x4B4D564B		; KMVK - KVM
	je init_timer_virt		; KVM detected? Then init_virt
	; If not, fall through to init_phys

init_timer_phys:
	; Verify there is a valid HPET address
	mov rax, [p_HPET_Address]
	cmp rax, 0
	jz init_timer_error
	; Initialize the HPET
	call init_timer_hpet
	mov qword [sys_timer], hpet_delay
	jmp init_timer_done

init_timer_virt:
	; Initialize the KVM timer
	call init_timer_kvm
	mov qword [sys_timer], kvm_delay
	jmp init_timer_done

init_timer_error:
	jmp $				; Spin forever as there was no timer source

init_timer_done:
	ret


; -----------------------------------------------------------------------------
; init_timer_hpet -- Initialize the HPET
init_timer_hpet:
	; Verify the capabilities of HPET
	mov ecx, HPET_GEN_CAP
	call hpet_read
	mov rbx, rax			; Save results for # of timers
	shr ebx, 8			; Bits 12:8 contain the # of timers
	and ebx, 11111b			; Save only the lower 5 bits
	inc bl				; Increment the number of timers by 1
	mov [p_HPET_Timers], bl		; Save the # of HPET timers
	shr rax, 32			; EAX contains the tick period in femtoseconds

	; Verify the Counter Clock Period is valid
	cmp eax, 0x05F5E100		; 100,000,000 femtoseconds is the maximum
	ja init_timer_hpet_error
	cmp eax, 0			; The Period has to be greater than 1 femtosecond
	je init_timer_hpet_error

	; Calculate the HPET frequency
	mov rbx, rax			; Move Counter Clock Period to RBX
	xor rdx, rdx
	mov rax, 1000000000000000	; femotoseconds per second
	div rbx				; RDX:RAX / RBX
	mov [p_HPET_Frequency], eax	; Save the HPET frequency

	; Disable interrupts on all timers
	xor ebx, ebx
	mov bl, [p_HPET_Timers]
	mov ecx, 0xE0			; HPET_TIMER_0_CONF - 0x20
init_timer_hpet_disable_int:
	add ecx, 0x20
	call hpet_read
	btc ax, 2
	btc ax, 3
	call hpet_write
	dec bl
	jnz init_timer_hpet_disable_int

	; Clear the main counter before it is enabled
	mov ecx, HPET_MAIN_COUNTER
	xor eax, eax
	call hpet_write

	; Enable HPET main counter (bit 0)
	mov eax, 1			; Bit 0 is set
	mov ecx, HPET_GEN_CONF
	call hpet_write

	jmp init_timer_hpet_done

init_timer_hpet_error:

init_timer_hpet_done:
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; hpet_read -- Read from a register in the High Precision Event Timer
;  IN:	ECX = Register to read
; OUT:	RAX = Register value
;	All other registers preserved
hpet_read:
	mov rax, [p_HPET_Address]
	mov rax, [rax + rcx]
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; hpet_write -- Write to a register in the High Precision Event Timer
;  IN:	ECX = Register to write
;	RAX = Value to write
; OUT:	All registers preserved
hpet_write:
	push rcx
	add rcx, [p_HPET_Address]
	mov [rcx], rax
	pop rcx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; hpet_delay -- Delay by X microseconds
; IN:	RAX = Time microseconds
; OUT:	All registers preserved
; Note:	There are 1,000,000 microseconds in a second
;	There are 1,000 milliseconds in a second
hpet_delay:
	push rdx
	push rcx
	push rbx
	push rax

	mov rbx, rax			; Save delay to RBX
	xor edx, edx
	xor ecx, ecx
	call hpet_read			; Get HPET General Capabilities and ID Register
	shr rax, 32
	mov rcx, rax			; RCX = RAX >> 32 (timer period in femtoseconds)
	mov rax, 1000000000
	div rcx				; Divide 1000000000 (RDX:RAX) / RCX (converting from period in femtoseconds to frequency in MHz)
	mul rbx				; RAX *= RBX, should get number of HPET cycles to wait, save result in RBX
	mov rbx, rax
	mov ecx, HPET_MAIN_COUNTER
	call hpet_read			; Get HPET counter in RAX
	add rbx, rax			; RBX += RAX Until when to wait
hpet_delay_loop:			; Stay in this loop until the HPET timer reaches the expected value
	mov ecx, HPET_MAIN_COUNTER
	call hpet_read			; Get HPET counter in RAX
	cmp rax, rbx			; If RAX >= RBX then jump to end, otherwise jump to loop
	jae hpet_delay_end
	jmp hpet_delay_loop
hpet_delay_end:

	pop rax
	pop rbx
	pop rcx
	pop rdx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; init_timer_kvm - Initialize the KVM timer
init_timer_kvm:
	; Check hypervisor feature bits
	mov eax, 0x40000001
	cpuid
	bt eax, 3
	jc init_timer_kvm_clocksource2
	bt eax, 0
	jc init_timer_kvm_clocksource
	jmp init_timer_done

init_timer_kvm_clocksource2:
	mov ecx, MSR_KVM_SYSTEM_TIME_NEW
	jmp init_timer_kvm_configure

init_timer_kvm_clocksource:
	mov ecx, MSR_KVM_SYSTEM_TIME

init_timer_kvm_configure:
	xor edx, edx
	mov eax, p_timer		; Memory address for structure
	bts eax, 0			; Enable bit
	wrmsr

	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; kvm_get_usec -- Returns # of microseconds elapsed since guest start
; IN:	Nothing
; OUT:	RAX = microseconds elapsed since start
;	All other registers preserved
kvm_get_usec:
	push r10
	push r9
	push r8
	push rdi
	push rdx
	push rcx
	push rbx

	mov rdi, p_timer
kvm_get_usec_wait:
	mov r10d, [rdi]			; Get 32-bit version
	test r10d, 1			; Check if version is odd (update in progress)
	jnz kvm_get_usec_wait		; If so, retry

	lfence

	rdtsc				; Read CPU TSC into EDX:EAX
	shl rdx, 32
	or rax, rdx			; Combine EDX:EAX into RAX
	mov r9, rax			; Save the 64-bit TSC value

	; Load KVM timer data
	mov rax, [rdi+0x08]		; 64-bit tsc_timestamp
	mov rbx, [rdi+0x10]		; 64-bit system_time
	mov ecx, [rdi+0x18]		; 32-bit tsc_to_system_mul
	movzx r8, byte [rdi+0x1C]	; 8-bit tsc_shift

	; Reorg register usage
	push rcx			; Save tsc_to_system_mul to stack
	mov ecx, r8d			; Copy tsc_shift to ECX

	; Calculate timer delta (CPU TSC - tsc_timestamp)
	sub r9, rax
	mov rax, r9

	; Apply tsc_shift
	cmp cl, 0
	jl kvm_get_usec_shift_right	; Signed comparison
	shl rax, cl
	jmp kvm_get_usec_shift_done
kvm_get_usec_shift_right:
	neg cl				; Ex: 0xFF = tsc shift of -1
	shr rax, cl
kvm_get_usec_shift_done:

	pop rcx				; Restore tsc_to_system_mul

	; Calculate nanoseconds as (delta * mul) >> 32
	mul rcx				; RDX:RAX = RAX * RCX
	shl rdx, 32
	shr rax, 32
	or rax, rdx

	; Add system time to nanoseconds
	add rax, rbx

	; Recheck struct version
	lfence
	mov ecx, [rdi]			; Load 32-bit version
	cmp r10d, ecx			; Compare to first version read
	jne kvm_get_usec_wait		; If not equal then an update occured, restart

	; Convert nanoseconds to microseconds
	xor edx, edx
	mov ecx, 1000
	div rcx

	pop rbx
	pop rcx
	pop rdx
	pop rdi
	pop r8
	pop r9
	pop r10
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; kvm_delay -- Delay by X microseconds
; IN:	RAX = Time microseconds
; OUT:	All registers preserved
; Note:	There are 1,000,000 microseconds in a second
;	There are 1,000 milliseconds in a second
kvm_delay:
	push rbx
	push rax

	mov rbx, rax			; Store delay in RBX
	call kvm_get_usec
	add rbx, rax			; Add elapsed time
kvm_delay_wait:
	call kvm_get_usec
	cmp rax, rbx
	jb kvm_delay_wait

	pop rax
	pop rbx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; timer_delay -- Delay by X microseconds
; IN:	RAX = Time microseconds
; OUT:	All registers preserved
; Note:	There are 1,000,000 microseconds in a second
;	There are 1,000 milliseconds in a second
timer_delay:
	push rax

	call [sys_timer]

	pop rax
	ret
; -----------------------------------------------------------------------------


; Register list (64-bits wide)
HPET_GEN_CAP		equ 0x000 ; COUNTER_CLK_PERIOD (63:32), LEG_RT_CAP (15), COUNT_SIZE_CAP (13), NUM_TIM_CAP (12:8)
; 0x008 - 0x00F are Reserved
HPET_GEN_CONF		equ 0x010 ; LEG_RT_CNF (1), ENABLE_CNF (0)
; 0x018 - 0x01F are Reserved
HPET_GEN_INT_STATUS	equ 0x020
; 0x028 - 0x0EF are Reserved
HPET_MAIN_COUNTER	equ 0x0F0
; 0x0F8 - 0x0FF are Reserved
HPET_TIMER_0_CONF	equ 0x100
HPET_TIMER_0_COMP	equ 0x108
HPET_TIMER_0_INT	equ 0x110
; 0x118 - 0x11F are Reserved
HPET_TIMER_1_CONF	equ 0x120
HPET_TIMER_1_COMP	equ 0x128
HPET_TIMER_1_INT	equ 0x130
; 0x138 - 0x13F are Reserved
HPET_TIMER_2_CONF	equ 0x140
HPET_TIMER_2_COMP	equ 0x148
HPET_TIMER_2_INT	equ 0x150
; 0x158 - 0x15F are Reserved
; 0x160 - 0x3FF are Reserved for Timers 3-31


; MSRs
MSR_KVM_SYSTEM_TIME_NEW	equ 0x4B564D01
MSR_KVM_SYSTEM_TIME	equ 0x00000012

; KVM pvclock structure
pvclock_version		equ 0x00 ; 32-bit
pvclock_tsc_timestamp	equ 0x08 ; 64-bit
pvclock_system_time	equ 0x10 ; 64-bit
pvclock_tsc_system_mul	equ 0x18 ; 32-bit
pvclock_tsc_shift	equ 0x1C ; 8-bit
pvclock_flags		equ 0x1D ; 8-bit



; =============================================================================
; EOF
