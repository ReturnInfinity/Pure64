; =============================================================================
; Pure64 MBR -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; This Master Boot Record will load Pure64 from a pre-defined location on the
; floppy drive without making use of the file system.
;
; In this code we are not expecting any formatted drive. A small
; check is made to make sure Pure64 was loaded by comparing a signature.
; =============================================================================

; Default location of the second stage boot loader. This loads
; 32 KiB from sector 2 into memory at 0x8000
; Extended read int 0x13 AH=0x42 not possible for floppy devices.
; http://www.ctyme.com/intr/rb-0607.htm

; Set the desired screen resolution values below
Horizontal_Resolution		equ 1024
Vertical_Resolution		equ 768

BITS 16
org 0x7C00

entry:
	jmp bootcode		; Jump past the BPB data
	nop

; BPB (BIOS Parameter Block)
dq 0		; OEM identifier
dw 0		; Bytes per sector
db 0		; Sectors per cluster
dw 0		; Reserved sectors
db 0		; Number of FATs
dw 0		; Number of root directory entries
dw 0		; The total sectors in the logical volume
db 0		; Media descriptor type
dw 0		; Number of sectors per FAT
dw 0		; Number of sectors per track
dw 0		; Number of heads or sides on the storage media
dd 0		; Number of hidden sectors
dd 0		; Large sector count

bootcode:
	cli				; Disable interrupts
	cld				; Clear direction flag
	xor eax, eax
	mov ss, ax
	mov es, ax
	mov ds, ax
	mov sp, 0x7C00
	sti				; Enable interrupts

	mov [DriveNumber], dl		; BIOS passes drive number in DL

; Get the BIOS E820 Memory Map
; https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_EAX_=_0xE820
; The code below is from https://wiki.osdev.org/Detecting_Memory_(x86)#Getting_an_E820_Memory_Map
; inputs: es:di -> destination buffer for 24 byte entries
; outputs: bp = entry count, trashes all registers except esi
; The function below creates a memory map at address 0x6000 and the records are:
; 64-bit Base
; 64-bit Length
; 32-bit Type (1 = normal, 2 reserved, ACPI reclaimable)
; 32-bit ACPI
; 64-bit Padding
do_e820:
	mov edi, 0x00006000		; location that memory map will be stored to
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
	add di, 32			; Pad to 32 bytes for each record
skipent:
	test ebx, ebx			; if ebx resets to 0, list is complete
	jne e820lp
nomemmap:
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

	mov cx, 0x4000 - 1		; Start looking from here
VBESearch:
	inc cx
	mov bx, cx			; Mode is saved to BX for the set command later
	cmp cx, 0x5000
	je halt
	mov edi, VBEModeInfoBlock	; VBE data will be stored at this address
	mov ax, 0x4F01			; VESA SuperVGA BIOS - GET SuperVGA MODE INFORMATION - http://www.ctyme.com/intr/rb-0274.htm
	int 0x10
	cmp ax, 0x004F			; Return value in AX should equal 0x004F if command supported and successful
	jne VBESearch			; Try next mode
	cmp byte [VBEModeInfoBlock.BitsPerPixel], 32 ; Desired bit depth
	jne VBESearch			; If not equal, try next mode
	cmp word [VBEModeInfoBlock.XResolution], Horizontal_Resolution ; Desired XRes here
	jne VBESearch
	cmp word [VBEModeInfoBlock.YResolution], Vertical_Resolution ; Desired YRes here
	jne VBESearch
	or bx, 0x4000			; Use linear/flat frame buffer model (set bit 14)
	mov ax, 0x4F02			; VESA SuperVGA BIOS - SET SuperVGA VIDEO MODE - http://www.ctyme.com/intr/rb-0275.htm
	int 0x10
	cmp ax, 0x004F			; Return value in AX should equal 0x004F if supported and successful
	jne halt

	; Read the 2nd stage boot loader into memory.
	; Load 4 cylinders - 7 sectors
	mov byte [cylinder], 0 
	mov word [sec_buff], 0x7E00	; For simplicity load whole cylinder but jump to 0x8000
	call load_sector
	mov byte [head], 1
	mov word [sec_buff], 0xA200
	call load_sector
	mov byte [cylinder], 1 
	mov byte [head], 0
	mov word [sec_buff], 0xC600
	call load_sector
	mov byte [head], 1
	mov word [sec_buff], 0xEA00
	mov byte [nr_sect], 11		; Remains just 11 for 16-bit memory limit
	call load_sector

	; Stop floppy drive motor
	push dx
	mov dx, 0x3F2			; Floppy DIGITAL_OUTPUT_REGISTER
	mov al, 0
	out dx, al
	pop dx

	mov bl, 'B'			; 'B' as we booted via BIOS
	mov bh, 'F'			; 'F' as we booted via floppy drive

; At this point we are done with real mode and BIOS interrupts. Jump to 32-bit mode.
	cli				; No more interrupts
	lgdt [cs:GDTR32]		; Load GDT register
	mov eax, cr0
	or al, 0x01			; Set protected mode bit
	mov cr0, eax
	jmp 8:0x8000			; Jump to 32-bit protected mode

halt:
	hlt
	jmp halt
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
load_sector:
	xor ah, ah			; Reset drive function
	int 0x13

	mov bx, [sec_buff]
	mov dl, [DriveNumber]
	mov dh, [head]
	mov al, [nr_sect]
	mov ch, [cylinder]
	mov cl, [sector]
	mov ah, 0x2
	int 0x13
	jnc exit			; If the carry flag is clear, it worked
	loop load_sector		; Read attempts
