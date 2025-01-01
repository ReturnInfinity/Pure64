; =============================================================================
; UEFI loader for Pure64
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
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
Horizontal_Resolution		equ 1024
Vertical_Resolution		equ 768

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

align 16

CODE:	; The code begins here with the entry point
EntryPoint:
	; Save the values passed by UEFI
	mov [EFI_IMAGE_HANDLE], rcx
	mov [EFI_SYSTEM_TABLE], rdx
	mov [EFI_RETURN], rsp
	sub rsp, 6*8+8						; Fix stack

	; When calling an EFI function the caller must pass the first 4 integer values in registers
	; via RCX, RDX, R8, and R9
	; 5 and onward are on the stack after the 32 byte shadow space

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
	mov rdx, 0x07						; IN UINTN Attribute - Black background, grey foreground
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_SET_ATTRIBUTE]

	; Clear screen (This also sets the cursor position to 0,0)
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_CLEAR_SCREEN]

	; Output 'UEFI '
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	lea rdx, [msg_uefi]					; IN CHAR16 *String
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_OUTPUTSTRING]

	; Find the address of the ACPI data from the UEFI configuration table
	mov rax, [EFI_SYSTEM_TABLE]
	mov rcx, [rax + EFI_SYSTEM_TABLE_NUMBEROFENTRIES]
	shl rcx, 3						; Quick multiply by 4
	mov rsi, [CONFIG]
nextentry:
	dec rcx
	cmp rcx, 0
	je error						; Bail out as no ACPI data was detected
	mov rdx, [ACPI_TABLE_GUID]				; First 64 bits of the ACPI GUID
	lodsq
	cmp rax, rdx						; Compare the table data to the expected GUID data
	jne nextentry
	mov rdx, [ACPI_TABLE_GUID+8]				; Second 64 bits of the ACPI GUID
	lodsq
	cmp rax, rdx						; Compare the table data to the expected GUID data
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

	; Parse the current graphics information
	; EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE Structure
	; 0  UINT32 - MaxMode
	; 4  UINT32 - Mode
	; 8  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION - *Info;
	; 16 UINTN - SizeOfInfo
	; 24 EFI_PHYSICAL_ADDRESS - FrameBufferBase
	; 32 UINTN - FrameBufferSize
	mov rax, [VIDEO]
	add rcx, EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE
	mov rcx, [rcx]						; RCX holds the address of the Mode structure
	mov eax, [rcx]						; EAX holds UINT32 MaxMode
	mov [vid_max], rax					; The maximum video modes we can check
	jmp vid_query

next_video_mode:
	mov rax, [vid_index]
	add rax, 1						; Increment the mode # to check
	mov [vid_index], rax
	mov rdx, [vid_max]
	cmp rax, rdx
	je skip_set_video					; If we have reached the max then bail out

vid_query:
	; Query a video mode
	mov rcx, [VIDEO]					; IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This
	mov rdx, [vid_index]					; IN UINT32 ModeNumber
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
	mov rdx, [vid_index]					; IN UINT32 ModeNumber
	call [rcx + EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE]
	cmp rax, EFI_SUCCESS
	jne next_video_mode
skip_set_video:

	; Gather video mode details
	mov rcx, [VIDEO]
	add rcx, EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE
	mov rcx, [rcx]						; RCX holds the address of the Mode structure
	mov rax, [rcx+24]					; RAX holds the FB base
	mov [FB], rax						; Save the FB base
	mov rax, [rcx+32]					; RAX holds the FB size
	mov [FBS], rax						; Save the FB size
	mov rcx, [rcx+8]					; RCX holds the address of the EFI_GRAPHICS_OUTPUT_MODE_INFORMATION Structure
	; EFI_GRAPHICS_OUTPUT_MODE_INFORMATION Structure
	; 0  UINT32 - Version
	; 4  UINT32 - HorizontalResolution
	; 8  UINT32 - VerticalResolution
	; 12 EFI_GRAPHICS_PIXEL_FORMAT - PixelFormat (UINT32)
	; 16 EFI_PIXEL_BITMASK - PixelInformation (4 UINT32 - RedMask, GreenMask, BlueMask, ReservedMask)
	; 32 UINT32 - PixelsPerScanLine - Defines the number of pixel elements per video memory line. Scan lines may be padded for memory alignment.
	mov eax, [rcx+4]					; RAX holds the Horizontal Resolution
	mov [HR], rax						; Save the Horizontal Resolution
	mov eax, [rcx+8]					; RAX holds the Vertical Resolution
	mov [VR], rax						; Save the Vertical Resolution
	mov eax, [rcx+32]					; RAX holds the PixelsPerScanLine
	mov [PPSL], rax						; Save the PixelsPerScanLine

	; Copy Pure64 to the correct memory address
	mov rsi, PAYLOAD
	mov rdi, 0x8000
	mov rcx, 61440						; Copy 60KB
	rep movsb
	mov ax, [0x8006]
	cmp ax, 0x3436						; Match against the Pure64 binary
	jne sig_fail

