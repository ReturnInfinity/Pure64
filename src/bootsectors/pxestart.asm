; =============================================================================
; Pure64 PXE Start -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
;
; This is a stub file for loading Pure64 and a kernel via PXE.
;
; Windows - copy /b pxestart.bin + pure64.sys + kernel64.sys pxeboot.bin
; Unix - cat pxestart.bin pure64.sys kernel64.sys > pxeboot.bin
;
; Max size of the resulting pxeboot.bin is 33792 bytes. 1K for the PXE loader
; stub and up to 32KiB for the code/data. PXE loads the file to address
; 0x00007C00 (Just like a boot sector).
;
; File Sizes
; pxestart.bin	 1024 bytes
; pure64.sys	 6144 bytes
; kernel64.sys	16384 bytes (or so)
; =============================================================================


USE16
org 0x7C00

start:
	cli				; Disable interrupts
	xor eax, eax
	mov ss, ax
	mov es, ax
	mov ds, ax
	mov sp, 0x7C00
	sti				; Enable interrupts

; Make sure the screen is set to 80x25 color text mode
	mov ax, 0x0003			; Set to normal (80x25 text) video mode
	int 0x10

; Get the BIOS E820 Memory Map
; use the INT 0x15, eax= 0xE820 BIOS function to get a memory map
; inputs: es:di -> destination buffer for 24 byte entries
; outputs: bp = entry count, trashes all registers except esi
do_e820:
	mov edi, 0x00004000		; location that memory map will be stored to
	xor ebx, ebx			; ebx must be 0 to start
	xor bp, bp			; keep an entry count in bp
	mov edx, 0x0534D4150		; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24			; ask for 24 bytes
	int 0x15
	jc nomemmap			; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150		; Some BIOSes apparently trash this register?
	cmp eax, edx			; on success, eax must have been reset to "SMAP"
	jne nomemmap
	test ebx, ebx			; ebx = 0 implies list is only 1 entry long (worthless)
	je nomemmap
	jmp jmpin
e820lp:
	mov eax, 0xe820			; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24			; ask for 24 bytes again
	int 0x15
	jc memmapend			; carry set means "end of list already reached"
	mov edx, 0x0534D4150		; repair potentially trashed register
jmpin:
	jcxz skipent			; skip any 0 length entries
	cmp cl, 20			; got a 24 byte ACPI 3.X response?
	jbe notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je skipent
notext:
	mov ecx, [es:di + 8]		; get lower dword of memory region length
	test ecx, ecx			; is the qword == 0?
	jne goodent
	mov ecx, [es:di + 12]		; get upper dword of memory region length
	jecxz skipent			; if length qword is 0, skip entry
goodent:
	inc bp				; got a good entry: ++count, move to next storage spot
	add di, 32
skipent:
	test ebx, ebx			; if ebx resets to 0, list is complete
	jne e820lp
nomemmap:
;	mov byte [cfg_e820], 0		; No memory map function
memmapend:
	xor eax, eax			; Create a blank record for termination (32 bytes)
	mov ecx, 8
	rep stosd

; Enable the A20 gate
set_A20:
	in al, 0x64
	test al, 0x02
	jnz set_A20
	mov al, 0xD1
	out 0x64, al
check_A20:
	in al, 0x64
	test al, 0x02
	jnz check_A20
	mov al, 0xDF
	out 0x60, al

	mov si, msg_Load		; Print message
	call print_string_16

	mov eax, [0x8000]
	cmp eax, 0x00018BE9		; Match against the Pure64 binary
	jne magic_fail

; At this point we are done with real mode and BIOS interrupts. Jump to 32-bit mode.
	cli				; No more interrupts
	lgdt [cs:GDTR32]		; Load GDT register
	mov eax, cr0
	or al, 0x01			; Set protected mode bit
	mov cr0, eax
	jmp 8:0x8000			; Jump to 32-bit protected mode

magic_fail:
	mov si, msg_MagicFail
	call print_string_16
halt:
	hlt
	jmp halt
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; 16-bit Function to print a string to the screen
; input: SI - Address of start of string
print_string_16:			; Output string in SI to screen
	pusha
	mov ah, 0x0E			; int 0x10 teletype function
.repeat:
	lodsb				; Get char from string
	test al, al
	jz .done			; If char is zero, end of string
	int 0x10			; Otherwise, print it
	jmp short .repeat
.done:
	popa
	ret
;------------------------------------------------------------------------------


msg_Load db "Loading via PXE... ", 0
msg_MagicFail db "Error!", 0

align 16
GDTR32:					; Global Descriptors Table Register
dw gdt32_end - gdt32 - 1		; limit of GDT (size minus one)
dq gdt32				; linear address of GDT

align 16
gdt32:
dq 0x0000000000000000			; Null desciptor
dq 0xFFFF00009A0000CF			; 32-bit code descriptor
dq 0xFFFF0000920000CF			; 32-bit data descriptor
gdt32_end:

times 510-$+$$ db 0			; Pad out for a normal boot sector

sign dw 0xAA55				; BIOS boot sector signature

times 1024-$+$$ db 0			; Padding so that Pure64 will be aligned at 0x8000
