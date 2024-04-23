; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2024 Return Infinity -- see LICENSE.TXT
;
; The first stage loader is required to gather information about the system
; while the BIOS or UEFI is still available and load the Pure64 binary to
; 0x00008000. Setup a minimal 64-bit environment, copy the 64-bit kernel from
; the end of the Pure64 binary to the 1MiB memory mark and jump to it.
;
; Pure64 requires a payload for execution! The stand-alone pure64.sys file
; is not sufficient. You must append your kernel or software to the end of
; the Pure64 binary. The maximum size of the kernel or software is 28KiB.
;
; Windows - copy /b pure64.sys + kernel64.sys
; Unix - cat pure64.sys kernel64.sys > pure64.sys
; Max size of the resulting pure64.sys is 32768 bytes (32KiB)
; =============================================================================


BITS 64
ORG 0x00008000
PURE64SIZE equ 4096			; Pad Pure64 to this length

start:
	jmp bootmode			; This command will be overwritten with 'NOP's before the AP's are started
	nop
	db 0x36, 0x34			; '64' marker

; =============================================================================
; Code for AP startup
BITS 16
	cli				; Disable all interrupts
	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	xor esi, esi
	xor edi, edi
	xor ebp, ebp
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov esp, 0x8000			; Set a known free location for the stack

%include "init/smp_ap.asm"		; AP's will start execution at 0x8000 and fall through to this code

; =============================================================================
; This is 32-bit code so it's important that the encoding of the first few instructions also
; work in 64-bit mode. If a 'U' is stored at 0x5FFF then we know it was a UEFI boot and can
; immediately proceed to start64. Otherwise we need to set up a minimal 64-bit environment.
BITS 32
bootmode:
	cmp bl, 'U'			; If it is 'U' then we booted via UEFI and are already in 64-bit mode for the BSP
	je start64			; Jump to the 64-bit code, otherwise fall through to 32-bit init

	mov eax, 16			; Set the correct segment registers
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	xor eax, eax			; Clear all registers
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	xor esi, esi
	xor edi, edi
	xor ebp, ebp
	mov esp, 0x8000			; Set a known free location for the stack

	; Save the frame buffer address, size (after its calculated), and the screen x,y
	mov bx, [0x5F00 + 20]
	push ebx
	mov ax, [0x5F00 + 18]
	push eax
	mul ebx
	mov ecx, eax
	shl ecx, 2			; Quick multiply by 4
	mov edi, 0x5F00
	mov eax, [0x5F00 + 40]
	stosd				; 64-bit Frame Buffer Base (low)
	xor eax, eax
	stosd				; 64-bit Frame Buffer Base (high)
	mov eax, ecx
	stosd				; 64-bit Frame Buffer Size in bytes (low)
	xor eax, eax
	stosd				; 64-bit Frame Buffer Size in bytes (high)
	pop eax
	mov ecx, eax			; Save Screen X
	stosw				; 16-bit Screen X
	pop eax
	stosw				; 16-bit Screen Y
	mov eax, ecx			; Restore Screen X
	stosd				; PixelsPerScanLine

	; Clear memory for the Page Descriptor Entries (0x10000 - 0x5FFFF)
	mov edi, 0x00210000
	mov ecx, 81920
	rep stosd			; Write 320KiB

; Create the temporary Page Map Level 4 Entries (PML4E)
; PML4 is stored at 0x0000000000202000, create the first entry there
; A single PML4 entry can map 512GiB with 2MiB pages
; A single PML4 entry is 8 bytes in length
	cld
	mov edi, 0x00202000		; Create a PML4 entry for the first 4GiB of RAM
	mov eax, 0x00203007		; Bits 0 (P), 1 (R/W), 2 (U/S), location of low PDP (4KiB aligned)
	stosd
	xor eax, eax
	stosd

; Create the temporary Page-Directory-Pointer-Table Entries (PDPTE)
; PDPTE is stored at 0x0000000000203000, create the first entry there
; A single PDPTE can map 1GiB with 2MiB pages
; A single PDPTE is 8 bytes in length
; 4 entries are created to map the first 4GiB of RAM
	mov ecx, 4			; number of PDPE's to make.. each PDPE maps 1GiB of physical memory
	mov edi, 0x00203000		; location of low PDPE
	mov eax, 0x00210007		; Bits 0 (P), 1 (R/W), 2 (U/S), location of first low PD (4KiB aligned)
