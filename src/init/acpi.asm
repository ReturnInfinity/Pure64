; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; INIT ACPI
;
; Comments reference the following document:
; Advanced Configuration and Power Interface (ACPI) Specification Release 6.5
; https://uefi.org/sites/default/files/resources/ACPI_Spec_6_5_Aug29.pdf
; =============================================================================


init_acpi:
	mov al, [p_BootMode]		; Check how the system was booted
	cmp al, 'U'			; UEFI?
	je foundACPIfromUEFI		; If so, jump - otherwise fall thru for BIOS

; Find the ACPI RSDP Structure on a BIOS system
	mov esi, 0x000E0000		; Start looking for the Root System Description Pointer Structure
	mov rbx, 'RSD PTR '		; This in the Signature for the ACPI Structure Table (0x2052545020445352)
searchingforACPI:
	lodsq				; Load a quad word from RSI and store in RAX, then increment RSI by 8
	cmp rax, rbx			; Verify the Signature
	je foundACPI
	cmp esi, 0x000FFFFF		; Keep looking until we get here
	jge noACPI			; ACPI tables couldn't be found, fail
	jmp searchingforACPI

; Find the ACPI RSDP Structure on a UEFI system
foundACPIfromUEFI:
	mov rsi, [0x400830]		; TODO This should be passed properly
	mov rbx, 'RSD PTR '		; This in the Signature for the ACPI Structure Table (0x2052545020445352)
	lodsq				; Signature
	cmp rax, rbx			; Verify the Signature
	jne noACPI			; If it isn't a match then fail

; Parse the Root System Description Pointer (RSDP) Structure (5.2.5.3)
foundACPI:				; Found a Pointer Structure, verify the checksum
	push rsi			; Save the RSDP location - currently pointing to the checksum
	xor ebx, ebx
	mov ecx, 20			; As per the spec only the first 20 bytes matter
	sub rsi, 8			; Bytes 0 thru 19 must sum to zero
nextchecksum:
	lodsb				; Get a byte
	add bl, al			; Add it to the running total
	sub cl, 1
	cmp cl, 0
	jne nextchecksum
	pop rsi				; Restore the RSDP location
	cmp bl, 0			; Verify the checksum is zero
	jne noACPI			; Checksum didn't check out? Fail

	lodsb				; Checksum
	lodsd				; OEMID (First 4 bytes)
	lodsw				; OEMID (Last 2 bytes)
	lodsb				; Revision (0 is v1.0, 1 is v2.0, 2 is v3.0, etc)
	cmp al, 0
	je foundACPIv1			; If AL is 0 then the system is using ACPI v1.0
	jmp foundACPIv2			; Otherwise it is v2.0 or higher

foundACPIv1:				; Root System Description Table (RSDT)
	xor eax, eax
	lodsd				; RsdtAddress - 32 bit physical address of the RSDT (Offset 16)
	mov rsi, rax			; RSI now points to the RSDT
	lodsd				; Load Signature
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
	lodsd				; Load a 32-bit Entry address
	push rax			; Push it to the stack as a 64-bit value
	add ecx, 1
	cmp ecx, edx
	je findACPITables
	jmp foundACPIv1_nextentry

foundACPIv2:				; Extended System Description Table (XSDT)
	lodsd				; RsdtAddress - 32 bit physical address of the RSDT (Offset 16)
	lodsd				; Length
	lodsq				; XsdtAddress - 64 bit physical address of the XSDT (Offset 24).
	mov rsi, rax			; RSI now points to the XSDT
	lodsd				; Load Signature
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
	lodsq				; Load a 64-bit Entry address
	push rax			; Push it to the stack
	add ecx, 1
	cmp ecx, edx
	jne foundACPIv2_nextentry

findACPITables:
	xor ecx, ecx
