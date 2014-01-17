; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2013 Return Infinity -- see LICENSE.TXT
;
; System Calls
; =================================================================


; -----------------------------------------------------------------------------
; os_move_cursor -- Moves the virtual cursor in text mode
;  IN:	AH, AL = row, column
; OUT:	Nothing. All registers preserved
os_move_cursor:
	mov r8, rbx
	mov r9, rax
	movzx eax, ax
	mov [screen_cursor_y], ax
;	mov [screen_cursor_y], al
	movzx ebx, al
	
	; Calculate the new offset
	shr eax, 8			; only keep the low 8 bits
	imul ax, 160			; EAX = (80*AL+BL)*2=160*AL+2*BL
	lea eax, [eax+ebx*2]

	add eax, 0xB8000
	mov [screen_cursor_offset], rax
	
	mov rax, r9
	mov rbx, r8
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; os_print_newline -- Reset cursor to start of next line and scroll if needed
;  IN:	Nothing
; OUT:	Nothing, all registers perserved
os_print_newline:
	mov r8, rax
	mov r9, rbx
;	mov ah, 0				; Set the cursor x value to 0
	movzx eax, byte [screen_cursor_y]	; Grab the cursor y value
	xor ebx, ebx
	cmp al, 24				; Compare to see if we are on the last line
	lea eax, [eax+1]
	cmove eax, ebx
	mov ebx, 0xB8000
	mov [screen_cursor_y], ax
	lea eax, [ebx+eax*2]
	mov [screen_cursor_offset], rax
	mov rax, r8
	mov rbx, r9
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; os_print_string -- Displays text
;  IN:	RSI = message location (zero-terminated string)
; OUT:	Nothing, all registers perserved
os_print_string:
	mov r10, rdi
	mov rdi, [screen_cursor_offset]
	mov r15, rsi
	mov r14, rax
	mov r13, rbx
	mov r12, rcx
	mov r11, rdx
os_print_string_nextreg:
	mov r8, [rsi]
	xor r9, r9
	xor ecx, ecx
	mov cl, -8
os_print_string_nextchar_reg:
	movzx eax, r8b				; Get char from string and store in AL
	shl r8, 8
	cmp al, 13			; Check if there was a newline character in the string
	jne os_print_string_char	; If not newline, skip to the standard part
	movzx edx, word [screen_cursor_y]
	movzx ebx, dl
	shr  edx, 8
	cmp bl, 24
	lea ebx, [ebx+1]
	cmove rbx, r9			; if ebx<=24 increment it, otherwise set it to 0
	imul dx, 160
	lea edi, [edx+ebx*2]
	add edi, 0xB8000
os_print_string_char:	
	mov [rdi], al
	add rdi, 2
	inc ecx
	test eax, eax
	cmovz rcx, r9
	test ecx, ecx
	jnz os_print_string_nextchar_reg
	add rsi, 8
	test eax,eax
	jnz os_print_string_nextreg

os_print_string_done:
	mov [screen_cursor_offset], rdi
	mov rdi, r10
	mov rdx, r11
	mov rcx, r12
	mov rbx, r13
	mov rax, r14
	mov rsi, r15
	ret


; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; os_print_char -- Displays a char
;  IN:	AL = char to display
; OUT:	Nothing. All registers preserved
os_print_char:
	mov r8, rdi

	mov rdi, [screen_cursor_offset]
	mov [rdi], al
	add rdi, 2
	mov [screen_cursor_offset], rdi	; Add 2 (1 byte for char and 1 byte for attribute)

	mov rdi, r8
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; os_print_char_hex -- Displays a char in hex mode
;  IN:	AL = char to display
; OUT:	Nothing. All registers preserved
os_print_char_hex:
	mov r8, rbx
	mov r9, rax
	mov r10, rcx
	mov r11, rdi

	shr eax, 4			; we want to work on the high part so shift right by 4 bits
	mov rdi, [screen_cursor_offset]
	mov rbx, hextable
	movzx ecx, al			; save rax for the next part
	movzx eax, byte [rbx+rax]
	and cl, 0x0f
	movzx ecx, byte [rbx+rcx]
	shl ecx, 8			; 2 bytes for char
	or eax, ecx
	mov [rdi], eax
	add rdi, 4
	mov [screen_cursor_offset], rdi

	mov rax, r9
	mov rbx, r8
	mov rcx, r10
	mov rdi, r11
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; os_debug_dump_(rax|eax|ax|al) -- Dump content of RAX, EAX, AX, or AL to the screen in hex format
;  IN:	RAX = content to dump
; OUT:	Nothing, all registers preserved
os_debug_dump_rax:
	ror rax, 56
	call os_print_char_hex
	rol rax, 8
	call os_print_char_hex
	rol rax, 8
	call os_print_char_hex
	rol rax, 8
	call os_print_char_hex
	rol rax, 32
