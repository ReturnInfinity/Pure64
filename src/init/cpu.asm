; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2024 Return Infinity -- see LICENSE.TXT
;
; INIT CPU - This code is called by all activated CPU cores in the system
; =============================================================================


init_cpu:

; Disable Cache
	mov rax, cr0
	btr rax, 29			; Clear No Write Thru (Bit 29)
	bts rax, 30			; Set Cache Disable (Bit 30)
	mov cr0, rax

; Flush Cache
	wbinvd

; Disable Paging Global Extensions
	mov rax, cr4
	btr rax, 7			; Clear Paging Global Extensions (Bit 7)
	mov cr4, rax
	mov rax, cr3
	mov cr3, rax

; Disable MTRRs and Configure default memory type to UC
	mov ecx, 0x000002FF
	rdmsr
	and eax, 0xFFFFF300		; Clear MTRR Enable (Bit 11), Fixed Range MTRR Enable (Bit 10), and Default Memory Type (Bits 7:0) to UC (0x00)
	wrmsr

; Setup variable-size address ranges
; Cache 0-64 MiB as type 6 (WB) cache
; See example in Intel Volume 3A. Example Base and Mask Calculations
;	mov ecx, 0x00000200		; MTRR_Phys_Base_MSR(0)
;	mov edx, 0x00000000		; Base is EDX:EAX, 0x0000000000000006
;	mov eax, 0x00000006		; Type 6 (write-back cache)
;	wrmsr
;	mov ecx, 0x00000201		; MTRR_Phys_Mask_MSR(0)
;;	mov edx, 0x00000000		; Mask is EDX:EAX, 0x0000000001000800 (Because bochs sucks)
;;	mov eax, 0x01000800		; Bit 11 set for Valid
;	mov edx, 0x0000000F		; Mask is EDX:EAX, 0x0000000F80000800 (2 GiB)
;	mov eax, 0x80000800		; Bit 11 set for Valid
;	wrmsr

; MTRR notes:
; Base 0x0000000000000000 = 0 MiB
; Base 0x0000000080000000 = 2048 MiB, 2048 is 0x800
; Base 0x0000000100000000 = 4096 MiB, 4096 is 0x1000
; Mask 0x0000000F80000000 = 2048 MiB, 0xFFFFFFFFF - F80000000 = 7FFFFFFF = 2147483647 (~2 GiB)
; Mask 0x0000000FC0000000 = 1024 MiB, 0xFFFFFFFFF - FC0000000 = 3FFFFFFF = 1073741823 (~1 GiB)
; Mask 0x0000000FFC000000 = 64 MiB,   0xFFFFFFFFF - FFC000000 =  3FFFFFF =   67108863 (~64 MiB)

; Enable MTRRs
	mov ecx, 0x000002FF
	rdmsr
	bts eax, 11			; Set MTRR Enable (Bit 11), Only enables Variable Range MTRR's
	wrmsr

; Flush Cache
	wbinvd

; Enable Cache
	mov rax, cr0
	btr rax, 29			; Clear No Write Thru (Bit 29)
	btr rax, 30			; Clear CD (Bit 30)
	mov cr0, rax

; Enable Paging Global Extensions
;	mov rax, cr4
;	bts rax, 7			; Set Paging Global Extensions (Bit 7)
;	mov cr4, rax

; Enable Floating Point
	mov rax, cr0
	bts rax, 1			; Set Monitor co-processor (Bit 1)
	btr rax, 2			; Clear Emulation (Bit 2)
	mov cr0, rax

; Enable SSE
	mov rax, cr4
	bts rax, 9			; Set Operating System Support for FXSAVE and FXSTOR instructions (Bit 9)
	bts rax, 10			; Set Operating System Support for Unmasked SIMD Floating-Point Exceptions (Bit 10)
	mov cr4, rax

; Enable Math Co-processor
	finit

; Enable AVX
	mov eax, 1			; CPUID Feature information 1
	cpuid				; Sets info in eax and ecx
	bt ecx, 28			; AVX is supported if bit 28 is set in ecx
	jnc avx_not_supported		; Skip activating AVX if not supported
avx_supported:
	mov rax, cr4
	bts rax, 18			; Enable OSXSAVE (Bit 18)
	mov cr4, rax
	mov rcx, 0			; Set load XCR Nr. 0
	xgetbv				; Load XCR0 register
	bts rax, 0			; Set X87 enable (Bit 0)
	bts rax, 1			; Set SSE enable (Bit 1)
	bts rax, 2			; Set AVX enable (Bit 2)
	xsetbv				; Save XCR0 register
avx_not_supported:

