; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; System Variables
; =============================================================================


; Messages
msg_pure64:		db 13, 10, 13, 10, '[ Pure64 ]', 0
msg_ok:			db 'ok', 0
msg_error:		db 'ERROR', 0
msg_exception:		db 'EXCEPTION 0x', 0
msg_pml4:		db 13, 10, 'pml4 ', 0
msg_boot:		db 13, 10, 'boot ', 0
msg_bios:		db 'bios', 0
msg_uefi:		db 'uefi', 0
msg_acpi:		db 13, 10, 'acpi ', 0
msg_bsp:		db 13, 10, 'bsp ', 0
msg_pic:		db 13, 10, 'pic ', 0
msg_smp:		db 13, 10, 'smp ', 0
msg_kernel:		db 13, 10, 'kernel start', 13, 10, 0

;CONFIG
cfg_smpinit:		db 1		; By default SMP is enabled. Set to 0 to disable.

; Memory locations
InfoMap:		equ 0x0000000000005000
IM_DetectedCoreIDs:	equ 0x0000000000005100		; 1 byte per entry - Each byte is the APIC ID of a core
IM_PCIE:		equ 0x0000000000005400		; 16 bytes per entry
IM_IOAPICAddress:	equ 0x0000000000005600		; 16 bytes per entry
IM_IOAPICIntSource:	equ 0x0000000000005700		; 8 bytes per entry
SystemVariables:	equ 0x0000000000005800
IM_ActivedCoreIDs:	equ 0x0000000000005E00		; 1 byte per entry - 1 if the core was activated
VBEModeInfoBlock:	equ 0x0000000000005F00		; 256 bytes

; DQ - Starting at offset 0, increments by 0x8
p_ACPITableAddress:	equ SystemVariables + 0x00
p_LocalAPICAddress:	equ SystemVariables + 0x10
p_Counter_Timer:	equ SystemVariables + 0x18
p_Counter_RTC:		equ SystemVariables + 0x20
p_HPET_Address:		equ SystemVariables + 0x28

; DD - Starting at offset 0x80, increments by 4
p_BSP:			equ SystemVariables + 0x80
p_mem_amount:		equ SystemVariables + 0x84	; in MiB
p_HPET_Frequency:	equ SystemVariables + 0x88

; DW - Starting at offset 0x100, increments by 2
p_cpu_speed:		equ SystemVariables + 0x100
p_cpu_activated:	equ SystemVariables + 0x102
p_cpu_detected:		equ SystemVariables + 0x104
p_PCIECount:		equ SystemVariables + 0x106
p_HPET_CounterMin:	equ SystemVariables + 0x108

; DB - Starting at offset 0x180, increments by 1
p_IOAPICCount:		equ SystemVariables + 0x180
p_BootMode:		equ SystemVariables + 0x181	; 'U' for UEFI, otherwise BIOS
p_IOAPICIntSourceC:	equ SystemVariables + 0x182
p_x2APIC:		equ SystemVariables + 0x183
p_HPET_Timers:		equ SystemVariables + 0x184
p_BootDisk:		equ SystemVariables + 0x185	; 'F' for Floppy drive

align 16
GDTR32:					; Global Descriptors Table Register
dw gdt32_end - gdt32 - 1		; limit of GDT (size minus one)
dq gdt32				; linear address of GDT

align 16
gdt32:
SYS32_NULL_SEL equ $-gdt32		; Null Segment
dq 0x0000000000000000
SYS32_CODE_SEL equ $-gdt32		; 32-bit code descriptor
dq 0x00CF9A000000FFFF			; 55 Granularity 4KiB, 54 Size 32bit, 47 Present, 44 Code/Data, 43 Executable, 41 Readable
SYS32_DATA_SEL equ $-gdt32		; 32-bit data descriptor		
dq 0x00CF92000000FFFF			; 55 Granularity 4KiB, 54 Size 32bit, 47 Present, 44 Code/Data, 41 Writeable
gdt32_end:

align 16
tGDTR64:				; Global Descriptors Table Register
dw gdt64_end - gdt64 - 1		; limit of GDT (size minus one)
dq gdt64				; linear address of GDT

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


; =============================================================================
; EOF
