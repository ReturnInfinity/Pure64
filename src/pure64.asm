; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; The first stage loader is required to gather information about the system
; while the BIOS or UEFI is still available and load the Pure64 binary to
; 0x00008000. Setup a minimal 64-bit environment, copy the 64-bit kernel from
; the end of the Pure64 binary to the 1MiB memory mark and jump to it.
;
; Pure64 requires a payload for execution! The stand-alone pure64.sys file
; is not sufficient. You must append your kernel or software to the end of
; the Pure64 binary. The maximum size of the kernel or software is 26KiB.
;
; Windows - copy /b pure64.sys + kernel64.sys
; Unix - cat pure64.sys kernel64.sys > pure64.sys
; Max size of the resulting pure64.sys is 32768 bytes (32KiB)
; =============================================================================


BITS 64
ORG 0x00008000
PURE64SIZE equ 6144			; Pad Pure64 to this length

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
	mov esp, 0x7000			; Set a known free location for the stack
	jmp 0x0000:init_smp_ap

%include "init/smp_ap.asm"		; AP's will start execution at 0x8000 and fall through to this code

; =============================================================================
; This is 32-bit code so it's important that the encoding of the first few instructions also
; work in 64-bit mode. If a 'U' is stored at 0x5FFF then we know it was a UEFI boot and can
; immediately proceed to start64. Otherwise we need to set up a minimal 64-bit environment.
BITS 32
bootmode:
	mov [p_BootDisk], bh		; Save from where system is booted
	cmp bl, 'U'			; If it is 'U' then we booted via UEFI and are already in 64-bit mode for the BSP
	je start64			; Jump to the 64-bit code, otherwise fall through to 32-bit init

%ifdef BIOS
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
	xor eax, eax
	xor ebx, ebx
	mov ax, [0x5F00 + 16]		; BytesPerScanLine
	push eax
	mov bx, [0x5F00 + 20]		; YResolution
	push ebx
	mov ax, [0x5F00 + 18]		; XResolution
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
	stosw				; 16-bit Screen X
	pop eax
	stosw				; 16-bit Screen Y
	pop eax
	shr eax, 2			; Quick divide by 4
	stosw				; PixelsPerScanLine
	mov eax, 32
	stosw				; BitsPerPixel

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
	mov bh, byte [p_BootDisk]

; Enable paging to activate long mode
	mov eax, cr0
	or eax, 0x80000000		; PG (Bit 31)
	mov cr0, eax

	jmp SYS64_CODE_SEL:start64	; Jump to 64-bit mode
%endif

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
	mov [p_BootDisk], bh

	; Mask all PIC interrupts
	mov al, 0xFF
	out 0x21, al
	out 0xA1, al

	; Disable NMIs
	in al, 0x70
	or al, 0x80
	out 0x70, al
	in al, 0x71

	; Disable PIT
	mov al, 0x30			; Channel 0 (7:6), Access Mode lo/hi (5:4), Mode 0 (3:1), Binary (0)
	out 0x43, al
	mov al, 0x00
	out 0x40, al

	; Clear screen
	xor eax, eax
	xor ecx, ecx
	xor edx, edx
	mov ax, [0x00005F10]
	mov cx, [0x00005F12]
	mul ecx
	mov ecx, eax
	mov rdi, [0x00005F00]
	mov eax, 0x00404040
	rep stosd

; Visual Debug (1/4)
	mov ebx, 0
	call debug_block

	; Configure serial port @ 0x03F8 as 115200 8N1
	call init_serial

	mov rsi, msg_pure64		; Output "[ Pure64 ]"
	call debug_msg

	; Output boot method
	mov rsi, msg_boot
	call debug_msg
	cmp byte [p_BootMode], 'U'
	je boot_uefi
	mov rsi, msg_bios
	call debug_msg
	jmp msg_boot_done
boot_uefi:
	mov rsi, msg_uefi
	call debug_msg