pdpte_low_32:
	stosd
	push eax
	xor eax, eax
	stosd
	pop eax
	add eax, 0x00001000		; 4KiB later (512 records x 8 bytes)
	dec ecx
	cmp ecx, 0
	jne pdpte_low_32

; Create the temporary low Page-Directory Entries (PDE).
; A single PDE can map 2MiB of RAM
; A single PDE is 8 bytes in length
	mov edi, 0x00210000		; Location of first PDE
	mov eax, 0x0000008F		; Bits 0 (P), 1 (R/W), 2 (U/S), 3 (PWT), and 7 (PS) set
	xor ecx, ecx
pde_low_32:				; Create a 2 MiB page
	stosd
	push eax
	xor eax, eax
	stosd
	pop eax
	add eax, 0x00200000		; Increment by 2MiB
	inc ecx
	cmp ecx, 2048
	jne pde_low_32			; Create 2048 2 MiB page maps.

; Load the GDT
	lgdt [tGDTR64]

; Enable extended properties
	mov eax, cr4
	or eax, 0x0000000B0		; PGE (Bit 7), PAE (Bit 5), and PSE (Bit 4)
	mov cr4, eax

; Point cr3 at PML4
	mov eax, 0x00202008		; Write-thru enabled (Bit 3)
	mov cr3, eax

; Enable long mode and SYSCALL/SYSRET
	mov ecx, 0xC0000080		; EFER MSR number
	rdmsr				; Read EFER
	or eax, 0x00000101 		; LME (Bit 8)
	wrmsr				; Write EFER

	mov bl, 'B'

; Enable paging to activate long mode
	mov eax, cr0
	or eax, 0x80000000		; PG (Bit 31)
	mov cr0, eax

	jmp SYS64_CODE_SEL:start64	; Jump to 64-bit mode


; =============================================================================
; 64-bit mode
BITS 64
start64:
	mov esp, 0x8000			; Set a known free location for the stack

	mov edi, 0x5000			; Clear the info map and system variable memory
	xor eax, eax
	mov ecx, 960			; 3840 bytes (Range is 0x5000 - 0x5EFF)
	rep stosd			; Don't overwrite the UEFI/BIOS data at 0x5F00

	mov [p_BootMode], bl

; Debug
	mov eax, 0x007F7F7F
	mov ebx, 0
	call debug_block

; Set up RTC
; Port 0x70 is RTC Address, and 0x71 is RTC Data
; http://www.nondot.org/sabre/os/files/MiscHW/RealtimeClockFAQ.txt
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
	mov al, 01000000b		; Set Periodic Interrupt Enable (bit 6)
	out 0x71, al			; Write the data
	mov al, 0x0C			; Status Register B
	out 0x70, al			; Select the address
	in al, 0x71			; Read the value to clear any existing interrupt value

; Remap PIC IRQ's
	mov al, 00010001b		; begin PIC 1 initialization
	out 0x20, al
	mov al, 00010001b		; begin PIC 2 initialization
	out 0xA0, al
	mov al, 0x20			; IRQ 0-7: interrupts 20h-27h
	out 0x21, al
	mov al, 0x28			; IRQ 8-15: interrupts 28h-2Fh
	out 0xA1, al
	mov al, 4
	out 0x21, al
	mov al, 2
	out 0xA1, al
	mov al, 1
	out 0x21, al
	out 0xA1, al

; Mask all PIC interrupts
	mov al, 0xFF
	out 0x21, al
	out 0xA1, al

; Configure serial port @ 0x03F8 as 115200 8N1
	mov dx, 0x03F8 + 1		; Interrupt Enable
	mov al, 0x00			; Disable all interrupts
	out dx, al
	mov dx, 0x03F8 + 3		; Line Control
	mov al, 80			; Enable DLAB
	out dx, al
	mov dx, 0x03F8 + 0		; Divisor Latch Low
	mov al, 1			; 1 = 115200 baud
	out dx, al
	mov dx, 0x03F8 + 1		; Divisor Latch High
	mov al, 0
	out dx, al
	mov dx, 0x03F8 + 3		; Line Control
	mov al, 3			; 8 data bits (0-1 set), one stop bit (2 set), no parity (3-5 clear), DLB (7 clear)
	out dx, al
	mov dx, 0x03F8 + 2		; Interrupt Identification and FIFO Control
	mov al, 0xC7			; Enable FIFO, clear them, with 14-byte threshold
	out dx, al
	mov dx, 0x03F8 + 4		; Modem Control
	mov al, 0			; No flow control, no interrupts
	out dx, al

	mov rsi, message_pure64		; Location of message
	call debug_msg

