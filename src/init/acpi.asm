; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2024 Return Infinity -- see LICENSE.TXT
;
; INIT ACPI
; =============================================================================


init_acpi:
	mov al, [p_BootMode]
	cmp al, 'U'
	je foundACPIfromUEFI
	mov esi, 0x000E0000		; Start looking for the Root System Description Pointer Structure
	mov rbx, 'RSD PTR '		; This in the Signature for the ACPI Structure Table (0x2052545020445352)
searchingforACPI:
	lodsq				; Load a quad word from RSI and store in RAX, then increment RSI by 8
	cmp rax, rbx
	je foundACPI
	cmp esi, 0x000FFFFF		; Keep looking until we get here
	jge noACPI			; ACPI tables couldn't be found, Fail.
	jmp searchingforACPI

foundACPIfromUEFI:
	mov rsi, [0x400830]		; TODO This should be passed properly
	mov rbx, 'RSD PTR '		; This in the Signature for the ACPI Structure Table (0x2052545020445352)
	lodsq
	cmp rax, rbx
	jne noACPI

foundACPI:				; Found a Pointer Structure, verify the checksum
	push rsi
	xor ebx, ebx
	mov ecx, 20			; As per the spec only the first 20 bytes matter
	sub rsi, 8			; Bytes 0 thru 19 must sum to zero
nextchecksum:
	lodsb				; Get a byte
	add bl, al			; Add it to the running total
	sub cl, 1
	cmp cl, 0
	jne nextchecksum
	pop rsi
	cmp bl, 0
	jne searchingforACPI		; Checksum didn't check out? Then keep looking.

	lodsb				; Checksum
	lodsd				; OEMID (First 4 bytes)
	lodsw				; OEMID (Last 2 bytes)
	lodsb				; Grab the Revision value (0 is v1.0, 1 is v2.0, 2 is v3.0, etc)
	cmp al, 0
	je foundACPIv1			; If AL is 0 then the system is using ACPI v1.0
	jmp foundACPIv2			; Otherwise it is v2.0 or higher

foundACPIv1:
	xor eax, eax
	lodsd				; Grab the 32 bit physical address of the RSDT (Offset 16).
	mov rsi, rax			; RSI now points to the RSDT
	lodsd				; Grab the Signature
	cmp eax, 'RSDT'			; Make sure the signature is valid
	jne novalidacpi			; Not the same? Bail out
	sub rsi, 4
	mov [p_ACPITableAddress], rsi	; Save the RSDT Table Address
	add rsi, 4
	xor eax, eax
	lodsd				; Length
	add rsi, 28			; Skip to the Entry offset
	sub eax, 36			; EAX holds the table size. Subtract the preamble
	shr eax, 2			; Divide by 4
	mov rdx, rax			; RDX is the entry count
	xor ecx, ecx
foundACPIv1_nextentry:
	lodsd
	push rax
	add ecx, 1
	cmp ecx, edx
	je findACPITables
	jmp foundACPIv1_nextentry

foundACPIv2:
	lodsd				; RSDT Address
	lodsd				; Length
	lodsq				; Grab the 64 bit physical address of the XSDT (Offset 24).
	mov rsi, rax			; RSI now points to the XSDT
	lodsd				; Grab the Signature
	cmp eax, 'XSDT'			; Make sure the signature is valid
	jne novalidacpi			; Not the same? Bail out
	sub rsi, 4
	mov [p_ACPITableAddress], rsi	; Save the XSDT Table Address
	add rsi, 4
	xor eax, eax
	lodsd				; Length
	add rsi, 28			; Skip to the start of the Entries (offset 36)
	sub eax, 36			; EAX holds the table size. Subtract the preamble
	shr eax, 3			; Divide by 8
	mov rdx, rax			; RDX is the entry count
	xor ecx, ecx
foundACPIv2_nextentry:
	lodsq
	push rax
	add ecx, 1
	cmp ecx, edx
	jne foundACPIv2_nextentry

findACPITables:
	xor ecx, ecx
nextACPITable:
	pop rsi
	lodsd
	add ecx, 1
	mov ebx, 'APIC'			; Signature for the Multiple APIC Description Table
	cmp eax, ebx
	je foundAPICTable
	mov ebx, 'HPET'			; Signature for the HPET Description Table
	cmp eax, ebx
	je foundHPETTable
;	mov ebx, 'MCFG'			; Signature for the PCIe Enhanced Configuration Mechanism
;	cmp eax, ebx
;	je foundMCFGTable
	cmp ecx, edx
	jne nextACPITable
	jmp init_smp_acpi_done		;noACPIAPIC

foundAPICTable:
	call parseAPICTable
	jmp nextACPITable

foundHPETTable:
	call parseHPETTable
	jmp nextACPITable

;foundMCFGTable:
;	call parseMCFGTable
;	jmp nextACPITable

init_smp_acpi_done:
	ret

noACPI:
novalidacpi:
	jmp $


; -----------------------------------------------------------------------------
parseAPICTable:
	push rcx
	push rdx

	lodsd				; Length of MADT in bytes
	mov ecx, eax			; Store the length in ECX
	xor ebx, ebx			; EBX is the counter
	lodsb				; Revision
	lodsb				; Checksum
	lodsd				; OEMID (First 4 bytes)
	lodsw				; OEMID (Last 2 bytes)
	lodsq				; OEM Table ID
	lodsd				; OEM Revision
	lodsd				; Creator ID
	lodsd				; Creator Revision
	lodsd				; Local APIC Address (This should match what was pulled already via the MSR)
	lodsd				; Flags (1 = Dual 8259 Legacy PICs Installed)
	add ebx, 44
	mov rdi, 0x0000000000005100	; Valid CPU IDs