msg_boot_done:

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
	mov eax, 0x00003003		; Bits 0 (P), 1 (R/W), location of low PDP (4KiB aligned)
	stosq
	mov edi, 0x00002800		; Create a PML4 entry for higher half (starting at 0xFFFF800000000000)
	mov eax, 0x00004003		; Bits 0 (P), 1 (R/W), location of high PDP (4KiB aligned)
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
; FIXME - This will completely fill the 64K set for the low PDE (only 16GiB identity mapped)
	mov ecx, 16			; number of PDPE's to make.. each PDPE maps 1GiB of physical memory
	mov edi, 0x00003000		; location of low PDPE
	mov eax, 0x00010003		; Bits 0 (P), 1 (R/W), location of first low PD (4KiB aligned)
pdpte_low:
	stosq
	add rax, 0x00001000		; 4KiB later (512 records x 8 bytes)
	dec ecx
	jnz pdpte_low

; Create the Low Page-Directory Entries (PDE)
; A single PDE can map 2MiB of RAM
; A single PDE is 8 bytes in length
	mov edi, 0x00010000		; Location of first PDE
	mov eax, 0x00000083		; Bits 0 (P), 1 (R/W), and 7 (PS) set
	mov ecx, 8192			; Create 8192 2MiB page maps
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
; 1024 entries are created to map the first 1024GiB of RAM
pdpte_1GB:
	; Overwrite the original PML4 entry for physical memory
	mov edi, 0x00002000		; Create a PML4 entry for physical memory
	mov eax, 0x00010003		; Bits 0 (P), 1 (R/W), location of low PDP (4KiB aligned)
	stosq
	add eax, 0x1000
	stosq
	mov ecx, 1024			; number of PDPE's to make.. each PDPE maps 1GiB of physical memory
	mov edi, 0x00010000		; location of low PDPE
	mov eax, 0x00000083		; Bits 0 (P), 1 (R/W), 7 (PS)
pdpte_low_1GB:				; Create a 1GiB page
	stosq
	add rax, 0x40000000		; Increment by 1GiB
	dec ecx
	jnz pdpte_low_1GB

skip1GB:

	mov rsi, msg_pml4
	call debug_msg

; Load the GDT
	lgdt [GDTR64]

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

	mov rsi, msg_ok
	call debug_msg

; Visual Debug (2/4)
	mov ebx, 2
	call debug_block

; Patch Pure64 AP code			; The AP's will be told to start execution at 0x8000
	mov edi, start			; We need to remove the BSP Jump call to get the AP's
	mov eax, 0x90909090		; to fall through to the AP Init code
	stosd
	stosd				; Write 8 bytes in total to overwrite the 'far jump' and marker

; Parse the Memory Map from BIOS/UEFI
	cmp byte [p_BootMode], 'U'
	jne bios_memmap

%ifdef UEFI
; Parse the memory map provided by UEFI
uefi_memmap:
; Stage 1 - Process the UEFI memory map to find all usable memory
; Types 1-7 are ok to use once Boot Services were exited. Anything else should be considered unusable.
; Build an usable memory map at 0x200000
	xor r9, r9
	xor ebx, ebx
	mov esi, 0x00220000 - 48	; The start of the UEFI map minus 48 bytes for the loop start
	mov edi, 0x00200000		; Where to build the clean map
uefi_memmap_next:
	add esi, 48			; Skip to start of next record
	mov rax, [rsi+24]		; Check for end of records
	cmp rax, 0			; If there are 0 pages we are at the end of the list
	je uefi_memmap_end
	mov rax, [rsi+8]
	cmp rax, 0x100000		; Test if the Physical Address less than 0x100000
	jl uefi_memmap_next		; If so, skip it
	mov rax, [rsi]
	cmp rax, 0			; EfiReservedMemoryType (Not usable)
	je uefi_memmap_next
	cmp rax, 7			; EfiConventionalMemory (Free)
	jle uefi_memmap_usable
	mov bl, 0
	jmp uefi_memmap_next
uefi_memmap_usable:
	cmp bl, 1
	je uefi_memmap_usable_contiguous
	mov rax, [rsi+8]
	stosq				; Store Physical Start
	mov rax, [rsi+24]
	stosq				; Store NumberOfPages
