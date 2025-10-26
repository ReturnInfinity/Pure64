; =============================================================================
; Pure64 MBR -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; This Master Boot Record will load Pure64 from a pre-defined location on the
; hard drive without making use of the file system.
;
; In this code we are expecting a BMFS-formatted drive. With BMFS the Pure64
; binary is required to start at sector 16 (8192 bytes from the start). A small
; check is made to make sure Pure64 was loaded by comparing a signature.
; =============================================================================

; Default location of the second stage boot loader. This loads
; 32 KiB from sector 16 into memory at 0x8000
%define DAP_SECTORS 64
%define DAP_STARTSECTOR 16
%define DAP_ADDRESS 0x8000
%define DAP_SEGMENT 0x0000

BITS 16
org 0x7C00

entry:
	jmp bootcode			; Jump past the BPB data
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

; EBPB (Extended Boot Record)
dd 0		; Sectors per FAT
dw 0		; Flags
dw 0		; FAT version number
dd 0		; The cluster number of the root directory
dw 0		; The sector number of the FSInfo structure
dw 0		; The sector number of the backup boot sector
dq 0		; Reserved
dd 0		; Reserved
db 0		; Drive number
db 0		; Flags in Windows NT
db 0		; Signature
dd 0		; Volume ID 'Serial' number
times 11 db 0	; Volume label string
dq 0		; System identifier string. Always "FAT32   "

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

	; Configure first serial port
	mov dx, 0			; Port
	mov ax, 0x00E3			; Init port, 9600bps 8N1
	int 0x14

	; Output message to serial
	mov si, msg_Load
	call output_serial

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

	; Video modes aren't supported in this boot sector
	; Clear the video detail memory for Pure64
	mov edi, 0x5F00
	xor eax, eax
	mov ecx, 64
	rep stosd

	; Read the 2nd stage boot loader into memory.
	mov ah, 0x42			; Extended Read
	mov dl, [DriveNumber]		; http://www.ctyme.com/intr/rb-0708.htm
	mov si, DAP
	int 0x13
	jc halt

	; Check signature
;	mov eax, [0x8000]
;	cmp eax, 0x00017EE9		; Match against the Pure64 binary
;	jne halt

	; Output message to serial
	mov si, msg_Ok
	call output_serial

	mov bl, 'B'			; 'B' as we booted via BIOS

	; At this point we are done with real mode and BIOS interrupts. Jump to 32-bit mode.
	cli				; No more interrupts
	lgdt [cs:GDTR32]		; Load GDT register
	mov eax, cr0
	or al, 0x01			; Set protected mode bit
	mov cr0, eax
	jmp 8:0x8000			; Jump to 32-bit protected mode

;magic_fail:
;	mov si, msg_Error
;	call output_serial
halt:
	hlt
	jmp halt
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; Output a string via serial
; IN:	SI - Address of start of string
output_serial:			; Output string via serial
	pusha
output_serial_next_char:
	mov dx, 0			; First serial port
	mov ah, 0x01			; SERIAL - WRITE CHARACTER TO PORT
	lodsb				; Get char from string
	cmp al, 0
	je output_serial_done		; If char is zero, end of string
	int 0x14			; Otherwise, output it
	jmp short output_serial_next_char
output_serial_done:
	popa
	ret
;------------------------------------------------------------------------------

msg_Load db "MBR ", 0
msg_Ok db "OK", 0
msg_Error db "Error!", 0

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

DAP:
db 0x10
db 0x00
dw DAP_SECTORS
dw DAP_ADDRESS
dw DAP_SEGMENT
dq DAP_STARTSECTOR

DriveNumber db 0x00

times 446-$+$$ db 0

; Partition entries (4x 16-bytes)

times 510-$+$$ db 0

dw 0xAA55				; Boot signature


; EOF
