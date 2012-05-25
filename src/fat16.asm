; =============================================================================
; Pure64 -- a 64-bit OS loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2012 Return Infinity -- see LICENSE.TXT
;
; FAT16 functions
; =============================================================================


; -----------------------------------------------------------------------------
; readcluster -- Read a cluster from the FAT16 partition
; IN:	AX - (cluster)
;	RDI - (memory location to store at least 32KB)
; OUT:	AX - (next cluster)
;	RDI - points one byte after the last byte read
readcluster:
	push rsi
	push rdx
	push rcx
	push rbx
	
	and rax, 0x000000000000FFFF		; Clear the top 48 bits
	mov rbx, rax				; Save the cluster number to be used later

	cmp ax, 2				; If less than 2 then bail out...
	jl near readcluster_bailout		; as clusters start at 2

; Calculate the LBA address --- startingsector = (cluster-2) * clustersize + data_start
	xor rcx, rcx	
	mov cl, byte [fat16_SectorsPerCluster]
	push rcx				; Save the number of sectors per cluster
	sub ax, 2
	imul cx					; EAX now holds starting sector
	add eax, dword [fat16_DataStart]	; EAX now holds the sector where our cluster starts
	add eax, [fat16_PartitionOffset]	; Add the offset to the partition

	pop rcx					; Restore the number of sectors per cluster
	call readsectors			; Read one cluster of sectors
	cmp rcx, 0
	je readcluster_error

; Calculate the next cluster
; Psuedo-code
; tint1 = Cluster / 256  <- Dump the remainder
; sector_to_read = tint1 + ReservedSectors
; tint2 = (Cluster - (tint1 * 256)) * 2
	push rdi
	mov rdi, hdbuffer1			; Read to this temporary buffer
	mov rsi, rdi				; Copy buffer address to RSI
	push rbx				; Save the original cluster value
	shr rbx, 8				; Divide the cluster value by 256. Keep no remainder
	movzx ax, [fat16_ReservedSectors]	; First sector of the first FAT
	add eax, [fat16_PartitionOffset]	; Add the offset to the partition
	add rax, rbx				; Add the sector offset
	mov rcx, 1
	call readsectors
	cmp rcx, 0
	je readcluster_error
	pop rax					; Get our original cluster value back
	shl rbx, 8				; Quick multiply by 256 (RBX was the sector offset in the FAT)
	sub rax, rbx				; RAX is now pointed to the offset within the sector
	shl rax, 1				; Quickly multiply by 2 (since entries are 16-bit)
	add rsi, rax
	lodsw					; AX now holds the next cluster
	pop rdi
	
	jmp readcluster_end

readcluster_bailout:
	xor ax, ax

readcluster_end:
	pop rbx
	pop rcx
	pop rdx
	pop rsi
ret

readcluster_error:
	mov rsi, readcluster_err_msg
	call os_print_string
	jmp exception_gate_halt
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; findfile -- 
; IN:	RSI(Pointer to file name, must be in 'FILENAMEEXT" format)
; OUT:	AX(Staring cluster), 0x0 if not found
; Notes: Only searches the root sector.. not the following sectors.
findfile:
	push rsi
	push rdi
	push rdx
	push rcx
	push rbx

	clc				; Clear carry
	xor rax, rax
	mov eax, [fat16_RootStart]	; eax points to the first sector of the root
	add eax, [fat16_PartitionOffset]	; Add the offset to the partition
	mov rdx, rax			; Save the sector value

os_fat16_find_file_read_sector:
	mov rdi, hdbuffer1
	push rdi
	mov rcx, 1
	call readsectors
	cmp rcx, 0
	je os_fat16_find_file_error
	pop rdi
	mov rbx, 16			; Each record is 32 bytes. 512 (bytes per sector) / 32 = 16

os_fat16_find_file_next_entry:
	cmp byte [rdi], 0x00		; end of records
	je os_fat16_find_file_notfound
	
	mov rcx, 11
	push rsi
	repe cmpsb
	pop rsi
	mov ax, [rdi+15]		; AX now holds the starting cluster # of the file we just looked at
	jz os_fat16_find_file_done	; The file was found. Note that rdi now is at dirent+11

	add rdi, byte 0x20
	and rdi, byte -0x20
	dec rbx
	cmp rbx, 0
	jne os_fat16_find_file_next_entry

; At this point we have read though one sector of file names. We have not found the file we are looking for and have not reached the end of the table. Load the next sector.

	add rdx, 1
	mov rax, rdx
	jmp os_fat16_find_file_read_sector

os_fat16_find_file_notfound:
	stc				; Set carry
	xor rax, rax

os_fat16_find_file_done:
	cmp ax, 0x0000			; BUG HERE
	jne wut				; Carry is not being set properly in this function
	stc
wut:
	pop rbx
	pop rcx
	pop rdx
	pop rdi
	pop rsi
ret

os_fat16_find_file_error:
	mov rsi, findfile_err_msg
	call os_print_string
	jmp exception_gate_halt
; -----------------------------------------------------------------------------

readcluster_err_msg:	db 'Error reading cluster', 0
findfile_err_msg:	db 'Error finding file', 0

; =============================================================================
; EOF