;	mov al, 'a'			; Newline
;	call debug_msg_char

; Clear out the first 20KiB of memory. This will store the 64-bit IDT, GDT, PML4, PDP Low, and PDP High
	mov ecx, 5120
	xor eax, eax
	mov edi, eax
	rep stosd

; Clear memory for the Page Descriptor Entries (0x10000 - 0x5FFFF)
	mov edi, 0x00010000
	mov ecx, 81920
	rep stosd			; Write 320KiB

; Copy the GDT to its final location in memory
	mov esi, gdt64
	mov edi, 0x00001000		; GDT address
	mov ecx, (gdt64_end - gdt64)
	rep movsb			; Copy it to final location

; Create the Page Map Level 4 Entries (PML4E)
; PML4 is stored at 0x0000000000002000, create the first entry there
; A single PML4 entry can map 512GiB
; A single PML4 entry is 8 bytes in length
	mov edi, 0x00002000		; Create a PML4 entry for physical memory
	mov eax, 0x00003007		; Bits 0 (P), 1 (R/W), 2 (U/S), location of low PDP (4KiB aligned)
	stosq
	mov edi, 0x00002800		; Create a PML4 entry for higher half (starting at 0xFFFF800000000000)
	mov eax, 0x00004007		; Bits 0 (P), 1 (R/W), 2 (U/S), location of high PDP (4KiB aligned)
	stosq

; Check to see if the system supports 1 GiB pages
; If it does we will use that for identity mapping the lower memory
	mov eax, 0x80000001
	cpuid
	bt edx, 26			; Page1GB
	jc pdpte_1GB

; Create the Low Page-Directory-Pointer-Table Entries (PDPTE)
; PDPTE is stored at 0x0000000000003000, create the first entry there
; A single PDPTE can map 1GiB
; A single PDPTE is 8 bytes in length
; 4 entries are created to map the first 4GiB of RAM
	mov ecx, 4			; number of PDPE's to make.. each PDPE maps 1GiB of physical memory
	mov edi, 0x00003000		; location of low PDPE
	mov eax, 0x00010007		; Bits 0 (P), 1 (R/W), 2 (U/S), location of first low PD (4KiB aligned)
pdpte_low:
	stosq
	add rax, 0x00001000		; 4KiB later (512 records x 8 bytes)
	dec ecx
	jnz pdpte_low

; Create the Low Page-Directory Entries (PDE)
; A single PDE can map 2MiB of RAM
; A single PDE is 8 bytes in length
	mov edi, 0x00010000		; Location of first PDE
	mov eax, 0x00000087		; Bits 0 (P), 1 (R/W), 2 (U/S), and 7 (PS) set
	mov ecx, 2048			; Create 2048 2MiB page maps
pde_low:				; Create a 2MiB page
	stosq
	add rax, 0x00200000		; Increment by 2MiB
	dec ecx
	jnz pde_low
	jmp skip1GB

; Create the Low Page-Directory-Pointer Table Entries (PDPTE)
; PDPTE is stored at 0x0000000000003000, create the first entry there
; A single PDPTE can map 1GiB
; A single PDPTE is 8 bytes in length
; 512 entries are created to map the first 512GiB of RAM
pdpte_1GB:
	mov ecx, 512			; number of PDPE's to make.. each PDPE maps 1GiB of physical memory
	mov edi, 0x00003000		; location of low PDPE
	mov eax, 0x00000087		; Bits 0 (P), 1 (R/W), 2 (U/S), 7 (PS)
pdpte_low_1GB:				; Create a 1GiB page
	stosq
	add rax, 0x40000000		; Increment by 1GiB
	dec ecx
	jnz pdpte_low_1GB

skip1GB:

; Debug
	mov eax, 0x00FFFFFF
	mov ebx, 1
	call debug_block

; Load the GDT
	lgdt [GDTR64]

; Enable extended properties
;	mov eax, cr4
;	or eax, 0x0000000B0		; PGE (Bit 7), PAE (Bit 5), and PSE (Bit 4)
;	mov cr4, eax