; Enable and Configure Local APIC
	mov ecx, APIC_TPR
	mov eax, 0x00000020
	call apic_write			; Disable softint delivery
	mov ecx, APIC_LVT_TMR
	mov eax, 0x00010000
	call apic_write			; Disable timer interrupts
	mov ecx, APIC_LVT_PERF
	mov eax, 0x00010000
	call apic_write			; Disable performance counter interrupts
	mov ecx, APIC_LDR
	xor eax, eax
	call apic_write			; Set Logical Destination Register
	mov ecx, APIC_DFR
	not eax				; Set EAX to 0xFFFFFFFF; Bits 31-28 set for Flat Mode
	call apic_write			; Set Destination Format Register
	mov ecx, APIC_LVT_LINT0
	mov eax, 0x00008700		; Bit 15 (1 = Level), Bits 10:8 for Ext
	call apic_write			; Enable normal external interrupts
	mov ecx, APIC_LVT_LINT1
	mov eax, 0x00000400
	call apic_write			; Enable normal NMI processing
	mov ecx, APIC_LVT_ERR
	mov eax, 0x00010000
	call apic_write			; Disable error interrupts
	mov ecx, APIC_SPURIOUS
	mov eax, 0x000001FF
	call apic_write			; Enable the APIC (bit 8) and set spurious vector to 0xFF

	lock inc word [cpu_activated]
	mov ecx, APIC_ID
	call apic_read			; APIC ID is stored in bits 31:24
	shr rax, 24			; AL now holds the CPU's APIC ID (0 - 255)
	mov rdi, 0x00005700		; The location where the cpu values are stored
	add rdi, rax			; RDI points to InfoMap CPU area + APIC ID. ex 0x5701 would be APIC ID 1
	mov al, 1
	stosb

	ret

; -----------------------------------------------------------------------------
; apic_read -- Read from a register in the APIC
;  IN:	ECX = Register to read
; OUT:	EAX = Register value
;	All other registers preserved
apic_read:
	push rsi
	mov rsi, [os_LocalAPICAddress]
	add rsi, rcx			; Add offset
	lodsd
	pop rsi
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; apic_write -- Write to a register in the APIC
;  IN:	ECX = Register to write
;	EAX = Value to write
; OUT:	All registers preserved
apic_write:
	push rdi
	mov rdi, [os_LocalAPICAddress]
	add rdi, rcx			; Add offset
	stosd
	pop rdi
	ret
; -----------------------------------------------------------------------------


; Register list
; 0x000 - 0x010 are Reserved
APIC_ID		equ 0x020		; ID Register
APIC_VER	equ 0x030		; Version Register
; 0x040 - 0x070 are Reserved
APIC_TPR	equ 0x080		; Task Priority Register
APIC_APR	equ 0x090		; Arbitration Priority Register
APIC_PPR	equ 0x0A0		; Processor Priority Register
APIC_EOI	equ 0x0B0		; End Of Interrupt
APIC_RRD	equ 0x0C0		; Remote Read Register
APIC_LDR	equ 0x0D0		; Logical Destination Register
APIC_DFR	equ 0x0E0		; Destination Format Register
APIC_SPURIOUS	equ 0x0F0		; Spurious Interrupt Vector Register
APIC_ISR	equ 0x100		; In-Service Register (Starting Address)
APIC_TMR	equ 0x180		; Trigger Mode Register (Starting Address)
APIC_IRR	equ 0x200		; Interrupt Request Register (Starting Address)
APIC_ESR	equ 0x280		; Error Status Register
; 0x290 - 0x2E0 are Reserved
APIC_ICRL	equ 0x300		; Interrupt Command Register (low 32 bits)
APIC_ICRH	equ 0x310		; Interrupt Command Register (high 32 bits)
APIC_LVT_TMR	equ 0x320		; LVT Timer Register
APIC_LVT_TSR	equ 0x330		; LVT Thermal Sensor Register
APIC_LVT_PERF	equ 0x340		; LVT Performance Monitoring Counters Register
APIC_LVT_LINT0	equ 0x350		; LVT LINT0 Register
APIC_LVT_LINT1	equ 0x360		; LVT LINT1 Register
APIC_LVT_ERR	equ 0x370		; LVT Error Register
APIC_TMRINITCNT	equ 0x380		; Initial Count Register (for Timer)
APIC_TMRCURRCNT	equ 0x390		; Current Count Register (for Timer)
; 0x3A0 - 0x3D0 are Reserved
APIC_TMRDIV	equ 0x3E0		; Divide Configuration Register (for Timer)
; 0x3F0 is Reserved


; =============================================================================
; EOF
