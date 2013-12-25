; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2013 Return Infinity -- see LICENSE.TXT
;
; INIT IO-APIC
; =============================================================================


init_ioapic:
	xor eax, eax
	mov al, 0x70			; IMCR access
	out 0x22, al
	mov al, 0x01			; set bit 1 for SMP mode
	out 0x23, al
	mov rsi, [os_IOAPICAddress]
	xor ecx, ecx
	mov cl, 1			; Register 1 - IOAPIC VERSION REGISTER
	mov [rsi], ecx
	mov eax, [rsi+0x10]
	shr eax, 16			; Extract bytes 16-23 (Maximum Redirection Entry)
	movzx ecx, ax			; Clear bits 16-31
	add ecx, 1
	xor eax, eax
	bts rax, 47			; Interrupt Mask Enabled
initentry:				; Initialize all entries 1:1
	or rax,rcx
	mov [rsi], rax
	dec ecx
	jnz initentry

	; Get the BSP APIC ID
			
	mov eax, [rsi+0x20]		; Load a 32-bit value. We only want the high 8 bits
	shr eax, 24			; Shift to the right and AL now holds the CPU's APIC ID
	shl rax, 56

	; Enable the Keyboard
	xor ecx, ecx
	mov cl, 0x21			; Interrupt Value
	or  rax,rcx
	mov cl, 0x12			; IRQ value 1, 2*1+16=18=0x12				; Interrupt value
	mov [rsi], ecx
	mov [rsi+0x10], rax

	; Enable the RTC		; IRQ value
	mov cl, 0x28			; Interrupt value
	or rax, rcx
	mov cl, 0x20			; 2*8+16=32=0x20
	mov [rsi], ecx
	mov [rsi+0x10], rax

	; Set the periodic flag in the RTC
	xor eax, eax
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	in al, 0x71			; Read the current settings
	mov ecx, eax
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	mov eax, ecx
	bts ax, 6			; Set Periodic(6)
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
	add rcx, 0x10			; IO Redirection tables start at 0x10

	; Write lower DWORD
	call ioapic_reg_write

	; Write higher DWORD
	shr rax, 32
	add rcx, 1
	call ioapic_reg_write

	pop rcx
	pop rax
	ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