readAPICstructures:
	cmp ebx, ecx
	jge parseAPICTable_done
	lodsb				; APIC Structure Type
	cmp al, 0x00			; Processor Local APIC
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
;	cmp al, 0x06			; I/O SAPIC Structure
;	je APICiosapic
;	cmp al, 0x07			; Local SAPIC Structure
;	je APIClocalsapic
;	cmp al, 0x08			; Platform Interrupt Source Structure
;	je APICplatformint
;	cmp al, 0x09			; Processor Local x2APIC
;	je APICx2apic
;	cmp al, 0x0A			; Local x2APIC NMI
;	je APICx2nmi

	jmp APICignore

APICapic:				; Entry Type 0
	xor eax, eax
	xor edx, edx
	lodsb				; Length (will be set to 8)
	add ebx, eax
	lodsb				; ACPI Processor ID
	lodsb				; APIC ID
	xchg eax, edx			; Save the APIC ID to EDX
	lodsd				; Flags (Bit 0 set if enabled/usable)
	bt eax, 0			; Test to see if usable
	jnc readAPICstructures		; Read the next structure if CPU not usable
	inc word [p_cpu_detected]
	xchg eax, edx			; Restore the APIC ID back to EAX
	stosb
	jmp readAPICstructures		; Read the next structure

APICioapic:				; Entry Type 1
	xor eax, eax
	lodsb				; Length (will be set to 12)
	add ebx, eax
	push rdi
	push rcx
	mov rdi, IM_IOAPICAddress	; Copy this data directly to the InfoMap
	xor ecx, ecx
	mov cl, [p_IOAPICCount]
	shl cx, 4			; Quick multiply by 16
	add rdi, rcx
	pop rcx
	xor eax, eax
	lodsb				; IO APIC ID
	stosd
	lodsb				; Reserved
	lodsd				; I/O APIC Address
	stosd
	lodsd				; Global System Interrupt Base
	stosd
	pop rdi
	inc byte [p_IOAPICCount]
	jmp readAPICstructures		; Read the next structure

APICinterruptsourceoverride:		; Entry Type 2
	xor eax, eax
	lodsb				; Length (will be set to 10)
	add ebx, eax
	push rdi
	push rcx
	mov rdi, IM_IOAPICIntSource	; Copy this data directly to the InfoMap
	xor ecx, ecx
	mov cl, [p_IOAPICIntSourceC]
	shl cx, 3			; Quick multiply by 8
	add rdi, rcx
	lodsb				; Bus Source
	stosb
	lodsb				; IRQ Source
	stosb
	lodsd				; Global System Interrupt
	stosd
	lodsw				; Flags - bit 1 Low(1)/High(0), Bit 3 Level(1)/Edge(0)
	stosw
	pop rcx
	pop rdi
	inc byte [p_IOAPICIntSourceC]
	jmp readAPICstructures		; Read the next structure

;APICx2apic:				; Entry Type 9
;	xor eax, eax
;	xor edx, edx
;	lodsb				; Length (will be set to 16)
;	add ebx, eax
;	lodsw				; Reserved; Must be Zero
;	lodsd
;	xchg eax, edx			; Save the x2APIC ID to EDX
;	lodsd				; Flags (Bit 0 set if enabled/usable)
;	bt eax, 0			; Test to see if usable
;	jnc APICx2apicEnd		; Read the next structure if CPU not usable
;	xchg eax, edx			; Restore the x2APIC ID back to EAX
;	; TODO - Save the ID's somewhere
;APICx2apicEnd:
;	lodsd				; ACPI Processor UID
;	jmp readAPICstructures		; Read the next structure

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
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
parseHPETTable:
	lodsd				; Length of HPET in bytes
	lodsb				; Revision
	lodsb				; Checksum
	lodsd				; OEMID (First 4 bytes)
	lodsw				; OEMID (Last 2 bytes)
	lodsq				; OEM Table ID
	lodsd				; OEM Revision
	lodsd				; Creator ID
	lodsd				; Creator Revision
	lodsd				; Event Timer Block ID
	lodsd				; Base Address Settings
	lodsq				; Base Address Value
	mov [p_HPETAddress], rax	; Save the Address of the HPET
	lodsb				; HPET Number
	lodsw				; Main Counter Minimum
	lodsw				; Page Protection And OEM Attribute
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
;parseMCFGTable:
;	lodsd				; Length of MCFG in bytes
;	lodsb				; Revision
;	lodsb				; Checksum
;	lodsd				; OEMID (First 4 bytes)
;	lodsw				; OEMID (Last 2 bytes)
;	lodsq				; OEM Table ID
;	lodsd				; OEM Revision
;	lodsd				; Creator ID
;	lodsd				; Creator Revision
;	lodsq				; Reserved
;
;	; Loop through each entry
;	lodsq				; Base address of enhanced configuration mechanism
;	lodsw				; PCI Segment Group Number
;	lodsb				; Start PCI bus number decoded by this host bridge
;	lodsb				; End PCI bus number decoded by this host bridge
;	lodsd				; Reserved
;
;	ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
