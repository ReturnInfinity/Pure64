; =============================================================================
; UEFI loader for Pure64
; Copyright (C) 2008-2022 Return Infinity -- see LICENSE.TXT
;
; Adapted from https://stackoverflow.com/questions/72947069/how-to-write-hello-world-efi-application-in-nasm
; and https://github.com/charlesap/nasm-uefi/blob/master/shoe-x64.asm
; PE https://wiki.osdev.org/PE
; GOP https://wiki.osdev.org/GOP
; Automatic boot: Assemble and save as /EFI/BOOT/BOOTX64.EFI
; Add payload up to 60KB
; dd if=PAYLOAD of=BOOTX64.EFI bs=4096 seek=1 conv=notrunc > /dev/null 2>&1
; =============================================================================

; Set the desired screen resolution values below
Horizontal_Resolution		equ 640
Vertical_Resolution		equ 480

BITS 64
ORG 0x00400000
%define u(x) __utf16__(x)

START:
PE:
HEADER:
DOS_HEADER:							; 128 bytes
DOS_SIGNATURE:			db 'MZ', 0x00, 0x00		; The DOS signature
DOS_HEADERS:			times 60-($-HEADER) db 0	; The DOS Headers
SIGNATURE_POINTER:		dd PE_SIGNATURE - START		; Pointer to the PE Signature
DOS_STUB:			times 64 db 0			; The DOS stub. Fill with zeros
PE_HEADER:							; 24 bytes
PE_SIGNATURE:			db 'PE', 0x00, 0x00		; This is the PE signature. The characters 'PE' followed by 2 null bytes
MACHINE_TYPE:			dw 0x8664			; Targeting the x86-64 machine
NUMBER_OF_SECTIONS:		dw 2				; Number of sections. Indicates size of section table that immediately follows the headers
CREATED_DATE_TIME:		dd 1670698099			; Number of seconds since 1970 since when the file was created
SYMBOL_TABLE_POINTER:		dd 0
NUMBER_OF_SYMBOLS:		dd 0
OHEADER_SIZE:			dw O_HEADER_END - O_HEADER	; Size of the optional header
CHARACTERISTICS:		dw 0x222E			; Attributes of the file

O_HEADER:
MAGIC_NUMBER:			dw 0x020B			; PE32+ (i.e. PE64) magic number
MAJOR_LINKER_VERSION:		db 0
MINOR_LINKER_VERSION:		db 0
SIZE_OF_CODE:			dd CODE_END - CODE		; The size of the code section
INITIALIZED_DATA_SIZE:		dd DATA_END - DATA		; Size of initialized data section
UNINITIALIZED_DATA_SIZE:	dd 0x00				; Size of uninitialized data section
ENTRY_POINT_ADDRESS:		dd EntryPoint - START		; Address of entry point relative to image base when the image is loaded in memory
BASE_OF_CODE_ADDRESS:		dd CODE - START			; Relative address of base of code
IMAGE_BASE:			dq 0x400000			; Where in memory we would prefer the image to be loaded at
SECTION_ALIGNMENT:		dd 0x1000			; Alignment in bytes of sections when they are loaded in memory. Align to page boundary (4kb)
FILE_ALIGNMENT:			dd 0x1000			; Alignment of sections in the file. Also align to 4kb
MAJOR_OS_VERSION:		dw 0
MINOR_OS_VERSION:		dw 0
MAJOR_IMAGE_VERSION:		dw 0
MINOR_IMAGE_VERSION:		dw 0
MAJOR_SUBSYS_VERSION:		dw 0
MINOR_SUBSYS_VERSION:		dw 0
WIN32_VERSION_VALUE:		dd 0				; Reserved, must be 0
IMAGE_SIZE:			dd END - START			; The size in bytes of the image when loaded in memory including all headers
HEADERS_SIZE:			dd HEADER_END - HEADER		; Size of all the headers
CHECKSUM:			dd 0
SUBSYSTEM:			dw 10				; The subsystem. In this case we're making a UEFI application.
DLL_CHARACTERISTICS:		dw 0
STACK_RESERVE_SIZE:		dq 0x200000			; Reserve 2MB for the stack
STACK_COMMIT_SIZE:		dq 0x1000			; Commit 4KB of the stack
HEAP_RESERVE_SIZE:		dq 0x200000			; Reserve 2MB for the heap
HEAP_COMMIT_SIZE:		dq 0x1000			; Commit 4KB of heap
LOADER_FLAGS:			dd 0x00				; Reserved, must be zero
NUMBER_OF_RVA_AND_SIZES:	dd 0x00				; Number of entries in the data directory
O_HEADER_END:

