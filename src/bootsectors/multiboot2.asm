; Multiboot 2 for BareMetal/Pure64
; http://nongnu.askapache.com/grub/phcoder/multiboot.pdf

[BITS 32]

MAGIC			equ 0xE85250D6
ARCHITECHTURE		equ 0
HEADER_LENGTH		equ multiboot_header_end - multiboot_header
CHECKSUM		equ -(MAGIC + ARCHITECHTURE + HEADER_LENGTH)

_start:				; We need some code before the multiboot header
	xor eax, eax		; Clear eax and ebx in the event
	xor ebx, ebx		; we are not loaded by GRUB.
	jmp multiboot_entry	; Jump over the multiboot header
	align 8			; Multiboot 2 header must be 64-bit aligned

multiboot_header:
	dd MAGIC
	dd ARCHITECHTURE
	dd HEADER_LENGTH
	dd CHECKSUM
multiboot_header_end:

multiboot_entry:
	jmp $
