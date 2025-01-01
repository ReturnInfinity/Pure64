; =============================================================================
; Floppy Disk Driver
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
; Implemented by Isa Isoux -- https://github.com/isoux/BMFLFS

; Here is a minimal simplified version intended only for reading whole floppy 
; disk into memory or writing back image from memory to whole disk at once.
; Errors are poorly handled due to the simplicity of the handling, one thing 
; that can be easily noticed is that the disk is damaged and does not load 
; completely.

; FDC - Floppy Disk Controler
; =============================================================================

STAT_REG_A		EQU 0x3F0	; read-only
STAT_REG_B		EQU 0x3F1	; read-only
DIG_OUT_REG		EQU 0x3F2	; write-only
MAIN_STAT_REG		EQU 0x3F4	; read-only
DATA_REG		EQU 0x3F5	; write-only
DIG_IN_REG		EQU 0x3F7	; read-only

READ			EQU 0x00
WRITE			EQU 0x01

DMA_BUFF EQU 0x210000	; temporary buffer for DMA
MEM_BUFF EQU 0x400000	; Start address of memory buffer

; Some constants needed for increasing delay for
; recalibrate, seek track and interrupt
					;right	choices	is slower...
IRQ_DELAY_STEP		EQU	0x40	;0x60	;0x80	;0xa0	;0xaa
CALIB_DELAY_STEP	EQU	0x60	;0xa0	;0x100	;0x180	;0x200
SEEK_DELAY_STEP		EQU	0x100	;0x180	;0x200	;0x240	;0x280
SHL_HEAD_STEP		EQU	8	;8	;8	;8	;8


; -----------------------------------------------------------------------------
fdc_sendbyte:
	push rcx
	push rax

	mov rcx, 0xff
	.delay:
	loop .delay

	.l1:
	mov dx, MAIN_STAT_REG		; 0x3F4 check status reg
	in al, dx
	and al, 11000000b		; 0xC0 MRQ=1 DIO=1
	cmp al, 10000000b		; 0x80 MRQ=1 DIO=0 ready for write?
	jnz .l1
	pop rax
	pop rcx
	mov dx, DATA_REG		; 0x3F5 send byte
	out dx, al
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
fdc_getbyte:
	push rcx
	push rax

	mov rcx, 0xff
	.delay:
	loop .delay

	.l1:
	mov dx, MAIN_STAT_REG		; 0x3F4 check status reg
	in al, dx
	and al, 11000000b		; 0xC0 MRQ=1 DIO=1
	cmp al, 11000000b		; 0xC0 MRQ=1 DIO=0 ready for read?
	jnz .l1
	pop rax
	pop rcx
	mov dx, DATA_REG		; 0x3F5 get the byte
	in al, dx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
wait_for_irq:
	push rax
	push rcx

	xor ecx, ecx
	xor eax, eax
	mov eax, [track]
	shl eax, SHL_HEAD_STEP
	mov rcx, IRQ_DELAY_STEP
	add rcx, rax
	.delay:
	loop .delay

	.l1:
	mov rax, [int_done]
	or rax, rax
	jz .l1

	clc
	pop rcx
	pop rax
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; Sense interrupt status command
sense_isc:
	push rax

	mov al, 0x08			; fdc command
	call fdc_sendbyte
	call fdc_getbyte
	mov ah, al			; save ST0 in ah
	call fdc_getbyte		; read PCN
	clc
	test ah, 0x80			; test for error:
	jz .end				; "invalid command"
	stc
	.end:
	pop rax
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
fdd_mot_on:
	push rcx

	mov dx, DIG_OUT_REG		; 0x3F2
	mov al, 00011100b		; 0x1C	motor 0 on
	out dx, al

	mov rcx, 0xffff
	delay2:
	loop delay2

	mov byte [motor_on], 1

	pop rcx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
fdd_mot_off:
	push rcx

	mov dx, DIG_OUT_REG		; 0x3F2
	mov al, 00000000b		; 0x0 motor 0 off
	out dx, al

	mov rcx, 0xffff
	delay3:
	loop delay3

	mov byte [motor_on], 0

	pop rcx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
fdc_recalib:
	push rax
	push rcx
	test byte [motor_on], 1
	jnz  .l1
	call fdd_mot_on			; turn motor on
	.l1:
	mov al, 0x07			; recalibrate command
	call fdc_sendbyte
	mov al, 0x00			; selects drive a:
	call fdc_sendbyte
	mov byte [res_C], 0

	xor ecx, ecx
	xor eax, eax
	mov eax, [track]
	shl eax, SHL_HEAD_STEP
	mov rcx, CALIB_DELAY_STEP
	add rcx, rax
	.delay:
	loop .delay

	mov  word [int_done], 0
	call wait_for_irq		; wait for floppy int.
	jc .error

	call sense_isc			; sense interrupt status command
	jc .error

	.error:
	stc
	pop	rcx
	pop rax
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
fdd_seek:
	push rcx
	push rax

	mov al, 0x0F			; seek command
	call fdc_sendbyte
	mov al, [driveno]		; drive # (00 = A)
	call fdc_sendbyte
	mov al, [track]			; cylinder #
	call fdc_sendbyte

	xor ecx, ecx
	xor eax, eax
	mov eax, [track]
	shl eax, SHL_HEAD_STEP
	mov rcx, SEEK_DELAY_STEP
	add rcx, rax
	.delay:
	loop .delay

	mov  word [int_done], 0
	call wait_for_irq		; wait for floppy int.
	jc .error

	call sense_isc			; sense interrupt status command
	jc .error

	pop rax
	clc
	jmp short .end
	.error:
	pop rax
	.end:
	pop rcx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; Read or Write whole track from both sides (heads)