uefi_memmap_usable_contiguous_next:
	mov r9, rax
	shl r9, 12			; Quick multiply by 4096
	add r9, [rsi+8]			; R9 should match the physical address of the next record if they are contiguous
	mov bl, 0			; Non-contiguous
	cmp r9, [rsi+56]		; Check R9 against the next Physical Start
	jne uefi_memmap_next
	mov bl, 1			; Contiguous
	jmp uefi_memmap_next
uefi_memmap_usable_contiguous:
	sub rdi, 8
	mov rax, [rsi+24]
	add rax, [rdi]
	stosq
	mov rax, [rsi+24]
	jmp uefi_memmap_usable_contiguous_next
uefi_memmap_end:
	xor eax, eax			; Store a blank record
	stosq
	stosq

; Stage 2 - Clear entries less than 3MiB in length
	mov esi, 0x00200000		; Start at the beginning of the records
	mov edi, 0x00200000
uefi_purge:
	lodsq
	cmp rax, 0
	je uefi_purge_end
	stosq
	lodsq
	cmp rax, 0x300
	jl uefi_purge_remove
	stosq
	jmp uefi_purge
uefi_purge_remove:
	sub edi, 8
	jmp uefi_purge
uefi_purge_end:
	xor eax, eax			; Store a blank record
	stosq
	stosq

; Stage 3 - Round up Physical Address to next 2MiB boundary if needed and convert 4KiB pages to 1MiB pages
	mov esi, 0x00200000 - 16	; Start at the beginning of the records
	xor ecx, ecx			; MiB counter
uefi_round:
	add esi, 16
	mov rax, [rsi]			; Load the Physical Address
	cmp rax, 0			; Is it zero? (End of list)
	je uefi_round_end		; If so, bail out
	mov rbx, rax			; Copy Physical Address to RBX
	and rbx, 0x1FFFFF		; Check if any bits between 20-0 are set
	cmp rbx, 0			; If not, RBX should be 0
	jz uefi_convert
	; At this point one of the bits between 20 and 0 in the starting address are set
	; Round the starting address up to the next 2MiB
	shr rax, 21
	shl rax, 21
	add rax, 0x200000
	mov [rsi], rax
	mov rax, [rsi+8]
	shr rax, 8			; Convert 4K blocks to MiB
	sub rax, 1			; Subtract 1MiB
	mov [rsi+8], rax
	add rcx, rax			; Add to MiB counter
	jmp uefi_round
uefi_convert:
	mov rax, [rsi+8]
	shr rax, 8			; Convert 4K blocks to MiB
	mov [rsi+8], rax
	add rcx, rax			; Add to MiB counter
	jmp uefi_round
uefi_round_end:
	sub ecx, 2
	mov dword [p_mem_amount], ecx
	xor eax, eax			; Store a blank record
	stosq
	stosq
	jmp memmap_end
%endif

; Parse the memory map provided by BIOS
bios_memmap:
%ifdef BIOS
; Stage 1 - Process the E820 memory map to find all possible 2MiB pages that are free to use
; Build an available memory map at 0x200000
	xor ecx, ecx
	xor ebx, ebx			; Running counter of available MiBs
	mov esi, 0x00006000		; E820 Map location
	mov edi, 0x00200000		; 2MiB
bios_memmap_nextentry:
	add esi, 16			; Skip ESI to type marker
	mov eax, [esi]			; Load the 32-bit type marker
	cmp eax, 0			; End of the list?
	je bios_memmap_end820
	cmp eax, 1			; Is it marked as free?
	je bios_memmap_processfree
	add esi, 16			; Skip ESI to start of next entry
	jmp bios_memmap_nextentry
bios_memmap_processfree:
	; TODO Check ACPI 3.0 Extended Attributes - Bit 0 should be set
	sub esi, 16
	mov rax, [rsi]			; Physical start address
	add esi, 8
	mov rcx, [rsi]			; Physical length
	add esi, 24
	shr rcx, 20			; Convert bytes to MiB
	cmp rcx, 0			; Do we have at least 1 page?
	je bios_memmap_nextentry
	stosq
	mov rax, rcx
	stosq
	add ebx, ecx
	jmp bios_memmap_nextentry
