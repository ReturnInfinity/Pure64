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
	jmp start64			; This command will be overwritten with 'NOP's before the AP's are started
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
; 64-bit mode
BITS 64
start64:
	mov esp, 0x8000			; Set a known free location for the stack

	mov edi, 0x5000			; Clear the info map and system variable memory
	xor eax, eax
	mov ecx, 960			; 3840 bytes (Range is 0x5000 - 0x5EFF)
	rep stosd			; Don't overwrite the VBE data at 0x5F00

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

; Configure serial port @ 0x03F8
	mov dx, 0x03F8 + 1		; Interrupt Enable
	mov al, 0x00			; Disable all interrupts
	out dx, al
	mov dx, 0x03F8 + 3		; Line Control
	mov al, 80
	out dx, al
	mov dx, 0x03F8 + 0		; Divisor Latch
	mov ax, 1			; 1 = 115200 baud
	out dx, ax
	mov dx, 0x03F8 + 3		; Line Control
	mov al, 3			; 8 bits, no parity, one stop bit
	out dx, al
	mov dx, 0x03F8 + 4		; Modem Control
	mov al, 3
	out dx, al
	mov al, 0xC7			; Enable FIFO, clear them, with 14-byte threshold
	mov dx, 0x03F8 + 2
	out dx, al

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
; A single PML4 entry can map 512GiB with 2MiB pages
; A single PML4 entry is 8 bytes in length
	cld
	mov edi, 0x00002000		; Create a PML4 entry for the first 4GiB of RAM
	mov eax, 0x00003007		; Bits 0 (P), 1 (R/W), 2 (U/S), location of low PDP (4KiB aligned)
	stosd
	xor eax, eax
	stosd

	mov edi, 0x00002800		; Create a PML4 entry for higher half (starting at 0xFFFF800000000000)
	mov eax, 0x00004007		; Bits 0 (P), 1 (R/W), 2 (U/S), location of high PDP (4KiB aligned)
	stosd
	xor eax, eax
	stosd

; Create the Page-Directory-Pointer-Table Entries (PDPTE)
; PDPTE is stored at 0x0000000000003000, create the first entry there
; A single PDPTE can map 1GiB with 2MiB pages
; A single PDPTE is 8 bytes in length
; 4 entries are created to map the first 4GiB of RAM
	mov ecx, 4			; number of PDPE's to make.. each PDPE maps 1GiB of physical memory
	mov edi, 0x00003000		; location of low PDPE
	mov eax, 0x00010007		; Bits 0 (P), 1 (R/W), 2 (U/S), location of first low PD (4KiB aligned)
create_pdpte_low:
	stosd
	push rax
	xor eax, eax
	stosd
	pop rax
	add eax, 0x00001000		; 4KiB later (512 records x 8 bytes)
	dec ecx
	cmp ecx, 0
	jne create_pdpte_low

; Create the low Page-Directory Entries (PDE).
; A single PDE can map 2MiB of RAM
; A single PDE is 8 bytes in length
	mov edi, 0x00010000		; Location of first PDE
	mov eax, 0x0000008F		; Bits 0 (P), 1 (R/W), 2 (U/S), 3 (PWT), and 7 (PS) set
	mov ecx, 2048			; Create 2048 2 MiB page maps.
pde_low:				; Create a 2 MiB page
	stosd
	push rax
	xor eax, eax
	stosd
	pop rax
	add eax, 0x00200000		; Increment by 2MiB
	dec ecx
	cmp ecx, 0
	jne pde_low

; Load the GDT
	lgdt [GDTR64]

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

; Patch Pure64 AP code			; The AP's will be told to start execution at 0x8000
	mov edi, start			; We need to remove the BSP Jump call to get the AP's
	mov eax, 0x90909090		; to fall through to the AP Init code
	stosd
	stosd				; Write 8 bytes in total to overwrite the 'far jump' and marker

uefi_memmap:				; TODO fix this as it is a terrible hack
	mov rdi, 0x400000
	mov al, 1
	mov rcx, 32
	rep stosb
	mov ebx, 64
	mov dword [p_mem_amount], ebx
memmap_end:

; Create the high memory map
	mov rcx, rbx
	shr rcx, 9			; TODO - This isn't the exact math but good enough
	add rcx, 1			; number of PDPE's to make.. each PDPE maps 1GB of physical memory
	mov edi, 0x00004000		; location of high PDPE
	mov eax, 0x00020007		; location of first high PD. Bits (0) P, 1 (R/W), and 2 (U/S) set
create_pdpe_high:
	stosq
	add rax, 0x00001000		; 4K later (512 records x 8 bytes)
	dec ecx
	cmp ecx, 0
	jne create_pdpe_high

; Create the high PD entries
; EBX contains the number of pages that should exist in the map, once they are all found bail out
	xor ecx, ecx
	xor eax, eax
	xor edx, edx
	mov edi, 0x00020000		; Location of high PD entries
	mov esi, 0x00400000		; Location of free pages map

pd_high:
	cmp rdx, rbx			; Compare mapped pages to max pages
	je pd_high_done
	lodsb
	cmp al, 1
	je pd_high_entry
	add rcx, 1
	jmp pd_high

pd_high_entry:
	mov eax, 0x0000008F		; Bits 0 (P), 1 (R/W), 2 (U/S), 3 (PWT), and 7 (PS) set
	shl rcx, 21
	add rax, rcx
	shr rcx, 21
	stosq
	add rcx, 1
	add rdx, 1			; We have mapped a valid page
	jmp pd_high

pd_high_done:

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

	call init_acpi			; Find and process the ACPI tables

	call init_cpu			; Configure the BSP CPU

	call init_pic			; Configure the PIC(s), also activate interrupts

	call init_smp			; Init of SMP

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

	mov di, 0x5080
	mov eax, [VBEModeInfoBlock.PhysBasePtr]		; Base address of video memory (if graphics mode is set)
	stosd
	mov eax, [VBEModeInfoBlock.XResolution]		; X and Y resolution (16-bits each)
	stosd
	mov al, [VBEModeInfoBlock.BitsPerPixel]		; Color depth
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
	cld				; Clear the direction flag.. we want to increment through the string
	mov dx, 0x03F8			; Address of first serial port
	mov rsi, message		; Location of message
	mov cx, 11			; Length of message
serial_nextchar:
	jrcxz serial_done		; If RCX is 0 then the function is complete
	add dx, 5			; Offset to Line Status Register
	in al, dx
	sub dx, 5			; Back to to base
	and al, 0x20
	cmp al, 0
	je serial_nextchar
	dec cx
	lodsb				; Get char from string and store in AL
	out dx, al			; Send the char to the serial port
	jmp serial_nextchar
serial_done:

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
	jmp 0x00100000


%include "init/acpi.asm"
%include "init/cpu.asm"
%include "init/pic.asm"
%include "init/smp.asm"
%include "interrupt.asm"
%include "sysvar.asm"

EOF:
	db 0xDE, 0xAD, 0xC0, 0xDE

; Pad to an even KB file
times PURE64SIZE-($-$$) db 0x90


; =============================================================================
; EOF