nextACPITable:
	cmp ecx, edx			; Compare current count to entry count
	je init_smp_acpi_done
	pop rsi				; Pop an Entry address from the stack
	lodsd
	add ecx, 1
	mov ebx, 'APIC'			; Signature for the Multiple APIC Description Table
	cmp eax, ebx
	je foundAPICTable
	mov ebx, 'HPET'			; Signature for the HPET Description Table
	cmp eax, ebx
	je foundHPETTable
	mov ebx, 'MCFG'			; Signature for the PCIe Enhanced Configuration Mechanism
	cmp eax, ebx
	je foundMCFGTable
	mov ebx, 'FADT'			; Signature for the Fixed ACPI Description Table
	cmp eax, ebx
	je foundFADTTable
	jmp nextACPITable

foundAPICTable:
	call parseAPICTable
	jmp nextACPITable

foundHPETTable:
	call parseHPETTable
	jmp nextACPITable

foundMCFGTable:
	call parseMCFGTable
	jmp nextACPITable

foundFADTTable:
	call parseFADTTable
	jmp nextACPITable

init_smp_acpi_done:
	ret

noACPI:
novalidacpi:
	; Set screen to Teal
	mov rdi, [0x00005F00]		; Frame buffer base
	mov rcx, [0x00005F08]		; Frame buffer size
	shr rcx, 2			; Quick divide by 4
	mov eax, 0x0000FFFF		; 0x00RRGGBB
	rep stosd
	jmp $


; -----------------------------------------------------------------------------
; 5.2.12 Multiple APIC Description Table (MADT)
; Chapter 5.2.12
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
; High Precision Event Timer (HPET)
; http://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf
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

	lodsb				; Hardware Revision ID
	lodsb				; # of Comparators (5:0), COUNT_SIZE_CAP (6), Legacy IRQ (7)
	lodsw				; PCI Vendor ID
	lodsd				; Generic Address Structure
	lodsq				; Base Address Value
	mov [p_HPET_Address], rax	; Save the Address of the HPET
	lodsb				; HPET Number
	lodsw				; Main Counter Minimum
	mov [p_HPET_CounterMin], ax	; Save the Counter Minimum
	lodsb				; Page Protection And OEM Attribute
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; PCI Express Memory-mapped Configuration (MCFG)
; Locked behind a paywall - Search Google for "pcie specification pdf"
parseMCFGTable:
	push rdi
	push rcx
	xor eax, eax
	xor ecx, ecx
	mov cx, [p_PCIECount]
	shl ecx, 4
	mov rdi, IM_PCIE
	add rdi, rcx
	lodsd				; Length of MCFG in bytes
	sub eax, 44			; Subtract the size of the table header
	shr eax, 4			; Quick divide by 16
	mov ecx, eax			; ECX now stores the number of 16-byte records
	add word [p_PCIECount], cx
	lodsb				; Revision
	lodsb				; Checksum
	lodsd				; OEMID (First 4 bytes)
	lodsw				; OEMID (Last 2 bytes)
	lodsq				; OEM Table ID
	lodsd				; OEM Revision
	lodsd				; Creator ID
	lodsd				; Creator Revision
	lodsq				; Reserved

	; Loop through each entry
parseMCFGTable_next:
	lodsq				; Base address of enhanced configuration mechanism
	stosq
	lodsw				; PCI Segment Group Number
	stosw
	lodsb				; Start PCI bus number decoded by this host bridge
	stosb
	lodsb				; End PCI bus number decoded by this host bridge
	stosb
	lodsd				; Reserved
	stosd
	sub ecx, 1
	jnz parseMCFGTable_next
	xor eax, eax
	not rax				; 0xFFFFFFFFFFFFFFFF
	stosq				; Mark the end of the table
	stosq

	pop rcx
	pop rdi
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; Fixed ACPI Description Table (FADT)
; Chapter 5.2.9
parseFADTTable:
	; At this point RSI points to offset 4 for the FADT
	add rsi, 116			; RESET_REG (Generic Address Structure - 5.2.3.2)
	lodsb				; Address Space ID (0x00 = Memory, 0x01 = I/O, 0x02 = PCI)
	lodsb				; Register Width
	lodsb				; Register Offset
	lodsb				; Access Size
	lodsq				; Address
	lodsb				; RESET_VALUE

;	add rsi, 36
;	lodsd				; DSDT
;	add rsi, 20
;	lodsd				; PM1a_CNT_BLK
	ret
; -----------------------------------------------------------------------------


; =============================================================================
; EOF
