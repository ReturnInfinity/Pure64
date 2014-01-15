; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2013 Return Infinity -- see LICENSE.TXT
;
; INIT IO-APIC
; =============================================================================


init_ioapic:
	mov rsi, [os_IOAPICAddress]
	xor edx, edx
	mov dl, 0x10			; offset
	xor eax, eax
	mov al, 0x70			; IMCR access
	out 0x22, al
	mov al, 0x01			; set bit 1 for SMP mode
	out 0x23, al

	xor eax, eax
	lea ecx, [eax+1]		; Register 1 - IOAPIC VERSION REGISTER
	mov dword [rsi], ecx            ; Write index to register selector
	mov eax, dword [rsi + rdx]     ; Read data from window register
	shr eax, 16			; Extract byte 16-23 (Maximum Redirection Entry)
	movzx ecx, ax
	xor eax, eax
	bts eax, 16			; Interrupt Mask Enabled
initentry:				; Initialize all entries 1:1
	lea ebx, [edx+ecx*4]
	mov [rsi], ebx
	mov [rsi+rdx], eax
	add ebx, 1
	mov [rsi], ebx
	mov [rsi+rdx], edx
	dec ecx
	jnz initentry

	; Get the BSP APIC ID
	mov eax, [rsi+0x20]		; Load a 32-bit value. We only want the high 8 bits
	xor ebx, ebx
	shr eax, 24			; Shift to the right and AL now holds the CPU's APIC ID
	shl eax, 56

	; Enable the Keyboard
	xor ecx, ecx
	mov cl, 0x12			; IRQ 1 <<2 +0x10 =0x12
	mov bl, 0x21			; Interrupt value
	or rbx, rax
	mov [rsi], ecx
	mov [rsi+rdx], ebx
	shr rbx, 32
	add ecx, 1
	mov [rsi], ecx			
	mov [rsi+rdx], ebx		; high-dword

	; Enable the RTC
	xor ebx, ebx
	mov bl, 0x28			; Interrupt value
	or rbx, rax
	mov [rsi], ecx
	mov [rsi+rdx], ebx
	shr rbx, 32
	add ecx, 1
	mov [rsi], ecx			
	mov [rsi+rdx], ebx		; high-dword


	; Set the periodic flag in the RTC
	xor eax, eax
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	in al, 0x71			; Read the current settings
	movzx ebx, al
	bts ebx, 6			; Set Periodic(6)
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	mov eax, ebx
	out 0x71, al			; Write the new settings

	sti				; Enable interrupts

	; Acknowledge the RTC
	mov al, 0x0C			; Status Register C
	out 0x70, al			; Select the address
	in al, 0x71			; Read the current settings

	ret


; -----------------------------------------------------------------------------
; ioapic_reg_read -- Read from an I/O APIC register
;  IN:	ECX = Index of register 
; OUT:	EAX = Value of register
;	All other registers preserved
ioapic_reg_read:
	push rsi
	mov rsi, [os_IOAPICAddress]
	mov dword [rsi], ecx		; Write index to register selector
	mov eax, dword [rsi + 0x10]	; Read data from window register
	pop rsi
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; ioapic_reg_write -- Write to an I/O APIC register
;  IN:	EAX = Value to write
;	ECX = Index of register 
; OUT:	Nothing. All registers preserved
ioapic_reg_write:
	push rsi
	mov rsi, [os_IOAPICAddress]
	mov dword [rsi], ecx		; Write index to register selector
	mov dword [rsi + 0x10], eax	; Write data to window register
	pop rsi
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; ioapic_entry_write -- Write to an I/O APIC entry in the redirection table
;  IN:	RAX = Data to write to entry
;	ECX = Index of the entry
; OUT:	Nothing. All registers preserved
ioapic_entry_write:
	push rax
	push rcx

	; Calculate index for lower DWORD
	shl rcx, 1			; Quick multiply by 2
	add ecx, 0x10			; IO Redirection tables start at 0x10

	; Write lower DWORD
	mov rsi, [os_IOAPICAddress]
	mov dword [rsi], ecx		; Write index to register selector
	mov dword [rsi + 0x10], eax	; Write data to window register

	; Write higher DWORD
	shr rax, 32
	add ecx, 1
	mov dword [rsi], ecx		; Write index to register selector
	mov dword [rsi + 0x10], eax	; Write data to window register

	pop rcx
	pop rax
	ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
