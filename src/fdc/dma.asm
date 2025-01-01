; =============================================================================
; DMA Driver
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
; Implemented by Isa Isoux -- https://github.com/isoux/BMFLFS

; The DMA controller performs data transfers:
; DMA read – transfer from memory to I/O device
; DMA write - transfer from I/O device to memory
; Memory to memory transfers

; Here is a minimal simplified version intended only for floppy drive

; DMA - controller for 8 bit transfer
; =============================================================================

;DMA_controler1:
DMA1_CH2_CAR	EQU 0x04		; 16 bits
DMA1_CH2_CWCR	EQU 0x05		; 16 bits
DMA1_MASK_REG	EQU 0x0A
DMA1_MODE_REG	EQU 0x0B		; 8 bits  Mode register – MR;
DMA1_CLEAR_REG	EQU 0x0C

;DMA_page_register
DMA1_CHR_ADDR	EQU 0x81

; -----------------------------------------------------------------------------
; dma_IO:
; DMA read write procedure
; IN:
; ecx = page:offset
; bl  = channel
; bh  = 1 = read, 0 = write
; esi = count
dma_IO: 

; Examine whether it is writing or reading
; what_mode
	or bh, bh
	jz write_mode
	mov bh, bl
	add bh, 0x48			; + Ch2 = 0x4A
	jmp short read_mode
	write_mode:			; DMA write: transfer from I/O device to memory;
	mov bh, bl
	add bh, 0x44			; + Ch2 = 0x46
	read_mode:			; DMA read: transfer from memory to I/O device;

; Disable 2. channel
	dec esi
	movzx dx, byte[mask_reg]
	mov al, bl
	or al, byte[ch_car]
	out dx, al			; disable the channel

; Setup flip flop
	movzx dx, byte[clear_reg]
	mov al, 0
	out dx, al			; init flip_flop

; At mode register put 0x46 for write or 0x4A for read
; Set mode for read or write
	movzx dx, byte[mode_reg]
	mov al, bh			; 1=read, 0=write
	out dx, al			; set DMA mode

; Write low-high offsets
	movzx dx, byte[ch_car]
	mov al, cl
	out dx, al			; write low offset
	mov al, ch
	out dx, al			; write high offset

; write_pg_port
	movzx dx, byte[pg_prt_reg]
	mov eax, ecx
	shr eax, 16
	out dx, al			; write page.

; Write_cnt_port
	movzx eax, bl
	movzx dx, byte[ch_cwcr]
	mov eax, esi
	out dx, al			; low count
	mov al, ah
	out dx, al			; high count

; Enable channel 2 at mask_reg
	movzx dx, byte[mask_reg]
	mov al, bl
	out dx, al			; enable channel
	ret
; -----------------------------------------------------------------------------

; Everything is for dma1 channel2 = FDC
; dma_struct:
ch_car		db	DMA1_CH2_CAR	; Chanel x ADDRES REGISTER
ch_cwcr		db	DMA1_CH2_CWCR	; Chanel word count
mask_reg	db	DMA1_MASK_REG	; Master register
mode_reg	db	DMA1_MODE_REG	; Mode register
clear_reg	db	DMA1_CLEAR_REG	; Clear MSB/LSB flip flop
pg_prt_reg	db	DMA1_CHR_ADDR	; Page port reg = High 4 bit of DMA channel x address


; =============================================================================
; EOF
