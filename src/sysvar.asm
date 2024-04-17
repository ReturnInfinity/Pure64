; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2024 Return Infinity -- see LICENSE.TXT
;
; System Variables
; =============================================================================


message_pure64:		db 10, 'Pure64 ', 0
message_ok:		db 'OK', 10, 0
message_error:		db 'Error', 10, 0

;CONFIG
cfg_smpinit:		db 1		; By default SMP is enabled. Set to 0 to disable.

; Memory locations
InfoMap:		equ 0x0000000000005000
IM_PCIE:		equ 0x0000000000005400		; 16 bytes per entry
IM_IOAPICAddress:	equ 0x0000000000005600		; 16 bytes per entry
IM_IOAPICIntSource:	equ 0x0000000000005700		; 8 bytes per entry
SystemVariables:	equ 0x0000000000005800
VBEModeInfoBlock:	equ 0x0000000000005F00		; 256 bytes

; DQ - Starting at offset 0, increments by 0x8
p_ACPITableAddress:	equ SystemVariables + 0x00
p_LocalAPICAddress:	equ SystemVariables + 0x10
p_Counter_Timer:	equ SystemVariables + 0x18
p_Counter_RTC:		equ SystemVariables + 0x20
p_HPETAddress:		equ SystemVariables + 0x28

; DD - Starting at offset 0x80, increments by 4
p_BSP:			equ SystemVariables + 0x80
p_mem_amount:		equ SystemVariables + 0x84	; in MiB

; DW - Starting at offset 0x100, increments by 2
p_cpu_speed:		equ SystemVariables + 0x100
p_cpu_activated:	equ SystemVariables + 0x102
p_cpu_detected:		equ SystemVariables + 0x104
p_PCIECount:		equ SystemVariables + 0x106

; DB - Starting at offset 0x180, increments by 1
p_IOAPICCount:		equ SystemVariables + 0x180
p_BootMode:		equ SystemVariables + 0x181	; 'U' for UEFI, otherwise BIOS
p_IOAPICIntSourceC:	equ SystemVariables + 0x182
p_x2APIC:		equ SystemVariables + 0x183

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
