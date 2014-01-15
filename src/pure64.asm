; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2013 Return Infinity -- see LICENSE.TXT
;
; Loaded from the first stage. Gather information about the system while
; in 16-bit mode (BIOS is still accessible), setup a minimal 64-bit
; environment, copy the 64-bit kernel from the end of the Pure64 binary to 
; the 1MiB memory mark and jump to it!
;
; Pure64 requires a payload for execution! The stand-alone pure64.sys file
; is not sufficient. You must append your kernel or software to the end of
; the Pure64 binary. The maximum size of the kernel of software is 26KiB.
;
; Windows - copy /b pure64.sys + kernel64.sys
; Unix - cat pure64.sys kernel64.sys > pure64.sys
; Max size of the resulting pure64.sys is 32768 bytes (32KiB)
; =============================================================================


USE16
ORG 0x00008000
start:
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

ap_modify:
	jmp start16			; This command will be overwritten with 'NOP's before the AP's are started
	nop				; The 'jmp' is only 3 bytes

%include "init/smp_ap.asm"		; AP's will start execution at 0x8000 and fall through to this code


;db '_16_'				; Debug
align 16

USE16
start16:
	jmp 0x0000:clearcs

clearcs:

; Configure serial port
	xor dx, dx			; First serial port
	mov ax, 0000000011100011b	; 9600 baud, no parity, 1 stop bit, 8 data bits
	int 0x14

; Make sure the screen is set to 80x25 color text mode
	mov ax, 0x0003			; Set to normal (80x25 text) video mode
	int 0x10

; Disable blinking
	mov ax, 0x1003
	mov bx, 0x0000
	int 0x10

; Print message
	mov si, msg_initializing
	call print_string_16

; Check to make sure the CPU supports 64-bit mode... If not then bail out
	mov eax, 0x80000000		; Extended-function 8000000h.
	cpuid				; Is largest extended function
	cmp eax, 0x80000000		; any function > 80000000h?
	jbe no_long_mode		; If not, no long mode.
	mov eax, 0x80000001		; Extended-function 8000001h.
	cpuid				; Now EDX = extended-features flags.
	bt edx, 29			; Test if long mode is supported.
	jnc no_long_mode		; Exit if not supported.

	call init_isa			; Setup legacy hardware

; Hide the hardware cursor (interferes with print_string_16 if called earlier)
	mov ax, 0x0200			; VIDEO - SET CURSOR POSITION
	mov bx, 0x0000			; Page number
	mov dx, 0x2000			; Row / Column
	int 0x10

; At this point we are done with real mode and BIOS interrupts. Jump to 32-bit mode.
	lgdt [cs:GDTR32]		; Load GDT register

	mov eax, cr0
	or al, 0x01			; Set protected mode bit
	mov cr0, eax

	jmp 8:start32			; Jump to 32-bit protected mode

; 16-bit function to print a sting to the screen
print_string_16:			; Output string in SI to screen
	pusha
	mov ah, 0x0E			; http://www.ctyme.com/intr/rb-0106.htm
print_string_16_repeat:
	lodsb				; Get char from string
	cmp al, 0
	je print_string_16_done		; If char is zero, end of string
	int 0x10			; Otherwise, print it
	jmp print_string_16_repeat
print_string_16_done:
	popa
	ret

; Display an error message that the CPU does not support 64-bit mode
no_long_mode:
	mov si, msg_no64
	call print_string_16
	jmp $

%include "init/isa.asm"

align 16
GDTR32:					; Global Descriptors Table Register
dw gdt32_end - gdt32 - 1		; limit of GDT (size minus one)
dq gdt32				; linear address of GDT

align 16
gdt32:
dw 0x0000, 0x0000, 0x0000, 0x0000	; Null desciptor
dw 0xFFFF, 0x0000, 0x9A00, 0x00CF	; 32-bit code descriptor
dw 0xFFFF, 0x0000, 0x9200, 0x00CF	; 32-bit data descriptor
gdt32_end:

;db '_32_'				; Debug
align 16


; =============================================================================
; 32-bit mode
USE32

start32:
	xor eax, eax
	mov al, 16			; load 4 GB data descriptor
	mov ds, ax			; to all data segment registers
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	xor esi, esi
	xor edi, edi
	xor ebp, ebp
	mov esp, 0x8000			; Set a known free location for the stack

	mov al, '2'			; Now in 32-bit protected mode (0x20 = 32)
	mov [0x000B809C], al
	mov al, '0'
	mov [0x000B809E], al

