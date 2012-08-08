; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2012 Return Infinity -- see LICENSE.TXT
;
; BareMetal File System functions
; =============================================================================


; -----------------------------------------------------------------------------
; loadkernel -- loads the kernel file
; IN:	RAX(Memory offset to load to)
loadkernel:
	push rdi
	push rcx

; The kernel is located 16KiB in, and is (up to) 64KiB long -- load it to
; the offset in rbx since that's where it expects to be located
	mov rdi, rax
	mov rax, 32			; start 32 sectors in = 16KiB
	mov rcx, 128			; load 128 sectors = 64KiB
	call readsectors

	pop rcx
	pop rdi
	ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