; Point cr3 at PML4
	mov rax, 0x00002008		; Write-thru enabled (Bit 3)
	mov cr3, rax

	xor eax, eax			; aka r0
	xor ebx, ebx			; aka r3
	xor ecx, ecx			; aka r1
	xor edx, edx			; aka r2
	xor esi, esi			; aka r6
	xor edi, edi			; aka r7
	xor ebp, ebp			; aka r5
	mov esp, 0x8000			; aka r4
	xor r8, r8
	xor r9, r9
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	xor r14, r14
	xor r15, r15

	mov ax, 0x10			; TODO Is this needed?
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	; Set CS with a far return
	push SYS64_CODE_SEL
	push clearcs64
	retfq
clearcs64:

	lgdt [GDTR64]			; Reload the GDT

; Debug
	mov eax, 0x007F7F7F
	mov ebx, 2
	call debug_block

; Patch Pure64 AP code			; The AP's will be told to start execution at 0x8000
	mov edi, start			; We need to remove the BSP Jump call to get the AP's
	mov eax, 0x90909090		; to fall through to the AP Init code
	stosd
	stosd				; Write 8 bytes in total to overwrite the 'far jump' and marker

; Parse the Memory Map at 0x200000
uefi_memmap:
	xor ebx, ebx			; Running counter of 4K pages
	mov esi, 0x200000
uefi_memmap_next:
	mov rax, [rsi]
	cmp rax, 7
	je uefi_memmap_conventional
	cmp rax, 0
	je uefi_memmap_end
uefi_memmap_skip:
	add esi, 48
	jmp uefi_memmap_next
uefi_memmap_conventional:
	mov rax, [rsi + 24]
	add rbx, rax
	jmp uefi_memmap_skip
uefi_memmap_end:
	shr rbx, 8
	mov dword [p_mem_amount], ebx

; FIXME - Don't hardcode the RAM to 64MiB
	mov eax, 64
	mov dword [p_mem_amount], eax

; Create the High Page-Directory-Pointer-Table Entries (PDPTE)
; High PDPTE is stored at 0x0000000000004000, create the first entry there
; A single PDPTE can map 1GiB with 2MiB pages
; A single PDPTE is 8 bytes in length
; 1 entry is created to map the first 1GiB of physical RAM to 0xFFFF800000000000
; FIXME - Create more than just one PDPE depending on the amount of RAM in the system
	add rcx, 1			; number of PDPE's to make.. each PDPE maps 1GB of physical memory
	mov edi, 0x00004000		; location of high PDPE
	mov eax, 0x00020007		; location of first high PD. Bits (0) P, 1 (R/W), and 2 (U/S) set
create_pdpe_high:
	stosq
	add rax, 0x00001000		; 4K later (512 records x 8 bytes)
	dec ecx
	cmp ecx, 0
	jne create_pdpe_high

; Create the High Page-Directory Entries (PDE).
; A single PDE can map 2MiB of RAM
; A single PDE is 8 bytes in length
; FIXME - Map more than 64MiB depending on the amount of RAM in the system
	mov edi, 0x00020000		; Location of first PDE
	mov eax, 0x0000008F		; Bits 0 (P), 1 (R/W), 2 (U/S), 3 (PWT), and 7 (PS) set
	add rax, 0x00400000		; Start at 4MiB in
	mov ecx, 32			; Create 32 2MiB page maps
pde_high:				; Create a 2MiB page
	stosq
	add rax, 0x00200000		; Increment by 2MiB
	dec ecx
	cmp ecx, 0
	jne pde_high

; Build the IDT
	xor edi, edi 			; create the 64-bit IDT (at linear address 0x0000000000000000)

	mov rcx, 32
make_exception_gates: 			; make gates for exception handlers
	mov rax, exception_gate
	push rax			; save the exception gate to the stack for later use
	stosw				; store the low word (15:0) of the address
	mov ax, SYS64_CODE_SEL
	stosw				; store the segment selector
	mov ax, 0x8E00
	stosw				; store exception gate marker
	pop rax				; get the exception gate back
	shr rax, 16
	stosw				; store the high word (31:16) of the address
	shr rax, 16
	stosd				; store the extra high dword (63:32) of the address.
	xor rax, rax
	stosd				; reserved
	dec rcx
	jnz make_exception_gates

	mov rcx, 256-32