SECTION_HEADERS:
SECTION_CODE:
.name				db ".text", 0x00, 0x00, 0x00
.virtual_size			dd CODE_END - CODE
.virtual_address		dd CODE - START
.size_of_raw_data		dd CODE_END - CODE
.pointer_to_raw_data		dd CODE - START
.pointer_to_relocations		dd 0
.pointer_to_line_numbers	dd 0
.number_of_relocations		dw 0
.number_of_line_numbers		dw 0
.characteristics		dd 0x70000020

SECTION_DATA:
.name				db ".data", 0x00, 0x00, 0x00
.virtual_size			dd DATA_END - DATA
.virtual_address		dd DATA - START
.size_of_raw_data		dd DATA_END - DATA
.pointer_to_raw_data		dd DATA - START
.pointer_to_relocations		dd 0
.pointer_to_line_numbers	dd 0
.number_of_relocations		dw 0
.number_of_line_numbers		dw 0
.characteristics		dd 0xD0000040

HEADER_END:

times 1024-($-PE) db 0

CODE:	; The code begins here with the entry point
EntryPoint:
	; Save the values passed by UEFI
	mov [EFI_IMAGE_HANDLE], rcx
	mov [EFI_SYSTEM_TABLE], rdx
	mov [EFI_RETURN], rsp
	sub rsp, 6*8+8						; Fix stack

	; When calling an EFI function the caller must pass the first 4 integer values in registers
	; via RCX, RDX, R8, and R9
	; 5 and onward are on the stack

	; Save entry addresses
	mov rax, [EFI_SYSTEM_TABLE]
	mov rax, [rax + EFI_SYSTEM_TABLE_BOOTSERVICES]
	mov [BS], rax
	mov rax, [EFI_SYSTEM_TABLE]
	mov rax, [rax + EFI_SYSTEM_TABLE_RUNTIMESERVICES]
	mov [RTS], rax
	mov rax, [EFI_SYSTEM_TABLE]
	mov rax, [rax + EFI_SYSTEM_TABLE_CONFIGURATION_TABLE]
	mov [CONFIG], rax
	mov rax, [EFI_SYSTEM_TABLE]
	mov rax, [rax + EFI_SYSTEM_TABLE_CONOUT]
	mov [OUTPUT], rax

	; Set screen colour attributes
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	mov rdx, 0x7F						; IN UINTN Attribute Light grey background, white foreground
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_SET_ATTRIBUTE]

	; Clear screen
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_CLEAR_SCREEN]

	; Find the address of the ACPI data from the UEFI configuration table
	mov rax, [EFI_SYSTEM_TABLE]
	mov rcx, [rax + EFI_SYSTEM_TABLE_NUMBEROFENTRIES]
	shl rcx, 3						; Quick multiply by 4
	mov rsi, [CONFIG]
nextentry:
	dec rcx
	cmp rcx, 0
	je error						; Bail out as no ACPI data was detected
	mov rbx, [ACPI_TABLE_GUID]				; First 64 bits of the ACPI GUID
	lodsq
	cmp rax, rbx						; Compare the table data to the expected GUID data
	jne nextentry
	mov rbx, [ACPI_TABLE_GUID+8]				; Second 64 bits of the ACPI GUID
	lodsq
	cmp rax, rbx						; Compare the table data to the expected GUID data
	jne nextentry
	lodsq							; Load the address of the ACPI table
	mov [ACPI], rax						; Save the address

	; Find the interface to GRAPHICS_OUTPUT_PROTOCOL via its GUID
	mov rcx, EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID		; IN EFI_GUID *Protocol
	mov rdx, 0						; IN VOID *Registration OPTIONAL
	mov r8, VIDEO						; OUT VOID **Interface
	mov rax, [BS]
	mov rax, [rax + EFI_BOOT_SERVICES_LOCATEPROTOCOL]
	call rax
	cmp rax, EFI_SUCCESS
	jne error

	; Parse the graphics information
	; Mode Structure
	; 0  UINT32 - MaxMode
	; 4  UINT32 - Mode
	; 8  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION - *Info;
	; 16 UINTN - SizeOfInfo
	; 24 EFI_PHYSICAL_ADDRESS - FrameBufferBase
	; 32 UINTN - FrameBufferSize
	mov rbx, [VIDEO]
	add rbx, EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE
	mov rbx, [rbx]						; RAX holds the address of the Mode structure
	mov eax, [rbx]						; RAX holds UINT32 MaxMode
	mov [vid_max], rax
	jmp vid_query
	