; Clear out the first 4096 bytes of memory. This will store the 64-bit IDT, GDT, PML4, and PDP
	mov ecx, 1024
	xor eax, eax
	xor edi, edi
	rep stosd

; Clear memory for the Page Descriptor Entries (0x10000 - 0x4FFFF)
	mov edi, 0x00010000
	mov ecx, 65536
	rep stosd

; Copy the GDT to its final location in memory
	mov esi, gdt64
	mov edi, 0x00001000		; GDT address
	mov ecx, (gdt64_end - gdt64)
	rep movsb			; Move it to final pos.

; Create the Level 4 Page Map. (Maps 4GBs of 2MB pages)
; First create a PML4 entry.
; PML4 is stored at 0x0000000000002000, create the first entry there
; A single PML4 entry can map 512GB with 2MB pages.
	cld
	mov edi, 0x00002000		; Create a PML4 entry for the first 4GB of RAM
	mov eax, 0x00003007
	mov [edi], eax
	xor ecx, ecx
	mov [edi+4], ecx

	mov edi, 0x00002800		; Create a PML4 entry for higher half (starting at 0xFFFF800000000000)
	mov [edi], eax
	mov [edi+4], ecx

; Create the PDP entries.
; The first PDP is stored at 0x0000000000003000, create the first entries there
; A single PDP entry can map 1GB with 2MB pages
	mov cl, 64			; number of PDPE's to make.. each PDPE maps 1GB of physical memory
	mov edi, 0x00003000
	xor ebx, ebx
	mov eax, 0x00010007		; location of first PD
create_pdpe:
	mov [edi], eax
	mov [edi+4], ebx	
	add eax, 0x00001000		; 4K later (512 records x 8 bytes)
	lea edi, [edi+8]
	dec ecx
	jnz create_pdpe

; Create the PD entries.
; PD entries are stored starting at 0x0000000000010000 and ending at 0x000000000004FFFF (256 KiB)
; This gives us room to map 64 GiB with 2 MiB pages
	mov edi, 0x00010000
	mov eax, 0x0000008F		; Bit 7 must be set to 1 as we have 2 MiB pages
	mov ecx, 2048	
pd_again:				; Create a 2 MiB page
	mov [edi], eax
	mov [edi+4], ebx
	add edi, 8
	add eax, 0x00200000
	dec ecx, 
	jnz pd_again			; Create 2048 2 MiB page maps.

; Load the GDT
	lgdt [GDTR64]

; Enable extended properties
	mov eax, cr4
	or eax, 0x0000000B0		; PGE (Bit 7), PAE (Bit 5), and PSE (Bit 4)
	mov cr4, eax

; Point cr3 at PML4
	mov eax, 0x00002008		; Write-thru (Bit 3)
	mov cr3, eax

; Enable long mode and SYSCALL/SYSRET
	mov ecx, 0xC0000080		; EFER MSR number
	rdmsr				; Read EFER
	or eax, 0x00000101 		; LME (Bit 8)
	wrmsr				; Write EFER

; Debug
	xor eax, eax
	mov al, '1'			; About to make the jump into 64-bit mode
	mov [0x000B809C], al
	mov al, 'E'
	mov [0x000B809E], al

; Enable paging to activate long mode
	mov eax, cr0
	bts eax, 31		; PG (Bit 31)
	mov cr0, eax

	jmp SYS64_CODE_SEL:start64	; Jump to 64-bit mode

;db '_64_'				; Debug
align 16


; =============================================================================
; 64-bit mode
USE64

start64:
; Debug
	xor eax, eax
	mov al, '4'			; Now in 64-bit mode (0x40 = 64)
	mov [0x000B809C], al
	mov al, '0'
	mov [0x000B809E], al
	
	mov eax, 0x1602
	call os_move_cursor

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

	mov ds, ax			; Clear the legacy segment registers
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	mov rax, clearcs64		; Do a proper 64-bit jump. Should not be needed as the ...
	jmp rax				; jmp SYS64_CODE_SEL:start64 would have sent us ...
	nop				; out of compatibility mode and into 64-bit mode
clearcs64:
	xor eax, eax

	lgdt [GDTR64]			; Reload the GDT

; Debug
	mov al, '2'
	mov [0x000B809E], al

