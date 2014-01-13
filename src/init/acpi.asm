; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2013 Return Infinity -- see LICENSE.TXT
;
; INIT ACPI
; =============================================================================


init_acpi:
	mov esi, 0xE0000		; Start looking for the Root System Description Pointer Structure
	mov rbx, 'RSD PTR '		; This in the Signature for the ACPI Structure Table (0x2052545020445352)
searchingforACPI:
	mov rax, [rsi]			; Load a quad word from RSI and store in RAX, then increment RSI by 8
	add rsi, 8
	cmp rsi, 0xFFFFF		; Keep looking until we get here
	jge noACPI			; ACPI tables couldn't be found, Fail.
	cmp rax, rbx
	jne searchingforACPI

foundACPI:				; Found a Pointer Structure, verify the checksum
	push rsi
	xor ebx, ebx
	mov ecx, -8
nextchecksum:
	mov rax, [rsi+rcx]				; Get a byte
	movzx edx, al			; Add it to the running total
	add ebx, edx
	shl rax, 8
	movzx edx, al			; Add it to the running total
	add ebx, edx
	shl rax, 8
	movzx edx, al			; Add it to the running total
	add ebx, edx
	shl rax, 8
	movzx edx, al			; Add it to the running total
	add ebx, edx
	shl rax, 8
	movzx edx, al			; Add it to the running total
	add ebx, edx
	shl rax, 8
	movzx edx, al			; Add it to the running total
	add ebx, edx
	shl rax, 8
	movzx edx, al			; Add it to the running total
	add ebx, edx
	shl rax, 8
	movzx edx, al			; Add it to the running total
	add ebx, edx
	shl rax, 8
	add ecx, 8
	jz nextchecksum
	mov eax, [rsi+16]
	movzx edx, al
	add ebx, edx
	shl eax, 8
	movzx edx, al
	add ebx, edx
	shl eax, 8
	movzx edx, al
	add ebx, edx
	shl eax, 8
	movzx edx, al
	add ebx, edx
	shl eax, 8
	
	cmp ebx, ebx
	jne searchingforACPI		; Checksum didn't check out? Then keep looking.
	add rsi, 27 
;	lodsb				; Checksum
;	lodsd				; OEMID (First 4 bytes)
;	lodsw				; OEMID (Last 2 bytes)
;	lodsb				; Grab the Revision value (0 is v1.0, 1 is v2.0, 2 is v3.0, etc)
	movzx eax, byte [rsi]
	lea ebx, [eax+49]
	mov [0x000B8098], al		; Print the ACPI version number
	test eax, eax
	jnz foundACPIv2			; Otherwise it is v2.0 or higher

foundACPIv1:
	mov rsi, [rsi]			; Grab the 32 bit physical address of the RSDT (Offset 16).
	mov eax, [rsi]			; Grab the Signiture
	cmp eax, 'RSDT'			; Make sure the signiture is valid
	jne novalidacpi			; Not the same? Bail out
	mov [os_ACPITableAddress], rsi	; Save the RSDT Table Address
	mov eax, [rsi+4]		; Length
	add rsi, 32			; Skip to the Entry offset
	sub eax, 36			; EAX holds the table size. Subtract the preamble
	shr eax, 2			; Divide by 4
	mov edx, eax			; RDX is the entry count
	xor ecx, ecx
foundACPIv1_nextentry:
	mov eax, [rsi+rcx*8]
	push rax
	inc ecx
	cmp ecx, edx
	je findACPITables
	jmp foundACPIv1_nextentry

foundACPIv2:
	mov rsi, [rsi+8]		; Grab the 64 bit physical address of the XSDT (Offset 24).
					; RSI now points to the XSDT
	mov eax, [rsi]			; Grab the Signiture
	cmp eax, 'XSDT'			; Make sure the signiture is valid
	jne novalidacpi			; Not the same? Bail out
	mov [os_ACPITableAddress], rsi	; Save the XSDT Table Address
	mov eax, [rsi+4]		; Length
	add rsi, 32			; Skip to the start of the Entries (offset 36)
	sub eax, 36			; EAX holds the table size. Subtract the preamble
	shr eax, 3			; Divide by 8
	mov edx, eax			; RDX is the entry count
	xor ecx, ecx
foundACPIv2_nextentry:
	mov eax, [rsi+rcx*8]
	push rax
	inc ecx
	cmp ecx, edx
	jne foundACPIv2_nextentry

findACPITables:
	xor eax, eax
	xor ecx, ecx
	mov al, '3'			; Search through the ACPI tables
	mov cl, '4'
	shl ecx, 16
	or eax, ecx
	mov [0x000B809C], ax
;	mov al, '4'
;	mov [0x000B809E], al
	xor ecx, ecx
	mov ebx, 'APIC'			; Signature for the Multiple APIC Description Table
	mov edi, 'HPET'			; Signiture for the HPET Description Table
nextACPITable:
	pop rsi
	mov eax, [rsi]
	cmp eax, ebx
	je parseAPICTable
	cmp eax, edi
	je parseHPETTable
	inc ecx
	cmp ecx, edx
	jne nextACPITable
	ret				;noACPIAPIC

init_smp_acpi_done:
	ret

noACPI:
novalidacpi:
	mov al, 'X'
	mov [0x000B809A], al	
	jmp $