next_video_mode:
	mov rax, [vid_current]
	add rax, 1						; Increment the mode # to check
	mov [vid_current], rax
	mov rbx, [vid_max]
	cmp rax, rbx
	je skip_set_video					; If we have reached the max then bail out
	
vid_query:
	; Query a video mode
	mov rcx, [VIDEO]					; IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This
	mov rdx, [vid_current]					; IN UINT32 ModeNumber
	lea r8, [vid_size]					; OUT UINTN *SizeOfInfo
	lea r9, [vid_info]					; OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info
	call [rcx + EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE]

	; Check mode settings
	mov rsi, [vid_info]
	lodsd							; UINT32 - Version
	lodsd							; UINT32 - HorizontalResolution
	cmp eax, Horizontal_Resolution
	jne next_video_mode
	lodsd							; UINT32 - VerticalResolution
	cmp eax, Vertical_Resolution
	jne next_video_mode
	lodsd							; EFI_GRAPHICS_PIXEL_FORMAT - PixelFormat (UINT32)
	bt eax, 0						; Bit 0 is set for 32-bit colour mode
	jnc next_video_mode

	; Set the video mode
	mov rcx, [VIDEO]					; IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This
	mov rdx, [vid_current]					; IN UINT32 ModeNumber
	call [rcx + EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE]

skip_set_video:
	; Gather video mode details
	mov rbx, [VIDEO]
	add rbx, EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE
	mov rbx, [rbx]						; RBX holds the address of the Mode structure
	mov rax, [rbx+24]					; RAX holds the FB base
	mov [FB], rax						; Save the FB base
	mov rax, [rbx+32]					; RAX holds the FB size
	mov [FBS], rax						; Save the FB size
	mov rbx, [rbx+8]					; RBX holds the address of the EFI_GRAPHICS_OUTPUT_MODE_INFORMATION Structure
	; EFI_GRAPHICS_OUTPUT_MODE_INFORMATION Structure
	; 0  UINT32 - Version
	; 4  UINT32 - HorizontalResolution
	; 8  UINT32 - VerticalResolution
	; 12 EFI_GRAPHICS_PIXEL_FORMAT - PixelFormat (UINT32)
	; 16 EFI_PIXEL_BITMASK - PixelInformation (4 UINT32 - RedMask, GreenMask, BlueMask, ReservedMask)
	; 32 UINT32 - PixelsPerScanLine (Should be the same as HorizontalResolution)
	mov eax, [rbx+4]					; RAX holds the Horizontal Resolution
	mov [HR], rax						; Save the Horizontal Resolution
	mov eax, [rbx+8]					; RAX holds the Vertical Resolution
	mov [VR], rax						; Save the Vertical Resolution

	; Copy Pure64 to the correct memory address
	mov rsi, PAYLOAD
	mov rdi, 0x8000
	mov rcx, 61440						; Copy 60KB
	rep movsb
	mov ax, [0x8006]
	cmp ax, 0x3436						; Match against the Pure64 binary
	jne sig_fail

	; Signal to Pure64 that it was booted via UEFI
	mov al, 'U'
	mov [0x8005], al

	; Save video values to the area of memory where Pure64 expects them
	mov rdi, 0x00005C00 + 40				; VBEModeInfoBlock.PhysBasePtr
	mov rax, [FB]
	stosd
	mov rdi, 0x00005C00 + 18				; VBEModeInfoBlock.XResolution & .YResolution
	mov rax, [HR]
	stosw
	mov rax, [VR]
	stosw
	mov rdi, 0x00005C00 + 25				; VBEModeInfoBlock.BitsPerPixel
	mov rax, 32
	stosb

	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	lea rdx, [msg_OK]					; IN CHAR16 *String
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_OUTPUTSTRING]

	; Get Memory Map