make_interrupt_gates: 			; make gates for the other interrupts
	mov rax, interrupt_gate
	push rax			; save the interrupt gate to the stack for later use
	stosw				; store the low word (15:0) of the address
	mov ax, SYS64_CODE_SEL
	stosw				; store the segment selector
	mov ax, 0x8F00
	stosw				; store interrupt gate marker
	pop rax				; get the interrupt gate back
	shr rax, 16
	stosw				; store the high word (31:16) of the address
	shr rax, 16
	stosd				; store the extra high dword (63:32) of the address.
	xor eax, eax
	stosd				; reserved
	dec rcx
	jnz make_interrupt_gates

	; Set up the exception gates for all of the CPU exceptions
	; The following code will be seriously busted if the exception gates are moved above 16MB
	mov word [0x00*16], exception_gate_00
	mov word [0x01*16], exception_gate_01
	mov word [0x02*16], exception_gate_02
	mov word [0x03*16], exception_gate_03
	mov word [0x04*16], exception_gate_04
	mov word [0x05*16], exception_gate_05
	mov word [0x06*16], exception_gate_06
	mov word [0x07*16], exception_gate_07
	mov word [0x08*16], exception_gate_08
	mov word [0x09*16], exception_gate_09
	mov word [0x0A*16], exception_gate_10
	mov word [0x0B*16], exception_gate_11
	mov word [0x0C*16], exception_gate_12
	mov word [0x0D*16], exception_gate_13
	mov word [0x0E*16], exception_gate_14
	mov word [0x0F*16], exception_gate_15
	mov word [0x10*16], exception_gate_16
	mov word [0x11*16], exception_gate_17
	mov word [0x12*16], exception_gate_18
	mov word [0x13*16], exception_gate_19

	mov edi, 0x21			; Set up Keyboard handler
	mov eax, keyboard
	call create_gate
	mov edi, 0x22			; Set up Cascade handler
	mov eax, cascade
	call create_gate
	mov edi, 0x28			; Set up RTC handler
	mov eax, rtc
	call create_gate

	lidt [IDTR64]			; load IDT register

; Clear memory 0xf000 - 0xf7ff for the InfoMap (2048 bytes)
	xor eax, eax
	mov ecx, 256
	mov edi, 0x0000F000
clearmapnext:
	stosq
	dec ecx
	cmp ecx, 0
	jne clearmapnext

; Read APIC Address from MSR
	mov ecx, 0x0000001B		; APIC_BASE
	rdmsr				; Returns APIC in EDX:EAX
	and eax, 0xFFFFF000		; Clear lower 12 bits
	shl rdx, 32			; Shift lower 32 bits to upper 32 bits
	add rax, rdx
	mov [p_LocalAPICAddress], rax

; Check for x2APIC support
	mov eax, 1
	cpuid				; x2APIC is supported if bit 21 is set
	shr ecx, 21
	and cl, 1
	mov byte [p_x2APIC], cl

; Debug
	mov eax, 0x00FFFFFF
	mov ebx, 3
	call debug_block

	call init_acpi			; Find and process the ACPI tables

; Debug
	mov eax, 0x007F7F7F
	mov ebx, 4
	call debug_block

	call init_cpu			; Configure the BSP CPU

; Debug
	mov eax, 0x00FFFFFF
	mov ebx, 5
	call debug_block

	call init_pic			; Configure the PIC(s), also activate interrupts

; Debug
	mov eax, 0x007F7F7F
	mov ebx, 6
	call debug_block

	call init_smp			; Init of SMP

; Debug
	mov eax, 0x00FFFFFF
	mov ebx, 7
	call debug_block

; Reset the stack to the proper location (was set to 0x8000 previously)
	mov rsi, [p_LocalAPICAddress]	; We would call p_smp_get_id here but the stack is not ...
	add rsi, 0x20			; ... yet defined. It is safer to find the value directly.
	lodsd				; Load a 32-bit value. We only want the high 8 bits
	shr rax, 24			; Shift to the right and AL now holds the CPU's APIC ID
	shl rax, 10			; shift left 10 bits for a 1024byte stack
	add rax, 0x0000000000050400	; stacks decrement when you "push", start at 1024 bytes in
	mov rsp, rax			; Pure64 leaves 0x50000-0x9FFFF free so we use that