bios_memmap_end820:

; Stage 2 - Sanitize the records
	mov esi, 0x00200000
memmap_sani:
	mov rax, [rsi]
	cmp rax, 0
	je memmap_saniend
	bt rax, 20
	jc memmap_itsodd
	add esi, 16
	jmp memmap_sani
memmap_itsodd:
	add rax, 0x100000
	mov [rsi], rax
	mov rax, [rsi+8]
	sub rax, 1
	mov [rsi+8], rax
	add esi, 16
	jmp memmap_sani
memmap_saniend:
	sub ebx, 2			; Subtract 2MiB for the CPU stacks
	mov dword [p_mem_amount], ebx
	xor eax, eax
	stosq
	stosq
%endif

memmap_end:

; Create the High Page-Directory-Pointer-Table Entries (PDPTE)
; High PDPTE is stored at 0x0000000000004000, create the first entry there
; A single PDPTE can map 1GiB with 2MiB pages
; A single PDPTE is 8 bytes in length
	mov ecx, dword [p_mem_amount]
	shr ecx, 10			; MBs -> GBs
	add rcx, 1			; Add 1. This is the number of PDPE's to make
	mov edi, 0x00004000		; location of high PDPE
	mov eax, 0x00020003		; location of first high PD. Bits 0 (P) and 1 (R/W) set
create_pdpe_high:
	stosq
	add rax, 0x00001000		; 4K later (512 records x 8 bytes)
	dec ecx
	cmp ecx, 0
	jne create_pdpe_high

; Create the High Page-Directory Entries (PDE).
; A single PDE can map 2MiB of RAM
; A single PDE is 8 bytes in length
	mov esi, 0x00200000		; Location of the available memory map
	mov edi, 0x00020000		; Location of first PDE
pde_next_range:
	lodsq				; Load the base
	xchg rax, rcx
	lodsq				; Load the length
	xchg rax, rcx
	cmp rax, 0			; Check if at end of records
	je pde_end			; Bail out if so
	cmp rax, 0x00200000
	jg skipfirst4mb
	add rax, 0x00200000		; Add 2 MiB to the base
	sub rcx, 2			; Subtract 2 MiB from the length
skipfirst4mb:
	shr ecx, 1			; Quick divide by 2 for 2 MB pages
	add rax, 0x00000083		; Bits 0 (P), 1 (R/W), and 7 (PS) set
pde_high:				; Create a 2MiB page
	stosq
	add rax, 0x00200000		; Increment by 2MiB
	cmp ecx, 0
	je pde_next_range
	dec ecx
	cmp ecx, 0
	jne pde_high
	jmp pde_next_range
pde_end:

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

	lidt [IDTR64]			; load IDT register

; Read APIC Address from MSR and enable it (if not done so already)
	mov ecx, IA32_APIC_BASE
	rdmsr				; Returns APIC in EDX:EAX
	bts eax, 11			; APIC Global Enable
	wrmsr
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

	mov rsi, msg_acpi
	call debug_msg
	call init_acpi			; Find and process the ACPI tables
	mov rsi, msg_ok
	call debug_msg

	mov rsi, msg_bsp
	call debug_msg
	call init_cpu			; Configure the BSP CPU
	call init_hpet			; Configure the HPET
	mov rsi, msg_ok
	call debug_msg

; Visual Debug (3/4)
	mov ebx, 4
	call debug_block

	mov rsi, msg_pic
	call debug_msg
	call init_pic			; Configure the PIC(s), activate interrupts
	mov rsi, msg_ok
	call debug_msg

	mov rsi, msg_smp
	call debug_msg
	call init_smp			; Init of SMP, deactivate interrupts
	mov rsi, msg_ok
	call debug_msg

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
	mov eax, [p_mem_amount]
	and eax, 0xFFFFFFFE
	stosd

	mov di, 0x5030
	mov al, [p_IOAPICCount]
	stosb
	mov al, [p_IOAPICIntSourceC]
	stosb

	mov di, 0x5040
	mov rax, [p_HPET_Address]
	stosq
	mov eax, [p_HPET_Frequency]
	stosd
	mov ax, [p_HPET_CounterMin]
	stosw
	mov al, [p_HPET_Timers]
	stosb

	mov di, 0x5060
	mov rax, [p_LocalAPICAddress]
	stosq