; -----------------------------------------------------------------------------
parseAPICTable:
	push rcx
	push rdx

	mov ecx, [rsi]			; Length of MADT in bytes
	xor ebx, ebx			; EBX is the counter
;	lodsb				; Revision
;	lodsb				; Checksum
;	lodsd				; OEMID (First 4 bytes)
;	lodsw				; OEMID (Last 2 bytes)
;	lodsq				; OEM Table ID
;	lodsd				; OEM Revision
;	lodsd				; Creator ID
;	lodsd				; Creator Revision
;	xor eax, eax
	mov rax, [rsi+32]		; Local APIC Address
	mov [os_LocalAPICAddress], rax	; Save the Address of the Local APIC
	mov eax, [rsi+40]		; Flags
	add ebx, 44
	add rsi, 44
	mov rdi, 0x0000000000005100	; Valid CPU IDs

readAPICstructures:
	cmp ebx, ecx
	jge parseAPICTable_done
;	call os_print_newline
	movzx eax, byte [rsi]		; APIC Structure Type
;	call os_debug_dump_al
;	push rax
;	mov al, ' '
;	call os_print_char
;	pop rax
	je APICapic
	cmp al, 0x01			; I/O APIC
	je APICioapic
	cmp al, 0x02			; Interrupt Source Override
	je APICinterruptsourceoverride
;	cmp al, 0x03			; Non-maskable Interrupt Source (NMI)
;	je APICnmi
;	cmp al, 0x04			; Local APIC NMI
;	je APIClocalapicnmi
;	cmp al, 0x05			; Local APIC Address Override
;	je APICaddressoverride
	cmp al, 0x09			; Processor Local x2APIC
	je APICx2apic
;	cmp al, 0x0A			; Local x2APIC NMI
;	je APICx2nmi
	test eax, eax			; Processor Local APIC
	jnz APICignore

APICapic:
	xor eax, eax
	xor edx, edx
	mov edx, [rsi]			; Length (will be set to 8)
	movzx eax, dl
	shl edx, 24
	add rsi, 4
	add ebx, eax
	movzx eax, byte [rsi]		; Flags (Bit 0 set if enabled/usable)
	test al, 1			; Test to see if usable
	jnz readAPICstructures		; Read the next structure if CPU not usable
	mov eax, edx			; Restore the APIC ID back to EAX
	movzx edx, word [cpu_detected]
	add edx, 1
	mov [cpu_detected], dx
	mov [rdi], al
	jmp readAPICstructures		; Read the next structure

APICioapic:
	xor eax, eax
	lodsb				; Length (will be set to 12)
	add ebx, eax
	lodsb				; IO APIC ID
	lodsb				; Reserved
	xor eax, eax
	lodsd				; IO APIC Address
	push rdi
	push rcx
	mov rdi, os_IOAPICAddress
	movzx ecx, byte [os_IOAPICCount]
	shl ecx, 3			; Quick multiply by 8
	add rdi, rcx
	pop rcx
	stosd				; Store the IO APIC Address
	lodsd				; System Vector Base
	stosd				; Store the IO APIC Vector Base
	pop rdi
	inc byte [os_IOAPICCount]
	jmp readAPICstructures		; Read the next structure

APICinterruptsourceoverride:
	xor eax, eax
	lodsb				; Length (will be set to 10)
	add ebx, eax
	lodsb				; Bus
	lodsb				; Source
;	call os_print_newline
;	call os_debug_dump_al
;	mov al, ' '
;	call os_print_char
	lodsd				; Global System Interrupt
;	call os_debug_dump_eax
	lodsw				; Flags
	jmp readAPICstructures		; Read the next structure

APICx2apic:
	xor eax, eax
	xor edx, edx
	movzx eax, byte [rsi]		; Length (will be set to 16)
	add ebx, eax
	mov rdx, [rsi+7]		; Save the x2APIC ID to EDX
	mov eax, edx
	shr rdx, 32	
	test al, 1			; Test to see if usable
	jne APICx2apicEnd		; Read the next structure if CPU not usable
	mov eax, edx			; Restore the x2APIC ID back to EAX
	call os_debug_dump_eax
	call os_print_newline
	; Save the ID's somewhere
APICx2apicEnd:
	add rsi, 15			; ACPI Processor UID
	jmp readAPICstructures		; Read the next structure

APICignore:
	xor eax, eax
	lodsb				; We have a type that we ignore, read the next byte
	add ebx, eax
	add rsi, rax
	sub rsi, 2			; For the two bytes just read
	jmp readAPICstructures		; Read the next structure

parseAPICTable_done:
	pop rdx
	pop rcx
	jmp nextACPITable
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
parseHPETTable:
;	lodsd				; Length of HPET in bytes
;	lodsb				; Revision
;	lodsb				; Checksum
;	lodsd				; OEMID (First 4 bytes)
;	lodsw				; OEMID (Last 2 bytes)
;	lodsq				; OEM Table ID
;	lodsd				; OEM Revision
;	lodsd				; Creator ID
;	lodsd				; Creator Revision
;	lodsd				; Event Timer Block ID
;	lodsd				; Base Address Settings
;	lodsq				; Base Address Value
	mov rax, [rsi+48]
	mov [os_HPETAddress], rax	; Save the Address of the HPET
	add rsi, 61
;	lodsb				; HPET Number
;	lodsw				; Main Counter Minimum
;	lodsw				; Page Protection And OEM Attribute
	jmp nextACPITable
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
