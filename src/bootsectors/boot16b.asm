USE16
org 0x7C00

entry:
	jmp short begin
	nop

%define bsOemName	bp+0x03 	; OEM label (8)
%define bsBytesPerSec	bp+0x0B 	; bytes/sector (dw)
%define bsSecsPerClust	bp+0x0D 	; sectors/allocation unit (db)
%define bsResSectors	bp+0x0E 	; # reserved sectors (dw)
%define bsFATs		bp+0x10 	; # of fats (db)
%define bsRootDirEnts	bp+0x11 	; Max # of root dir entries (dw)
%define bsSectors	bp+0x13 	; # sectors total in image (dw)
%define bsMedia 	bp+0x15 	; media descriptor (db)
%define bsSecsPerFat	bp+0x16 	; # sectors in a fat (dw)
%define bsSecsPerTrack	bp+0x18 	; # sectors/track
%define bsHeads		bp+0x1A 	; # heads (dw)
%define bsHidden 	bp+0x1C 	; # hidden sectors (dd)
%define bsSectorHuge	bp+0x20 	; # sectors if > 65536 (dd)
%define bsDriveNumber	bp+0x24 	; (dw)
%define bsSigniture	bp+0x26 	; (db)
%define bsVolumeSerial	bp+0x27 	; (dd)
%define bsVolumeLabel	bp+0x2B 	; (11)
%define bsSysID		bp+0x36 	; (8)

times 0x3B db 0				; Code starts at offset 0x3E

begin:
	mov bp, 0x7c00
	mov [bsDriveNumber], dl	; BIOS passes drive number in DL
	xor eax, eax
	xor esi, esi
	xor edi, edi
	mov ds, ax
	mov es, ax

; Make sure the screen is set to 80x25 color text mode
	mov ax, 0x0003			; Set to normal (80x25 text) video mode
	int 0x10

; Print message
	mov si, msg_Load
	call print_string_16

; read in the root cluster
; check for the filename
; if found save the starting cluster
; if not then error

;fatstart = bsResSectors

;rootcluster = bsResSectors + (bsFATs * bsSecsPerFat)
; 4 + (2 * 254) = sector 512

;datastart = bsResSectors + (bsFATs * bsSecsPerFat) + ((bsRootDirEnts * 32) / bsBytesPerSec) 
; 4 + (2 * 254) + ((512 * 32) / 512) = sector 544

