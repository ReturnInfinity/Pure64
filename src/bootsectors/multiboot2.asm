; Multiboot 2 for BareMetal/Pure64
; http://nongnu.askapache.com/grub/phcoder/multiboot.pdf

[BITS 32]

MAGIC			equ 0xE85250D6
ARCHITECHTURE		equ 0
HEADER_LENGTH		equ multiboot_header_end - multiboot_header
CHECKSUM		equ -(MAGIC + ARCHITECHTURE + HEADER_LENGTH)

multiboot_header:
	dd MAGIC
	dd ARCHITECHTURE
	dd HEADER_LENGTH
	dd CHECKSUM
multiboot_header_end:
