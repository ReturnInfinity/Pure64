; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2012 Return Infinity -- see LICENSE.TXT
;
; System Variables
; =============================================================================


;CONFIG
cfg_smpinit:		db 1	; By default SMP is enabled. Set to 0 to disable.
cfg_vesa:		db 0	; By default VESA is disabled. Set to 1 to enable.
cfg_default:		db 0	; By default we don't need a config file so set to 0. If a config file is found set to 1.
cfg_e820:		db 1	; By default E820 should be present. Pure64 will set this to 0 if not found/usable.
cfg_mbr:		db 0	; Did we boot off of a disk with a proper MBR
cfg_hdd:		db 0	; Was a bootable drive detected

; Memory locations
E820Map:		equ 0x0000000000004000
InfoMap:		equ 0x0000000000005000
SystemVariables:	equ 0x0000000000005A00
VBEModeInfoBlock:	equ 0x0000000000005C00
hdbuffer:		equ 0x0000000000060000  ; 32768 bytes = 0x6000 -> 0xDFFF VERIFY THIS!!!
hdbuffer1:		equ 0x000000000006E000  ; 512 bytes = 0xE000 -> 0xE1FF VERIFY THIS!!!

%ifidn HDD,AHCI
ahci_cmdlist:		equ 0x0000000000070000	; 4096 bytes	0x070000 -> 0x071FFF
ahci_cmdtable:		equ 0x0000000000072000	; 57344 bytes	0x072000 -> 0x07FFFF
%endif

; DQ - Starting at offset 0, increments by 0x8
os_ACPITableAddress:	equ SystemVariables + 0x00
screen_cursor_offset:	equ SystemVariables + 0x08
hd1_maxlba:		equ SystemVariables + 0x10
os_Counter_Timer:	equ SystemVariables + 0x18
os_Counter_RTC:		equ SystemVariables + 0x20
os_LocalAPICAddress:	equ SystemVariables + 0x28
os_IOAPICAddress:	equ SystemVariables + 0x30
os_HPETAddress:		equ SystemVariables + 0x38
sata_base:		equ SystemVariables + 0x40

; DD - Starting at offset 128, increments by 4
hd1_size:		equ SystemVariables + 128
os_BSP:			equ SystemVariables + 132
drive_port:		equ SystemVariables + 136
mem_amount:		equ SystemVariables + 140
%ifidn FS,FAT16
fat16_FatStart:		equ SystemVariables + 144
fat16_TotalSectors:	equ SystemVariables + 148
fat16_DataStart:	equ SystemVariables + 152
fat16_RootStart:	equ SystemVariables + 156
fat16_PartitionOffset:	equ SystemVariables + 160
%endif

; DW - Starting at offset 256, increments by 2
cpu_speed:		equ SystemVariables + 256
cpu_activated:		equ SystemVariables + 258
cpu_detected:		equ SystemVariables + 260
ata_base:		equ SystemVariables + 262
%ifidn FS,FAT16
fat16_BytesPerSector:	equ SystemVariables + 264
fat16_ReservedSectors:	equ SystemVariables + 266
fat16_SectorsPerFat:	equ SystemVariables + 268
fat16_RootDirEnts:	equ SystemVariables + 270
%endif

; DB - Starting at offset 384, increments by 1
hd1_enable:		equ SystemVariables + 384
hd1_lba48:		equ SystemVariables + 385
screen_cursor_x:	equ SystemVariables + 386
screen_cursor_y:	equ SystemVariables + 387
%ifidn FS,FAT16
fat16_SectorsPerCluster:	equ SystemVariables + 388
fat16_Fats:		equ SystemVariables + 389
%endif
memtempstring:		equ SystemVariables + 390
speedtempstring:	equ SystemVariables + 400
cpu_amount_string:	equ SystemVariables + 410
hdtempstring:		equ SystemVariables + 420
os_key:			equ SystemVariables + 421
os_IOAPICCount:		equ SystemVariables + 424

;MISC
screen_cols:		db 80
screen_rows:		db 25
hextable: 		db '0123456789ABCDEF'

;STRINGS
pure64:			db 'Pure64 - ', 0
kernelerror:		db 'ERROR: Could not find ', 0
kernelname:		db KERNEL, 0
msg_done:		db ' Done', 0
msg_CPU:		db '[CPU: ', 0
msg_mhz:		db 'MHz x', 0
msg_MEM:		db ']  [MEM: ', 0
msg_mb:			db ' MiB]', 0
msg_HDD:		db '  [HDD: ', 0
msg_gb:			db ' GiB]', 0
msg_loadingkernel:	db 'Loading kernel...', 0
msg_startingkernel:	db 'Starting kernel...', 0
no64msg:		db 'ERROR: This computer does not support 64-Bit mode. Press any key to reboot.', 0
initStartupMsg:		db 'Pure64 v0.5.1 - http://www.returninfinity.com', 13, 10, 13, 10, 'Initializing system... ', 0
msg_date:		db '2012/04/15', 0
;hdd_setup_no_sata:	db 'No supported SATA Controller detected', 0
hdd_setup_no_disk:	db 'No HDD detected', 0
hdd_setup_read_error:	db 'ERROR: Could not read HDD', 0
fs_read_error:		db 'ERROR: Could not read filesystem', 0

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
; Mandatory information for VBE 3.0 and above
;VBEModeInfoBlock.LinBytesPerScanLine	dw 0	; bytes per scan line for linear modes
;VBEModeInfoBlock.BnkNumberOfImagePages	db 0	; number of images for banked modes
;VBEModeInfoBlock.LinNumberOfImagePages	db 0	; number of images for linear modes
;VBEModeInfoBlock.LinRedMaskSize	db 0	; size of direct color red mask (linear modes)
;VBEModeInfoBlock.LinRedFieldPosition	db 0	; bit position of lsb of red mask (linear modes)
;VBEModeInfoBlock.LinGreenMaskSize	db 0	; size of direct color green mask  (linear modes)
;VBEModeInfoBlock.LinGreenFieldPosition	db 0	; bit position of lsb of green mask (linear modes)
;VBEModeInfoBlock.LinBlueMaskSize	db 0	; size of direct color blue mask  (linear modes)
;VBEModeInfoBlock.LinBlueFieldPosition	db 0	; bit position of lsb of blue mask (linear modes)
;VBEModeInfoBlock.LinRsvdMaskSize	db 0	; size of direct color reserved mask (linear modes)
;VBEModeInfoBlock.LinRsvdFieldPosition	db 0	; bit position of lsb of reserved mask (linear modes)
;VBEModeInfoBlock.MaxPixelClock		dd 0	; maximum pixel clock (in Hz) for graphics mode


; -----------------------------------------------------------------------------
align 16
GDTR64:					; Global Descriptors Table Register
	dw gdt64_end - gdt64 - 1	; limit of GDT (size minus one)
	dq 0x0000000000001000		; linear address of GDT

gdt64:					; This structure is copied to 0x0000000000001000
SYS64_NULL_SEL equ $-gdt64		; Null Segment
	dq 0x0000000000000000
SYS64_CODE_SEL equ $-gdt64		; Code segment, read/execute, nonconforming
	dq 0x0020980000000000		; 0x00209A0000000000
SYS64_DATA_SEL equ $-gdt64		; Data segment, read/write, expand down
	dq 0x0000900000000000		; 0x0020920000000000
gdt64_end:

IDTR64:					; Interrupt Descriptor Table Register
	dw 256*16-1			; limit of IDT (size minus one) (4096 bytes - 1)
	dq 0x0000000000000000		; linear address of IDT
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
