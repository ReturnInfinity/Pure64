; =============================================================================
; Pure64 Multiboot 2 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
;
; http://nongnu.askapache.com/grub/phcoder/multiboot.pdf
; =============================================================================


[BITS 32]
[global _start]
[ORG 0x100000]		;If using '-f bin' we need to specify the
			;origin point for our code with ORG directive
			;multiboot loaders load us at physical
			;address 0x100000

MAGIC			equ 0xE85250D6
ARCHITECHTURE		equ 0
HEADER_LENGTH		equ multiboot_header_end - multiboot_header_start
CHECKSUM		equ 0x100000000 - (MAGIC + ARCHITECHTURE + HEADER_LENGTH)

_start:				; We need some code before the multiboot header
	xor eax, eax		; Clear eax and ebx in the event
	xor ebx, ebx		; we are not loaded by GRUB.
	jmp multiboot_entry	; Jump over the multiboot header
	align 8			; Multiboot 2 header must be 64-bit aligned

multiboot_header_start:
	dd MAGIC
	dd ARCHITECHTURE
	dd HEADER_LENGTH
	dd CHECKSUM
entry_address_tag_start:
	dw 3
	dw 0
	dd entry_address_tag_end - entry_address_tag_start
	dq multiboot_entry
entry_address_tag_end:
framebuffer_tag_start:
        dw 5
        dw 0
        dd framebuffer_tag_end - framebuffer_tag_start
        dd 800
        dd 600
        dd 32
framebuffer_tag_end:
	dw 0			; End type
	dw 0
	dd 8
multiboot_header_end:

multiboot_entry:
	cmp eax, 0x36D76289	; Magic value
	jne error
error:
	jmp $
