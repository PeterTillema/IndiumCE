	section	.text
	public	__Z13roundf_customf

__Z13roundf_customf:
	push	ix
	ld	ix, 0
	add	ix, sp
	ld	iy, (ix + 6)		; IY = mantissa + 1 bit of the exponent
	ld	l, (ix + 9)		; L = sign + 7 bits of exponent
	add	iy, iy			; Add bit from IY to HL
	adc	hl, hl			; HL = sign + exponent
	ld	a, l			; A = exponent
	ld	b, 24			; B = remaining bits to shift euhl
	sub	a, 126			; Check again 2^-1
	jr	c, .output0		; [0, 0.5)
	jr	z, .output1		; [0.5, 1)
	cp	a, b			; If the exponent is > 2^23, the mantissa doesn't matter anymore
	jr	nc, .is_rounded		; This is also true for Inf and NAN, in which case it returns the same
.loop:
	dec	b			; Check the next bit
	dec	a
	jr	z, .test
	add	iy, iy			; Move bit from IY to EUHL
	adc	hl, hl
	rl	e
	jr	.loop
.test:
; HL is guaranteed to contain the integer <= num.
; If the next bit is set, we need to increase HL to get the next integer > num
	add	iy, iy
	jr	nc, .rounded_down
	inc	hl			; This is guaranteed it will never overflow, since 2^22 (149) is the max
.rounded_down:				; exponent, which sets HL to 0x7FFFFF at most.
	add	hl, hl			; Now EUHL contains the right upper bits of the output num
	rl	e			; Shift left until enough zero bits are appended
	djnz	.rounded_down
	jr	.ret
.is_rounded:
	ld	hl, (ix + 6)
	ld	e, (ix + 9)
	jr	.ret
.output0:
	xor	a, a
	rr	h
	rra
	ld	e, a
	sbc	hl, hl
	jr	.ret
.output1:
	ld	a, 0x3F shl 1
	rr	h
	rra
	ld	e, a
	ld	hl, 0x800000
.ret:
	ld	sp, ix
	pop	ix
	ret