fdd_rw:
	push rcx

	mov byte[errorcode], 4		; put error code in ah, just incase

	test byte [motor_on], 1
	jnz .l1
	call fdd_mot_on
	.l1:
	mov dx, DIG_IN_REG		; 0x3F7
	mov al, 00000000b		; 500Kb/sec mode
	out dx, al
	mov byte [errorcode], 0x80	; put basic error code, just in case.

	call fdc_recalib

	xor rcx, rcx
	mov cx, 3			; we want to try seek 3 times
	.l2:
	call fdd_seek			; we need to move to the right track.
	jnc .l3				; we should be on the right track.
	loop .l2
	jmp .error			; timeout.
	.l3:
	mov dx, MAIN_STAT_REG		; check status reg (to see if DMA bit set)
	in al, dx
	test al, 00100000b		; test sr0 is 0x80
	jnz .error

	cmp byte [rw], READ
	je .read_fdd

	; write_fdd:
	mov bl, 2			; channel 2
	mov esi, 0x4800			; bytes to write
	mov rcx, [dma_buff]		; load address of buffer for transfer
	mov bh, 1			; "read DMA" from writing at floppy,
	call dma_IO

	mov	al, 0xC5		; write sector command
	call	fdc_sendbyte
	jmp	.cont

	.read_fdd:
	mov bl, 2			; channel 2
	mov esi, 0x4800			; bytes to read
	mov rcx, [dma_buff]		; load address of buffer for transfer
	mov bh, 0			; "write DMA" from reading of floppy,
	call dma_IO

	mov al, 0xE6			; 0xe6 read sector command
	call fdc_sendbyte

	.cont:
	mov al, [driveno]		; head no. 0, drive A:
	call fdc_sendbyte
	mov al, [track]			; cylinder
	call fdc_sendbyte

	mov al, 0			; head side 0, but DMA menage to be and 1
	call fdc_sendbyte
	mov al, 1			; sector number, starts at 1
	call fdc_sendbyte
	mov al, 0x02			; sector size - 512 bytes
	call fdc_sendbyte

	mov al, 0x12			; 18 decimal sectors to a track
	call fdc_sendbyte
	mov al, 27			; gap length for a 3.5" 1.44Mb
	call fdc_sendbyte
	mov al, 0xFF			; not used data length cause sec size has been filled
	call fdc_sendbyte

	mov  word [int_done], 0
	call wait_for_irq		; wait for floppy int.
	jc .error

	call fdc_getbyte
	mov [res_ST0], al		; save res_ of ST0 in var
	call fdc_getbyte
	mov [dummy_var], al		; save res_ of ST1 in var
	call fdc_getbyte
	mov [dummy_var], al		; save res_ of ST2 in var
	call fdc_getbyte
	mov [res_C], al			; save res_ of cylinder
	call fdc_getbyte
	mov [dummy_var], al		; save res_ of head
	call fdc_getbyte
	mov [res_R], al			; save res_ of sector number.
	call fdc_getbyte
	mov [dummy_var], al		; save res_ of sector size

	jmp short .end

	.error:
	stc
	.end:
	pop rcx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; This procedure is related with pure64.asm file from Pure64 repo
show_status:
	xor ebx, ebx
	mov bl, [progressbar]
	call debug_progressbar
	add byte [progressbar], 2
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; Because DMA has a 64Kb limit, in one shut it is possible to read only 3
; cylinders from both sides: 3 * 0x4800 = 0xd800 or 55296 bytes * 26 repetitions
; = 78 + 2 = 80 cylinders on 1.44 Mb floppy
; Read whole disk
read_floppy:
	mov rcx, 26
	.l_26:
	push rcx
	mov rcx, 3
	.l_3:
	push rcx
	call fdd_rw			; load whole track * 2 side
	call show_status		; show progress
	pop rcx
	inc byte [track]
	add dword [dma_buff], 0x4800	; track * 2 sides of head
	loop .l_3

	; Copy 3 tracks (* 2 head ) to mem_buff
	xor esi, esi
	xor edi, edi
	xor ecx, ecx
	cld
	mov esi, DMA_BUFF
	mov edi, [mem_buff]
	cmp byte [track], 80
	je .final
	mov rcx, 0x1b00
	rep movsq
	mov dword [dma_buff], DMA_BUFF
	mov [mem_buff], rdi
	pop rcx
	loop .l_26

	; load last 2 tracks
	mov rcx, 2
	jmp .l_3
	.final:
	mov rcx, 0x1200
	rep movsq
	call fdd_mot_off
	ret
; -----------------------------------------------------------------------------


int_done	dw	0
motor_on	db	0
driveno		db	0
track		db	0
errorcode	db	0
res_ST0		db	0
dummy_var	db	0		; For simplicity and memory minimizing
res_C		db	0
res_R		db	0
rw		db	0

progressbar	db	0
dma_buff	dd	DMA_BUFF
mem_buff	dd	MEM_BUFF


; =============================================================================
; EOF