; Debug
;	mov rbx, [FB]						; Display the framebuffer address
;	call printhex

get_memmap:
	; Output 'OK' as we are about to leave UEFI
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	lea rdx, [msg_OK]					; IN CHAR16 *String
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_OUTPUTSTRING]

	; Get Memory Map from UEFI and save it [memmap]
	lea rcx, [memmapsize]					; IN OUT UINTN *MemoryMapSize
	mov rdx, [memmap]					; OUT EFI_MEMORY_DESCRIPTOR *MemoryMap
	lea r8, [memmapkey]					; OUT UINTN *MapKey
	lea r9, [memmapdescsize]				; OUT UINTN *DescriptorSize
	lea r10, [memmapdescver]				; OUT UINT32 *DescriptorVersion
	mov [rsp+32], r10
	mov rax, [BS]
	call [rax + EFI_BOOT_SERVICES_GETMEMORYMAP]
	cmp al, EFI_BUFFER_TOO_SMALL
	je get_memmap						; Attempt again as the memmapsize was updated by EFI
	cmp rax, EFI_SUCCESS
	jne exitfailure
	; Each 48-byte record has the following format:
	; 0  UINT32 - Type
	; 4  UNIT32 - Padding
	; 8  EFI_PHYSICAL_ADDRESS (UINT64) - PhysicalStart
	; 16 EFI_VIRTUAL_ADDRESS (UINT64) - VirtualStart
	; 24 UINT64 - NumberOfPages - This is a number of 4K pages (must be a non-zero value)
	; 32 UINT64 - Attribute
	; 40 UINT64 - Blank
	;
	; UEFI Type:
	; 0x0 = EfiReservedMemoryType - Not usable
	; 0x1 = EfiLoaderCode - Usable after ExitBootSerivces
	; 0x2 = EfiLoaderData - Usable after ExitBootSerivces
	; 0x3 = EfiBootServicesCode - Usable after ExitBootSerivces
	; 0x4 = EfiBootServicesData - Usable after ExitBootSerivces
	; 0x5 = EfiRuntimeServicesCode
	; 0x6 = EfiRuntimeServicesData
	; 0x7 = EfiConventionalMemory - Usable after ExitBootSerivces
	; 0x8 = EfiUnusableMemory - Not usable - errors detected
	; 0x9 = EfiACPIReclaimMemory - Usable after ACPI is enabled
	; 0xA = EfiACPIMemoryNVS
	; 0xB = EfiMemoryMappedIO
	; 0xC = EfiMemoryMappedIOPortSpace
	; 0xD = EfiPalCode
	; 0xE = EfiPersistentMemory
	; 0xF = EfiMaxMemoryTyp

	; Exit Boot services as UEFI is no longer needed
	mov rcx, [EFI_IMAGE_HANDLE]				; IN EFI_HANDLE ImageHandle
	mov rdx, [memmapkey]					; IN UINTN MapKey
	mov rax, [BS]
	call [rax + EFI_BOOT_SERVICES_EXITBOOTSERVICES]
	cmp rax, EFI_SUCCESS
	jne get_memmap						; If it failed, get the memory map and try to exit again

	; Stop interrupts
	cli

	; Save UEFI values to the area of memory where Pure64 expects them
	mov rdi, 0x00005F00
	mov rax, [FB]
	stosq							; 64-bit Frame Buffer Base
	mov rax, [FBS]
	stosq							; 64-bit Frame Buffer Size in bytes
	mov rax, [HR]
	stosw							; 16-bit Screen X
	mov rax, [VR]
	stosw							; 16-bit Screen Y
	mov rax, [PPSL]
	stosw							; 16-bit PixelsPerScanLine
	mov rax, 32						; TODO - Verify this
	stosw							; 16-bit BitsPerPixel
	mov rax, [memmap]
	stosq							; Memory Map Base
	mov rax, [memmapsize]
	stosq							; Size of Memory Map in bytes
	mov rax, [memmapkey]
	stosq							; The key used to exit Boot Services
	mov rax, [memmapdescsize]
	stosq							; EFI_MEMORY_DESCRIPTOR size in bytes
	mov rax, [memmapdescver]
	stosq							; EFI_MEMORY_DESCRIPTOR version
	mov rax, [ACPI]
	stosq							; ACPI Table Address

	; Set screen to black before jumping to Pure64
	mov rdi, [FB]
	mov eax, 0x00000000					; 0x00RRGGBB
	mov rcx, [FBS]
	shr rcx, 2						; Quick divide by 4 (32-bit colour)
	rep stosd

	; Clear registers
	xor eax, eax
	xor ecx, ecx
	xor edx, edx
	xor ebx, ebx
	mov rsp, 0x8000
	xor ebp, ebp
	xor esi, esi
	xor edi, edi
	xor r8, r8
	xor r9, r9
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	xor r14, r14
	xor r15, r15

	mov bl, 'U'
	jmp 0x8000