; Build the InfoMap
	xor edi, edi
	mov di, 0x5000
	mov rax, [p_ACPITableAddress]
	stosq
	mov eax, [p_BSP]
	stosd

	mov di, 0x5010
	mov ax, [p_cpu_speed]
	stosw
	mov ax, [p_cpu_activated]
	stosw
	mov ax, [p_cpu_detected]
	stosw

	mov di, 0x5020
	mov ax, [p_mem_amount]
	stosd

	mov di, 0x5030
	mov al, [p_IOAPICCount]
	stosb
	mov al, [p_IOAPICIntSourceC]
	stosb

	mov di, 0x5040
	mov rax, [p_HPETAddress]
	stosq

	mov di, 0x5060
	mov rax, [p_LocalAPICAddress]
	stosq

	; Copy the data we received from UEFI/BIOS
	mov di, 0x5080
	mov rax, [0x00005F00]		; Base address of video memory 
	stosq
	mov eax, [0x00005F00 + 0x10]	; X and Y resolution (16-bits each)
	stosd
	mov al, 32			; Color depth
	stosb

	mov di, 0x5090
	mov ax, [p_PCIECount]
	stosw

; Move the trailing binary to its final location
	mov esi, 0x8000+PURE64SIZE	; Memory offset to end of pure64.sys
	mov edi, 0x100000		; Destination address at the 1MiB mark
	mov ecx, ((32768 - PURE64SIZE) / 8)
	rep movsq			; Copy 8 bytes at a time

; Output message via serial port
	mov rsi, message_ok		; Location of message
	call debug_msg

; Debug
	mov eax, 0x0000FF00
	mov ebx, 99
	call debug_block

; Clear all registers (skip the stack pointer)
	xor eax, eax			; These 32-bit calls also clear the upper bits of the 64-bit registers
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	xor esi, esi
	xor edi, edi
	xor ebp, ebp
	xor r8, r8
	xor r9, r9
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	xor r14, r14
	xor r15, r15
	jmp 0x00100000			; Done with Pure64, jump to the kernel


%include "init/acpi.asm"
%include "init/cpu.asm"
%include "init/pic.asm"
%include "init/smp.asm"
%include "interrupt.asm"
%include "sysvar.asm"


; -----------------------------------------------------------------------------
; debug_block - Create a block of colour on the screen
; IN:	EAX = Colour
;	EBX = Index #
debug_block:
	push rax
	push rbx
	push rcx
	push rdx
	push rdi

	mov rdi, [0x00005F00]		; Frame buffer base
	xor edx, edx
	mov edx, [0x00005F00 + 0x14]	; PixelsPerScanLine
	shl ebx, 5
	add rdi, rbx
	shl edx, 2			; Quick multiply by 4 for line offset

	mov ebx, 8			; 8 pixels tall
nextline:
	mov ecx, 8			; 8 pixels wide
	rep stosd
	add rdi, rdx			; Add line offset
	sub rdi, 8*4
	dec ebx
	jnz nextline

	pop rdi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; debug_msg_char - Send a single char via the serial port
; IN: AL = Byte to send
debug_msg_char:
	pushf
	push rdx
	push rax			; Save the byte
	mov dx, 0x03F8			; Address of first serial port
debug_msg_char_wait:
	add dx, 5			; Offset to Line Status Register
	in al, dx
	sub dx, 5			; Back to to base
	and al, 0x20
	cmp al, 0
	je debug_msg_char_wait
	pop rax				; Restore the byte
	out dx, al			; Send the char to the serial port
debug_msg_char_done:
	pop rdx
	popf
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; debug_msg_char - Send a message via the serial port
; IN: RSI = Location of message
debug_msg:
	pushf
	push rdx
	push rax
	cld				; Clear the direction flag.. we want to increment through the string
	mov dx, 0x03F8			; Address of first serial port
debug_msg_next:
	add dx, 5			; Offset to Line Status Register
	in al, dx
	sub dx, 5			; Back to to base
	and al, 0x20
	cmp al, 0
	je debug_msg_next
	lodsb				; Get char from string and store in AL
	cmp al, 0
	je debug_msg_done
	out dx, al			; Send the char to the serial port
	jmp debug_msg_next
debug_msg_done:
	pop rax
	pop rdx
	popf
	ret
; -----------------------------------------------------------------------------


EOF:
	db 0xDE, 0xAD, 0xC0, 0xDE

; Pad to an even KB file
times PURE64SIZE-($-$$) db 0x90


; =============================================================================
; EOF