os_debug_dump_eax:
	ror rax, 24
	call os_print_char_hex
	rol rax, 8
	call os_print_char_hex
	rol rax, 16
os_debug_dump_ax:
	ror rax, 8
	call os_print_char_hex
	rol rax, 8
os_debug_dump_al:
	call os_print_char_hex
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; os_dump_regs -- Dump the values on the registers to the screen (For debug purposes)
; IN/OUT: Nothing
os_dump_regs:
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rsp
	push rbp
	push rdi
	push rsi
	push rdx
	push rcx
	push rbx
	push rax

	xor edx, edx
	mov rcx, rsp
	call os_print_newline
	mov rsi, os_dump_reg_string00

os_dump_regs_again:
	lea eax, [edx+edx*4] 			; each string is 5 bytes
	add rsi, rax
	call os_print_string			; Print the register name

	mov rax, [rcx]
	add rcx, 8
	call os_debug_dump_rax
	inc edx
	cmp dl, 0x10
	jnz os_dump_regs_again

	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop rbp
	pop rsp
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15 

ret

os_dump_reg_string00: db '  A:', 0
os_dump_reg_string01: db '  B:', 0
os_dump_reg_string02: db '  C:', 0
os_dump_reg_string03: db '  D:', 0
os_dump_reg_string04: db ' SI:', 0
os_dump_reg_string05: db ' DI:', 0
os_dump_reg_string06: db ' BP:', 0
os_dump_reg_string07: db ' SP:', 0
os_dump_reg_string08: db '  8:', 0
os_dump_reg_string09: db '  9:', 0
os_dump_reg_string0A: db ' 10:', 0
os_dump_reg_string0B: db ' 11:', 0
os_dump_reg_string0C: db ' 12:', 0
os_dump_reg_string0D: db ' 13:', 0
os_dump_reg_string0E: db ' 14:', 0
os_dump_reg_string0F: db ' 15:', 0
os_dump_reg_stage: db 0x00
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; os_dump_mem -- Dump some memory content to the screen (For debug purposes)
; IN: RSI = memory to dump (512bytes)
;OUT: 
os_dump_mem:
	push rdx
	push rcx
	push rbx
	push rax

	push rsi

	mov ecx, 512
dumpit:
	lodsb
	call os_print_char_hex
	dec ecx
	jnz dumpit
	
	pop rsi
	
;	call os_print_newline

	pop rax
	pop rbx
	pop rcx
	pop rdx
ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; os_int_to_string -- Convert a binary interger into an string string
;  IN:	RAX = binary integer
;	RDI = location to store string
; OUT:	RDI = pointer to end of string
;	All other registers preserved
; Min return value is 0 and max return value is 18446744073709551615 so your
; string needs to be able to store at least 21 characters (20 for the number
; and 1 for the string terminator).
; Adapted from http://www.cs.usfca.edu/~cruse/cs210s09/rax2uint.s
os_int_to_string:
	push rdx
	push rcx
	push rbx
	push rax


	xor ecx, ecx				; number of digits generated
	xor ebx, ebx
	mov bl, 10				; base of the decimal system
os_int_to_string_next_divide:
	xor edx, edx				; RAX extended to (RDX,RAX)
	div rbx					; divide by the number-base
	push rdx				; save remainder on the stack
	inc rcx					; and count this remainder
	test rax, rax 				; was the quotient zero?
	jne os_int_to_string_next_divide	; no, do another division
os_int_to_string_next_digit:
	pop rdx					; else pop recent remainder
	add dl, '0'				; and convert to a numeral
	mov [rdi], dl				; store to memory-buffer
	inc rdi
	dec ecx
	jnz os_int_to_string_next_digit		; again for other remainders
	xor eax, eax
	mov [rdi], al				; Store the null terminator at the end of the string

	pop rax
	pop rbx
	pop rcx
	pop rdx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; create_gate
; rax = address of handler
; rdi = gate # to configure
create_gate:
	mov r8, rdi
	mov r9, rax
	
	shl rdi, 4			; quickly multiply rdi by 16
	mov [rdi], ax			; store the low word (15..0)
	shr rax, 16
;	add rdi, 4			; skip the gate marker
	mov [rdi+6], ax			; store the high word (31..16)
	shr rax, 16
	mov [rdi+8], eax		; store the high dword (63..32)

	mov r9, rax
	mov r8, rdi
ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