get_memmap:
	lea rcx, [memmapsize]					; IN OUT UINTN *MemoryMapSize
	lea rdx, 0x6000						; OUT EFI_MEMORY_DESCRIPTOR *MemoryMap
	lea r8, [memmapkey]					; OUT UINTN *MapKey
	lea r9, [memmapdescsize]				; OUT UINTN *DescriptorSize
	lea r10, [memmapdescver]				; OUT UINT32 *DescriptorVersion
	push r10
	sub rsp, 32
	mov rax, [BS]
	call [rax + EFI_BOOT_SERVICES_GETMEMORYMAP]
	add rsp, 32
	pop r10
	cmp al, 5						; EFI_BUFFER_TOO_SMALL
	je get_memmap						; Attempt again as the memmapsize was updated by EFI
	cmp rax, EFI_SUCCESS
	jne error
	; Output at 0x6000 is as follows:
	; 0  UINT32 - Type
	; 8  EFI_PHYSICAL_ADDRESS - PhysicalStart
	; 16 EFI_VIRTUAL_ADDRESS - VirtualStart
	; 24 UINT64 - NumberOfPages
	; 32 UINT64 - Attribute
	; 40 UINT64 - Blank

	; Exit Boot services as EFI is no longer needed
	mov rcx, [EFI_IMAGE_HANDLE]				; IN EFI_HANDLE ImageHandle
	mov rdx, [memmapkey]					; IN UINTN MapKey
	mov rax, [BS]
	call [rax + EFI_BOOT_SERVICES_EXITBOOTSERVICES]
	cmp rax, EFI_SUCCESS
	jne exitfailure

	; Stop interrupts
	cli

	; Build a 32-bit memory table for 4GiB of identity mapped memory
	mov rdi, 0x200000
	mov rax, 0x00000083
	mov rcx, 1024
nextpage:
	stosd
	add rax, 0x400000
	dec rcx
	cmp rcx, 0
	jne nextpage	

	; Load the custom GDT
	lgdt [gdtr]

	; Switch to compatibility mode
	mov rax, SYS32_CODE_SEL					; Compatibility mode
	push rax
	lea rax, [compatmode]
	push rax
	retfq

BITS 32
compatmode:
	; Set the segment registers
	mov eax, SYS32_DATA_SEL
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Deactivate IA-32e mode by clearing CR0.PG
	mov eax, cr0
	btc eax, 31						; Clear PG (Bit 31)
	mov cr0, eax

	; Load CR3
	mov eax, 0x00200000					; Address of memory map
	mov cr3, eax

	; Disable IA-32e mode by setting IA32_EFER.LME = 0
	mov ecx, 0xC0000080					; EFER MSR number
	rdmsr							; Read EFER
	and eax, 0xFFFFFEFF 					; Clear LME (Bit 8)
	wrmsr							; Write EFER

	mov eax, 0x00000010					; Set PSE (Bit 4)
	mov cr4, eax

	; Enable legacy paged-protected mode by setting CR0.PG
	mov eax, 0x00000001					; Set PM (Bit 0)
	mov cr0, eax

	jmp SYS32_CODE_SEL:0x8000				; 32-bit jump to set CS

BITS 64
exitfailure:
	mov rdi, [FB]
	mov eax, 0x00FF0000					; Red
	mov rcx, [FBS]
	shr rcx, 2						; Quick divide by 4 (32-bit colour)
	rep stosd
	jmp halt
error:
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	lea rdx, [msg_error]					; IN CHAR16 *String
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_OUTPUTSTRING]
	jmp halt
sig_fail:
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	lea rdx, [msg_SigFail]					; IN CHAR16 *String
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_OUTPUTSTRING]
halt:
	hlt
	jmp halt


align 2048
CODE_END:

; Data begins here
DATA:
EFI_IMAGE_HANDLE:	dq 0	; EFI gives this in RCX
EFI_SYSTEM_TABLE:	dq 0	; And this in RDX
EFI_RETURN:		dq 0	; And this in RSP
BS:			dq 0	; Boot services
RTS:			dq 0	; Runtime services
CONFIG:			dq 0	; Config Table address
ACPI:			dq 0	; ACPI table address
OUTPUT:			dq 0	; Output services
VIDEO:			dq 0	; Video services
FB:			dq 0	; Frame buffer base address
FBS:			dq 0	; Frame buffer size
HR:			dq 0	; Horizontal Resolution
VR:			dq 0	; Vertical Resolution
memmapsize:		dq 8192
memmapkey:		dq 0
memmapdescsize:		dq 0
memmapdescver:		dq 0
vid_current:		dq 0
vid_max:		dq 0
vid_size:		dq 0
vid_info:		dq 0

