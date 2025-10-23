; =============================================================================
; Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
; Copyright (C) 2008-2025 Return Infinity -- see LICENSE.TXT
;
; INIT PIT
; =============================================================================


init_pit:
	; Enable PIT
	; Base rate is 1193182 Hz
	mov al, 0x36			; Channel 0 (7:6), Access Mode lo/hi (5:4), Mode 3 (3:1), Binary (0)
	out 0x43, al
	mov al, 0x3C			; 60
	out 0x40, al			; Set low byte of PIT reload value
	mov al, 0x00
	out 0x40, al			; Set high byte of PIT reload value
	; New rate is 19866 Hz (1193182 / 60)
	; 0.050286633812733 milliseconds (ms)
	; 50.28663381273300814 microseconds (us)
	ret


; =============================================================================
; EOF