exitfailure:
	; Set screen to red on exit failure
	mov rdi, [FB]
	mov eax, 0x00FF0000					; 0x00RRGGBB
	mov rcx, [FBS]
	shr rcx, 2						; Quick divide by 4 (32-bit colour)
	rep stosd
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

; -----------------------------------------------------------------------------
; printhex - Display a 64-bit value in hex
; IN: RBX = Value
printhex:			 
	mov rbp, 16						; Counter
	push rax
	push rcx
	push rdx						; 3 pushes also align stack on 16 byte boundary
								; (8+3*8)=32, 32 evenly divisible by 16
	sub rsp, 32						; Allocate 32 bytes of shadow space
printhex_loop:
	rol rbx, 4
	mov rax, rbx
	and rax, 0Fh
	lea rcx, [Hex]
	mov rax, [rax + rcx]
	mov byte [Num], al
	lea rdx, [Num]
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_OUTPUTSTRING]
	dec rbp
	jnz printhex_loop
	lea rdx, [newline]
	mov rcx, [OUTPUT]					; IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_OUTPUTSTRING]

	add rsp, 32
	pop rdx
	pop rcx
	pop rax
	ret
; -----------------------------------------------------------------------------


times 2048-$+$$ db 0
CODE_END:

; Data begins here
DATA:
EFI_IMAGE_HANDLE:	dq 0					; EFI gives this in RCX
EFI_SYSTEM_TABLE:	dq 0					; And this in RDX
EFI_RETURN:		dq 0					; And this in RSP
BS:			dq 0					; Boot services
RTS:			dq 0					; Runtime services
CONFIG:			dq 0					; Config Table address
ACPI:			dq 0					; ACPI table address
OUTPUT:			dq 0					; Output services
VIDEO:			dq 0					; Video services
FB:			dq 0					; Frame buffer base address
FBS:			dq 0					; Frame buffer size
HR:			dq 0					; Horizontal Resolution
VR:			dq 0					; Vertical Resolution
PPSL:			dq 0					; PixelsPerScanLine
BPP:			dq 0					; BitsPerPixel
memmap:			dq 0x220000				; Store the Memory Map from UEFI here
memmapsize:		dq 32768				; Max size we are expecting in bytes
memmapkey:		dq 0
memmapdescsize:		dq 0
memmapdescver:		dq 0
vid_orig:		dq 0
vid_index:		dq 0
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

msg_uefi:		dw u('UEFI '), 0
msg_OK:			dw u('OK '), 0
msg_error:		dw u('Error'), 0
msg_SigFail:		dw u('Bad Sig!'), 0
Hex:			db '0123456789ABCDEF'
Num:			dw 0, 0
newline:		dw 13, 10, 0


align 4096							; Pad out to 4KiB for UEFI loader
PAYLOAD:

align 65536							; Pad out to 64KiB for payload (Pure64, Kernel, etc)
RAMDISK:

times 65535+1048576-$+$$ db 0					; 1MiB of padding for RAM disk image
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

EFI_SYSTEM_TABLE_CONOUT					equ 64
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