; Copy the data we received from UEFI/BIOS
	mov di, 0x5080
	mov rax, [0x00005F00]		; Base address of video memory
	stosq
	mov eax, [0x00005F00 + 0x10]	; X and Y resolution (16-bits each)
	stosd
	mov eax, [0x00005F00 + 0x14]	; Pixels per scan line
	stosw
	mov ax, 32
	stosw

; Set the Linear Frame Buffer to use write-combining
	mov eax, 0x80000001
	cpuid
	bt edx, 26			; Page1GB
	jnc lfb_wc_2MB
; Set the 1GB page the frame buffer is in to WC - PAT = 1, PCD = 0, PWT = 1
lfb_wc_1GB:
	mov rax, [0x00005F00]		; Base address of video memory
	mov rbx, 0x100000000		; Compare to 4GB
	cmp rax, rbx
	jle lfb_wc_end			; If less, don't set WC
	shr rax, 27			; Quick divide
	mov edi, 0x00003000		; Address of low PDPTEs 
	add edi, eax			; Add the offset
	mov eax, [edi]			; Load the 8-byte value
	or ax, 0x1008			; Set bits 12 (PAT) and 3 (PWT)
	and ax, 0xFFEF			; Clear bit 4 (PCD)
	mov [edi], eax			; Write it back
	jmp lfb_wc_end
; Set the relevant 2MB pages the frame buffer is in to WC
lfb_wc_2MB:
	mov ecx, 4			; 4 2MiB pages - TODO only set the pages needed
	mov edi, 0x00010000
	mov rax, [0x00005F00]		; Base address of video memory
	shr rax, 18
	add rdi, rax
lfb_wc_2MB_nextpage:
	mov eax, [edi]			; Load the 8-byte value
	or ax, 0x1008			; Set bits 12 (PAT) and 3 (PWT)
	and ax, 0xFFEF			; Clear bit 4 (PCD)
	mov [edi], eax			; Write it back
	add edi, 8
	sub ecx, 1
	jnz lfb_wc_2MB_nextpage
lfb_wc_end:
	mov rax, cr3			; Flush TLB
	mov cr3, rax
	wbinvd				; Flush Cache

; Store the PCI(e) data
	mov di, 0x5090
	mov ax, [p_PCIECount]
	stosw

; Move the trailing binary to its final location
	mov esi, 0x8000+PURE64SIZE	; Memory offset to end of pure64.sys
	mov edi, 0x100000		; Destination address at the 1MiB mark
	mov ecx, ((32768 - PURE64SIZE) / 8)
	rep movsq			; Copy 8 bytes at a time

; Visual Debug (4/4)
	mov ebx, 6
	call debug_block

	mov rsi, msg_kernel
	call debug_msg

%ifdef BIOS
	cmp byte [p_BootDisk], 'F'	; Check if sys is booted from floppy?
	jnz clear_regs
	call read_floppy		; Then load whole floppy at memory
%endif

; Clear all registers (skip the stack pointer)
clear_regs:
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
%include "init/serial.asm"
%include "init/hpet.asm"
%include "init/smp.asm"
%ifdef BIOS
%include "fdc/dma.asm"
%include "fdc/fdc_64.asm"
%endif
%include "interrupt.asm"
%include "sysvar.asm"