;cluster X starting sector
; starting sector = (bsSecsPerClust * (cluster# - 2)) + datastart

; 0x7C5C as of Jan 6, 2010
getoffset:
	xor eax, eax
	mov bx, 0x8000
	call readsector		; Read the MBR to 0x8000
	mov eax, [0x81C6]	; Grab the dword at 0x01C6 (num of sectors between MBR and first sector in partition)
	mov [secoffset], eax	; Save it for later use
	xor eax, eax

ff:
	mov ax, [bsSecsPerFat]
	shl ax, 1	; quick multiply by two
	add ax, [bsResSectors]
	mov [rootstart], ax

	mov bx, [bsRootDirEnts]
	shr bx, 4	; bx = (bx * 32) / 512
	add bx, ax	; BX now holds the datastart sector number
	mov [datastart], bx
	
ff_next_sector:
	mov bx, 0x8000
	mov si, bx
	mov di, bx
	add eax, [secoffset]
	call readsector

; Search for file name, and find start cluster.
ff_next_entry:
	mov cx, 11
	mov si, loadername
	repe cmpsb
	jz ff_done		; note that di now is at dirent+11

	add di, byte 0x20
	and di, byte -0x20
	cmp di, [bsBytesPerSec]
	jnz ff_next_entry
	; load next sector
	dec dx			; next sector in cluster
	jnz ff_next_sector

ff_done:
	add di, 15
	mov ax, [di]	; AX now holds the starting cluster #

; At this point we have found the file we want and know the cluster where the file starts

	mov bx, 0x8000	; We want to load to 0x0000:0x8000
loadfile:	
	call readcluster
	cmp ax, 0xFFF8	; Have we reached the end cluster marker?
	jg loadfile	; If not then load another
	
	jmp 0x0000:0x8000

	
	
;------------------------------------------------------------------------------
; Read a sector from a disk, using LBA
; input:  EAX - 32-bit DOS sector number
;	  ES:BX - destination buffer
; output: ES:BX points one byte after the last byte read
;	  EAX - next sector
readsector:
	push dx
	push si
	push di

read_it:
	push eax	; Save the sector number
	mov di, sp	; remember parameter block end

	push byte 0	; other half of the 32 bits at [C]
	push byte 0	; [C] sector number high 32bit
	push eax	; [8] sector number low 32bit
	push es 	; [6] buffer segment
	push bx 	; [4] buffer offset
	push byte 1	; [2] 1 sector (word)
	push byte 16	; [0] size of parameter block (word)

	mov si, sp
	mov dl, [bsDriveNumber]
	mov ah, 42h	; EXTENDED READ
	int 0x13	; http://hdebruijn.soo.dto.tudelft.nl/newpage/interupt/out-0700.htm#0651

	mov sp, di	; remove parameter block from stack
	pop eax		; Restore the sector number

	jnc read_ok 	; jump if no error

	push ax
	xor ah, ah	; else, reset and retry
	int 0x13
	pop ax
	jmp read_it

read_ok:
	inc eax 		; next sector
	add bx, 512		; Add bytes per sector
	jnc no_incr_es		; if overflow...

incr_es:
	mov dx, es
	add dh, 0x10		; ...add 1000h to ES
	mov es, dx

no_incr_es:
	pop di
	pop si
	pop dx
	ret
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; Read a cluster from a disk partition, using LBA
; input:  AX - 16-bit cluster number
;	  ES:BX - destination buffer
; output: ES:BX points one byte after the last byte read
;	  AX - next cluster
readcluster:
	push cx
	mov [tcluster], ax		; save our cluster value
;cluster X starting sector
; starting sector = (bsSecsPerClust * (cluster# - 2)) + datastart
	xor cx, cx
	sub ax, 2
	mov cl, byte [bsSecsPerClust]
	imul cx				; EAX now holds starting sector
	add ax, word [datastart]	; add the datastart offset

	xor cx, cx
	mov cl, byte [bsSecsPerClust]
	add eax, [secoffset]
readcluster_nextsector:
	call readsector
	dec cx
	cmp cx, 0
	jne readcluster_nextsector

; Great! We read a cluster.. now find out where the next cluster is
	push bx			; save our memory pointer
	mov bx, 0x7E00		; load a sector from the root cluster here
	push bx
	mov ax, [bsResSectors]
	add eax, [secoffset]
	call readsector
	pop bx			; bx points to 0x7e00 again
	mov ax, [tcluster]	; ax holds the cluster # we just read
	shl ax, 1		; multipy by 2
	add bx, ax
	mov ax, [bx]
	
	pop bx			; restore our memory pointer
	pop cx
	
	ret
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; 16-bit Function to print a sting to the screen
; input: SI - Address of start of string
print_string_16:			; Output string in SI to screen
	pusha
	mov ah, 0x0E			; int 0x10 teletype function
.repeat:
	lodsb				; Get char from string
	cmp al, 0
	je .done			; If char is zero, end of string
	int 0x10			; Otherwise, print it
	jmp short .repeat
.done:
	popa
	ret
;------------------------------------------------------------------------------


msg_Load db "Loading... ", 0
msg_Error db "No "
loadername db "PURE64  SYS", 0
kernelname db "KERNEL64SYS", 0
datastart dw 0x0000
rootstart dw 0x0000
tcluster dw 0x0000
secoffset dd 0x00000000

times 510-$+$$ db 0

sign dw 0xAA55
