; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2012 Return Infinity -- see LICENSE.TXT
;
; INIT IO-APIC
; =============================================================================


init_ioapic:
	mov al, 0x70			; IMCR access
	out 0x22, al
	mov al, 0x01			; set bit 1 for SMP mode
	out 0x23, al

	xor eax, eax
	mov rcx, 1			; Register 1 - IOAPIC VERSION REGISTER
	call ioapic_reg_read
	shr eax, 16			; Extract bytes 16-23 (Maximum Redirection Entry)
	and eax, 0xFF			; Clear bits 16-31
	add eax, 1
	mov rcx, rax
	xor rax, rax
	mov eax, dword [rsi+0x20]	; Grab the BSP APIC ID; stored in bits 31:24
	shr rax, 24			; AL now holds the BSP CPU's APIC ID
	shl rax, 56
	bts rax, 16			; Interrupt Mask Enabled
initentry:				; Initialize all entries 1:1
	dec rcx
	call ioapic_entry_write
	cmp rcx, 0
	jne initentry

	; Enable the Keyboard
	mov rcx, 1			; IRQ value
	mov rax, 0x21			; Interrupt value
	call ioapic_entry_write

	; Enable the RTC
	mov rcx, 8			; IRQ value
	mov rax, 0x28			; Interrupt value
	call ioapic_entry_write

	; Set the periodic flag in the RTC
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	in al, 0x71			; Read the current settings
	push rax
	mov al, 0x0B			; Status Register B
	out 0x70, al			; Select the address
	pop rax
	bts ax, 6			; Set Periodic(6)
	out 0x71, al			; Write the new settings

	sti				; Enable interrupts

	; Acknowledge the RTC
	mov al, 0x0C			; Status Register C
	out 0x70, al			; Select the address
	in al, 0x71			; Read the current settings

	ret


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
; ioapic_entry_write -- Write to an I/O APIC entry in the redirection table
;  IN:	RAX = Data to write to entry
;	ECX = Index of the entry
; OUT:	Nothing. All registers preserved
ioapic_entry_write:
	push rax
	push rcx

	; Calculate index for lower DWORD
	shl rcx, 1				; Quick multiply by 2
	add rcx, 0x10				; IO Redirection tables start at 0x10

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


; -----------------------------------------------------------------------------
; ioapic_entry_read -- Read an I/O APIC entry from the redirection table
;  IN:	ECX = Index of the entry
; OUT:	RAX = Data that was read
;	All other registers preserved
ioapic_entry_read:
	push rbx
	push rcx

	; Calculate index for lower DWORD
	shl rcx, 1				; Quick multiply by 2
	add rcx, 0x10				; IO Redirection tables start at 0x10

	; Read lower DWORD
	call ioapic_reg_read
	mov rbx, rax

	; Read higher DWORD
	add rcx, 1
	call ioapic_reg_read

	; Combine
	shr rax, 32
	or rbx, rax
	xchg rbx, rax

	pop rcx
	pop rbx
	ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
