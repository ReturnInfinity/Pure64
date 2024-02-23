; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2023 Return Infinity -- see LICENSE.TXT
;
; System Variables
; =============================================================================


message: db 10, 'Pure64 OK', 10

;CONFIG
cfg_smpinit:		db 1		; By default SMP is enabled. Set to 0 to disable.

; Memory locations
E820Map:		equ 0x0000000000004000
InfoMap:		equ 0x0000000000005000
SystemVariables:	equ 0x0000000000005800
VBEModeInfoBlock:	equ 0x0000000000005C00		; 256 bytes

; DQ - Starting at offset 0, increments by 0x8
os_ACPITableAddress:	equ SystemVariables + 0x00
os_LocalX2APICAddress:	equ SystemVariables + 0x10
os_Counter_Timer:	equ SystemVariables + 0x18
os_Counter_RTC:		equ SystemVariables + 0x20
os_LocalAPICAddress:	equ SystemVariables + 0x28
os_HPETAddress:		equ SystemVariables + 0x38

; DD - Starting at offset 0x80, increments by 4
os_BSP:			equ SystemVariables + 0x80
mem_amount:		equ SystemVariables + 0x84	; in MiB

; DW - Starting at offset 0x100, increments by 2
cpu_speed:		equ SystemVariables + 0x100
cpu_activated:		equ SystemVariables + 0x102
cpu_detected:		equ SystemVariables + 0x104

; DB - Starting at offset 0x180, increments by 1
os_IOAPICCount:		equ SystemVariables + 0x180
BootMode:		equ SystemVariables + 0x181	; 'U' for UEFI, otherwise BIOS
os_IOAPICIntSourceC:	equ SystemVariables + 0x182

; Lists - Starting at offset 0x200
os_IOAPICAddress:	equ SystemVariables + 0x200	; 16 bytes each
os_IOAPICIntSource:	equ SystemVariables + 0x280	; 8 bytes each

align 16
GDTR32:					; Global Descriptors Table Register
dw gdt32_end - gdt32 - 1		; limit of GDT (size minus one)
dq gdt32				; linear address of GDT

align 16
gdt32:
dq 0x0000000000000000			; Null descriptor
dq 0x00CF9A000000FFFF			; 32-bit code descriptor
					; 55 Granularity 4KiB, 54 Size 32bit, 47 Present, 44 Code/Data, 43 Executable, 41 Readable
dq 0x00CF92000000FFFF			; 32-bit data descriptor
					; 55 Granularity 4KiB, 54 Size 32bit, 47 Present, 44 Code/Data, 41 Writeable
gdt32_end:

; -----------------------------------------------------------------------------
align 16
GDTR64:					; Global Descriptors Table Register
dw gdt64_end - gdt64 - 1		; limit of GDT (size minus one)
dq 0x0000000000001000			; linear address of GDT

gdt64:					; This structure is copied to 0x0000000000001000
SYS64_NULL_SEL equ $-gdt64		; Null Segment
dq 0x0000000000000000
SYS64_CODE_SEL equ $-gdt64		; Code segment, read/execute, nonconforming
dq 0x00209A0000000000			; 53 Long mode code, 47 Present, 44 Code/Data, 43 Executable, 41 Readable
SYS64_DATA_SEL equ $-gdt64		; Data segment, read/write, expand down
dq 0x0000920000000000			; 47 Present, 44 Code/Data, 41 Writable
gdt64_end:

IDTR64:					; Interrupt Descriptor Table Register
dw 256*16-1				; limit of IDT (size minus one) (4096 bytes - 1)
dq 0x0000000000000000			; linear address of IDT
; -----------------------------------------------------------------------------

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

; =============================================================================
; EOF