; Patch Pure64 AP code			; The AP's will be told to start execution at 0x8000
	mov edi, ap_modify		; We need to remove the BSP Jump call to get the AP's
	mov eax, 0x90909090		; to fall through to the AP Init code
	mov [rdi], eax

; Build the rest of the page tables (4GiB+)
	mov ecx, 30720
	mov rax, 0x10000008F
	mov edi, 0x14000
buildem:
	mov [rdi], rax
	add rdi, 8
	add rax, 0x200000
	dec rcx
	jnz buildem
	; We have 64 GiB mapped now

; Build a temporary IDT
	xor edi, edi 			; create the 64-bit IDT (at linear address 0x0000000000000000)

	xor ecx, ecx
	mov cl, 32
make_exception_gates: 			; make gates for exception handlers
	mov rax, exception_gate
	mov rbx, rax			; save the exception gate to the stack for later use
	movzx edx, ax
	mov [rdi], word ax				; store the low word (15..0) of the address
	mov ax, SYS64_CODE_SEL
	movzx eax, ax
	shl eax, 16
	or  edx, eax
	mov [rdi+2], word ax				; store the segment selector
	mov ax, 0x8E00
	movzx eax, ax
	shl rax, 32
	or rdx, rax
	mov [rdi], rdx				; store exception gate marker
	mov rax, rbx			; get the exception gate back
	shr rax, 16
	mov [rdi+6], word ax			; store the high word (31..16) of the address
	shr rax, 16
	mov ebx, eax
	mov [rdi+8], rbx			; store the extra high dword (63..32) of the address.
	add edi, 16
	dec ecx
	jnz make_exception_gates

	xor ecx, ecx
	mov cl, 256-32
make_interrupt_gates: 			; make gates for the other interrupts
	xor eax, eax
	mov rbx, interrupt_gate
					; save the interrupt gate to the stack for later use
	mov [rdi], word bx		; store the low word (15..0) of the address
	mov ax, SYS64_CODE_SEL
	mov [rdi+2], word ax		; store the segment selector
	mov ax, 0x8F00
	mov [rdi+4], word ax		; store interrupt gate marker
					; get the interrupt gate back
	shr rbx, 16
	mov [rdi+6], word bx		; store the high word (31..16) of the address
	shr rbx, 16
	mov eax, ebx
	mov [rdi+8], rax		; store the extra high dword (63..32) of the address.
	xor rax, rax
	add edi, 16
	dec ecx
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

	xor edi, edi
	mov edi, 0x21			; Set up Keyboard IRQ handler
	mov eax, keyboard
	call create_gate
	mov edi, 0x28			; Set up RTC IRQ handler
	mov eax, rtc
	call create_gate
	mov edi, 0xF8			; Set up Spurious handler
	mov eax, spurious
	call create_gate

	lidt [IDTR64]			; load IDT register

; Debug
	mov al, '4'
	mov [0x000B809E], al

; Clear memory 0xf000 - 0xf7ff for the infomap (2048 bytes)
	bts ecx, 8			; ecx=256
	xor eax, eax
	mov edi, 0xF000
clearmapnext:
	rep stosq

	call init_acpi			; Find and process the ACPI tables

	call init_cpu			; Configure the BSP CPU

	call init_ioapic		; Configure the IO-APIC(s), also activate interrupts

; Debug
	mov al, '6'			; CPU Init complete
	mov [0x000B809E], al

; Make sure exceptions are working.
;	xor rax, rax
;	xor rbx, rbx
;	xor rcx, rcx
;	xor rdx, rdx
;	div rax

; Init of SMP
	call init_smp

; Reset the stack to the proper location (was set to 0x8000 previously)
	mov esi, [os_LocalAPICAddress]	; We would call os_smp_get_id here but the stack is not ...
	mov eax, [rsi+0x20]
	shr eax, 24			; Shift to the right and AL now holds the CPU's APIC ID
	shl eax, 10			; shift left 10 bits for a 1024byte stack
	add eax, 0x50400		; stacks decrement when you "push", start at 1024 bytes in
	mov esp, eax			; Pure64 leaves 0x50000-0x9FFFF free so we use that

; Debug
	xor eax, eax
	mov al, '6'			; SMP Init complete
	mov [0x000B809C], al
	mov al, '0'
	mov [0x000B809E], al

; Calculate amount of usable RAM from Memory Map
	xor ecx, ecx
	xor ebx, ebx
	xor edx, edx
	mov esi, 0x4000	; E820 Map location