ACPI_TABLE_GUID:
dd 0xeb9d2d30
dw 0x2d88, 0x11d3
db 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d

EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID:
dd 0x9042a9de
dw 0x23dc, 0x4a38
db 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a

hextable: 		db '0123456789ABCDEF'
msg_error:		dw u('Error'), 0
msg_SigFail:		dw u('Bad Sig!'), 0
msg_OK:			dw u('OK'), 0

align 16
gdtr:					; Global Descriptors Table Register
dw gdt_end - gdt - 1			; limit of GDT (size minus one)
dq gdt					; linear address of GDT

align 16
gdt:
SYS64_NULL_SEL equ $-gdt		; Null Segment
dq 0x0000000000000000
SYS32_CODE_SEL equ $-gdt		; 32-bit code descriptor
dq 0x00CF9A000000FFFF			; 55 Granularity 4KiB, 54 Size 32bit, 47 Present, 44 Code/Data, 43 Executable, 41 Readable
SYS32_DATA_SEL equ $-gdt		; 32-bit data descriptor
dq 0x00CF92000000FFFF			; 55 Granularity 4KiB, 54 Size 32bit, 47 Present, 44 Code/Data, 41 Writeable
SYS64_CODE_SEL equ $-gdt		; 64-bit code segment, read/execute, nonconforming
dq 0x00209A0000000000			; 53 Long mode code, 47 Present, 44 Code/Data, 43 Executable, 41 Readable
SYS64_DATA_SEL equ $-gdt		; 64-bit data segment, read/write, expand down
dq 0x0000920000000000			; 47 Present, 44 Code/Data, 41 Writable
gdt_end:

align 4096
PAYLOAD:

align 65536				; Pad out to 64K
DATA_END:
END:

; Define the needed EFI constants and offsets here.
EFI_SUCCESS						equ 0
EFI_LOAD_ERROR						equ 1
EFI_INVALID_PARAMETER					equ 2
EFI_UNSUPPORTED						equ 3
EFI_BAD_BUFFER_SIZE					equ 4
EFI_BUFFER_TOO_SMALL					equ 5
EFI_NOT_READY						equ 6
EFI_DEVICE_ERROR					equ 7
EFI_WRITE_PROTECTED					equ 8
EFI_OUT_OF_RESOURCES					equ 9
EFI_VOLUME_CORRUPTED					equ 10
EFI_VOLUME_FULL						equ 11
EFI_NO_MEDIA						equ 12
EFI_MEDIA_CHANGED					equ 13
EFI_NOT_FOUND						equ 14

EFI_SYSTEM_TABLE_CONOUT                         	equ 64
EFI_SYSTEM_TABLE_RUNTIMESERVICES			equ 88
EFI_SYSTEM_TABLE_BOOTSERVICES				equ 96
EFI_SYSTEM_TABLE_NUMBEROFENTRIES			equ 104
EFI_SYSTEM_TABLE_CONFIGURATION_TABLE			equ 112

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_RESET			equ 0
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_OUTPUTSTRING		equ 8
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_TEST_STRING		equ 16
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_QUERY_MODE		equ 24
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_SET_MODE		equ 32
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_SET_ATTRIBUTE		equ 40
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_CLEAR_SCREEN		equ 48
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_SET_CURSOR_POSITION	equ 56
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_ENABLE_CURSOR		equ 64
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_MODE			equ 70

EFI_BOOT_SERVICES_GETMEMORYMAP				equ 56
EFI_BOOT_SERVICES_LOCATEHANDLE				equ 176
EFI_BOOT_SERVICES_LOADIMAGE				equ 200
EFI_BOOT_SERVICES_EXIT					equ 216
EFI_BOOT_SERVICES_EXITBOOTSERVICES			equ 232
EFI_BOOT_SERVICES_STALL					equ 248
EFI_BOOT_SERVICES_SETWATCHDOGTIMER			equ 256
EFI_BOOT_SERVICES_LOCATEPROTOCOL			equ 320

EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE			equ 0
EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE			equ 8
EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT			equ 16
EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE			equ 24

EFI_RUNTIME_SERVICES_RESETSYSTEM			equ 104

; EOF