; -----------------------------------------------------------------------------
; debug_block - Create a block of colour on the screen
; IN:	EBX = Index #
debug_block:
	push rax
	push rbx
	push rcx
	push rdx
	push rdi

	; Calculate parameters
	push rbx
	push rax
	xor edx, edx
	xor eax, eax
	xor ebx, ebx
	mov ax, [0x00005F00 + 0x12]	; Screen Y
	sub ax, 16			; Upper row
	shr ax, 1			; Quick divide by 2
	mov bx, [0x00005F00 + 0x10]	; Screen X
	shl ebx, 2			; Quick multiply by 4
	mul ebx				; Multiply EDX:EAX by EBX
	mov rdi, [0x00005F00]		; Frame buffer base
	add rdi, rax			; Offset is ((screeny - 8) / 2 + screenx * 4)
	pop rax
	pop rbx
	xor edx, edx
	mov dx, [0x00005F00 + 0x14]	; PixelsPerScanLine
	shl edx, 2			; Quick multiply by 4 for line offset
	xor ecx, ecx
	mov cx, [0x00005F00 + 0x10]	; Screen X
	shr cx, 4			; CX = total amount of 8-pixel wide blocks
	sub cx, 4
	add ebx, ecx
	shl ebx, 5			; Quick multiply by 32 (8 pixels by 4 bytes each)
	add rdi, rbx

	; Draw the 8x8 pixel block
	mov ebx, 8			; 8 pixels tall
	mov eax, 0x00F7CA54		; Return Infinity Yellow/Orange
nextline:
	mov ecx, 8			; 8 pixels wide
	rep stosd
	add rdi, rdx			; Add line offset
	sub rdi, 8*4			; 8 pixels by 4 bytes each
	dec ebx
	jnz nextline

	pop rdi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
; -----------------------------------------------------------------------------


%ifdef BIOS
; -----------------------------------------------------------------------------
; debug_progressbar
; IN:	EBX = Index #
; Note:	During a floppy load this function gets called 40 times
debug_progressbar:
	mov rdi, [0x00005F00]		; Frame buffer base
	add rdi, rbx
	; Offset to start of progress bar location
	; 1024 - screen width
	; 367 - Line # to display bar
	; 32 - Offset to start of the progress blocks
	; 512 - Middle of screen
	; 4 - 32-bit pixels
	add rdi, ((1024 * 387 - 32) + 512) * 4

	; Draw the pixel
	mov eax, 0x00FFFFFF		; White
	stosd
	add rdi, (1024 * 4) - 4
	stosd

	ret
; -----------------------------------------------------------------------------
%endif


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


; -----------------------------------------------------------------------------
; debug_dump_(rax|eax|ax|al) -- Dump content of RAX, EAX, AX, or AL
;  IN:	RAX/EAX/AX/AL = content to dump
; OUT:	Nothing, all registers preserved
debug_dump_rax:
	rol rax, 8
	call debug_dump_al
	rol rax, 8
	call debug_dump_al
	rol rax, 8
	call debug_dump_al
	rol rax, 8
	call debug_dump_al
	rol rax, 32
debug_dump_eax:				; RAX is used here instead of EAX to preserve the upper 32-bits
	rol rax, 40
	call debug_dump_al
	rol rax, 8
	call debug_dump_al
	rol rax, 16
debug_dump_ax:
	rol ax, 8
	call debug_dump_al
	rol ax, 8
debug_dump_al:
	push rax			; Save RAX
	push ax				; Save AX for the low nibble
	shr al, 4			; Shift the high 4 bits into the low 4, high bits cleared
	or al, '0'			; Add "0"
	cmp al, '9'+1			; Digit?
	jl debug_dump_al_h		; Yes, store it
	add al, 7			; Add offset for character "A"
debug_dump_al_h:
	call debug_msg_char
	pop ax				; Restore AX
	and al, 0x0F			; Keep only the low 4 bits
	or al, '0'			; Add "0"
	cmp al, '9'+1			; Digit?
	jl debug_dump_al_l		; Yes, store it
	add al, 7			; Add offset for character "A"
debug_dump_al_l:
	call debug_msg_char
	pop rax				; Restore RAX
	ret
; -----------------------------------------------------------------------------


EOF:
	db 0xDE, 0xAD, 0xC0, 0xDE

; Pad to an even KB file
times PURE64SIZE-($-$$) db 0x90


; =============================================================================
; EOF