readnextrecord:
	mov eax, [rsi+20]
	cmp eax, 1			; Useable RAM
	sete bl
	cmp eax, 3			; ACPI Reclaimable
	sete dl
	or ebx, edx
	cmp eax, 6			; BIOS Reclaimable
	sete dl
	add ebx, edx
	jz badmem
	add rcx, [rsi+8]
	add esi, 16
	test eax, eax			; Are we at the end?
	jnz readnextrecord
badmem:
	add rcx, [rsi+8]
	add esi, 32
	test eax, eax
	jnz readnextrecord

endmemcalc:
	shr rcx, 20			; Value is in bytes so do a quick divide by 1048576 to get MiB's
	add ecx, 1			; The BIOS will usually report actual memory minus 1
	and ecx, 0xFE			; Make sure it is an even number (in case we added 1 to an even number)
	mov dword [mem_amount], ecx

; Debug
	mov al, '2'
	mov [0x000B809E], al

; Convert CPU speed value to string
	movzx eax, word [cpu_speed]
	mov rdi, speedtempstring
	call os_int_to_string

; Convert CPU amount value to string
	movzx eax, word [cpu_activated]
	mov rdi, cpu_amount_string
	call os_int_to_string

; Convert RAM amount value to string
	mov eax, [mem_amount]
	mov rdi, memtempstring
	call os_int_to_string

; Build the infomap
	xor edi, edi
	mov di, 0x5000
	mov rax, [os_ACPITableAddress]
	mov [rdi], rax
	mov eax, [os_BSP]
	mov [rdi+8], eax

	add edi, 0x10
	mov rax, [cpu_speed]
	mov [rdi], rax

	movzx eax, word [mem_amount]
	mov [rdi+0x20], eax

	movzx eax, byte [os_IOAPICCount]
	mov [rdi+0x30], al

	mov rax, [os_HPETAddress]
	mov [rdi+0x40], rax

	mov rax, [os_LocalAPICAddress]
	mov [rdi+0x60], rax
	add edi, 0x68
	xor ecx, ecx
	mov cl, [os_IOAPICCount]
	mov esi, os_IOAPICAddress
nextIOAPIC:
	rep movsq
	xor edi, edi
	mov di, 0x5080
	mov eax, [VBEModeInfoBlock.PhysBasePtr]		; Base address of video memory (if graphics mode is set)
	mov ebx, [VBEModeInfoBlock.XResolution]		; X and Y resolution (16-bits each)
	shl rbx, 32
	or rax, rbx
	mov [rdi], rax
	movzx eax, byte [VBEModeInfoBlock.BitsPerPixel]		; Color depth
	mov [rdi+8], al

; Initialization is now complete... write a message to the screen
	mov rsi, msg_done
	call os_print_string

; Debug
	mov al, '4'
	mov [0x000B809E], al

; Print info on CPU and MEM
	mov ax, 0x0004
	call os_move_cursor
	mov rsi, msg_CPU
	call os_print_string
	mov rsi, speedtempstring
	call os_print_string
	mov rsi, msg_mhz
	call os_print_string
	mov rsi, cpu_amount_string
	call os_print_string
	mov rsi, msg_MEM
	call os_print_string
	mov rsi, memtempstring
	call os_print_string
	mov rsi, msg_mb
	call os_print_string

; Move the trailing binary to its final location
	mov rsi, 0x8000+6144		; Memory offset to end of pure64.sys
	mov rdi, 0x100000		; Destination address at the 1MiB mark
	mov rcx, 0x0D00			; For up to 26KiB kernel (26624 / 8)
	rep movsq			; Copy 8 bytes at a time

; Print a message that the kernel is being started
	mov ax, 0x0006
	call os_move_cursor
	mov rsi, msg_startingkernel
	call os_print_string

; Debug
	mov rdi, 0x000B8092		; Clear the debug messages
	mov ax, 0x0720
	mov cx, 7
	movzx  ecx, cx
clearnext:
	mov [rdi], word ax
	add rdi, 2
	dec ecx
	jnz clearnext

; Clear all registers (skip the stack pointer)
	xor eax, eax
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

	jmp 0x0000000000100000		; Jump to the kernel


%include "init/acpi.asm"
%include "init/cpu.asm"
%include "init/ioapic.asm"
%include "init/smp.asm"
%include "syscalls.asm"
%include "interrupt.asm"
%include "sysvar.asm"

; Pad to an even KB file (6 KiB)
times 6144-($-$$) db 0x90


; =============================================================================
; EOF