exit:
	ret
;------------------------------------------------------------------------------


align 16

GDTR32:					; Global Descriptors Table Register
dw gdt32_end - gdt32 - 1		; limit of GDT (size minus one)
dq gdt32				; linear address of GDT

align 16
gdt32:
dw 0x0000, 0x0000, 0x0000, 0x0000	; Null descriptor
dw 0xFFFF, 0x0000, 0x9A00, 0x00CF	; 32-bit code descriptor
dw 0xFFFF, 0x0000, 0x9200, 0x00CF	; 32-bit data descriptor
gdt32_end:

align 4

; CHS
sector		db 1
cylinder	db 0
head		db 0
nr_sect		db 18
sec_buff	dw 0

DriveNumber db 0x00

times 510-$+$$ db 0

dw 0xAA55				; Boot signature

VBEModeInfoBlock: equ 0x5F00
; VESA
; Mandatory information for all VBE revisions
VBEModeInfoBlock.ModeAttributes		equ VBEModeInfoBlock + 0	; DW - mode attributes
VBEModeInfoBlock.WinAAttributes		equ VBEModeInfoBlock + 2	; DB - window A attributes
VBEModeInfoBlock.WinBAttributes		equ VBEModeInfoBlock + 3	; DB - window B attributes
VBEModeInfoBlock.WinGranularity		equ VBEModeInfoBlock + 4	; DW - window granularity in KB
VBEModeInfoBlock.WinSize		equ VBEModeInfoBlock + 6	; DW - window size in KB
VBEModeInfoBlock.WinASegment		equ VBEModeInfoBlock + 8	; DW - window A start segment
VBEModeInfoBlock.WinBSegment		equ VBEModeInfoBlock + 10	; DW - window B start segment
VBEModeInfoBlock.WinFuncPtr		equ VBEModeInfoBlock + 12	; DD - real mode pointer to window function
VBEModeInfoBlock.BytesPerScanLine	equ VBEModeInfoBlock + 16	; DW - bytes per scan line
; Mandatory information for VBE 1.2 and above
VBEModeInfoBlock.XResolution		equ VBEModeInfoBlock + 18	; DW - horizontal resolution in pixels or characters
VBEModeInfoBlock.YResolution		equ VBEModeInfoBlock + 20	; DW - vertical resolution in pixels or characters
VBEModeInfoBlock.XCharSize		equ VBEModeInfoBlock + 22	; DB - character cell width in pixels
VBEModeInfoBlock.YCharSize		equ VBEModeInfoBlock + 23	; DB - character cell height in pixels
VBEModeInfoBlock.NumberOfPlanes		equ VBEModeInfoBlock + 24	; DB - number of memory planes
VBEModeInfoBlock.BitsPerPixel		equ VBEModeInfoBlock + 25	; DB - bits per pixel
VBEModeInfoBlock.NumberOfBanks		equ VBEModeInfoBlock + 26	; DB - number of banks
VBEModeInfoBlock.MemoryModel		equ VBEModeInfoBlock + 27	; DB - memory model type
VBEModeInfoBlock.BankSize		equ VBEModeInfoBlock + 28	; DB - bank size in KB
VBEModeInfoBlock.NumberOfImagePages	equ VBEModeInfoBlock + 29	; DB - number of image pages
VBEModeInfoBlock.Reserved		equ VBEModeInfoBlock + 30	; DB - reserved (0x00 for VBE 1.0-2.0, 0x01 for VBE 3.0)
; Direct Color fields (required for direct/6 and YUV/7 memory models)
VBEModeInfoBlock.RedMaskSize		equ VBEModeInfoBlock + 31	; DB - size of direct color red mask in bits
VBEModeInfoBlock.RedFieldPosition	equ VBEModeInfoBlock + 32	; DB - bit position of lsb of red mask
VBEModeInfoBlock.GreenMaskSize		equ VBEModeInfoBlock + 33	; DB - size of direct color green mask in bits
VBEModeInfoBlock.GreenFieldPosition	equ VBEModeInfoBlock + 34	; DB - bit position of lsb of green mask
VBEModeInfoBlock.BlueMaskSize		equ VBEModeInfoBlock + 35	; DB - size of direct color blue mask in bits
VBEModeInfoBlock.BlueFieldPosition	equ VBEModeInfoBlock + 36	; DB - bit position of lsb of blue mask
VBEModeInfoBlock.RsvdMaskSize		equ VBEModeInfoBlock + 37	; DB - size of direct color reserved mask in bits
VBEModeInfoBlock.RsvdFieldPosition	equ VBEModeInfoBlock + 38	; DB - bit position of lsb of reserved mask
VBEModeInfoBlock.DirectColorModeInfo	equ VBEModeInfoBlock + 39	; DB - direct color mode attributes
; Mandatory information for VBE 2.0 and above
VBEModeInfoBlock.PhysBasePtr		equ VBEModeInfoBlock + 40	; DD - physical address for flat memory frame buffer
VBEModeInfoBlock.Reserved1		equ VBEModeInfoBlock + 44	; DD - Reserved - always set to 0
VBEModeInfoBlock.Reserved2		equ VBEModeInfoBlock + 48	; DD - Reserved - always set to 0

; EOF
