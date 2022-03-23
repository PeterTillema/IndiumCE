	section	.text
	public	__Z10is_pos_intf

__Z10is_pos_intf:
; Tests whether a given float is a positive integer. It works by first checking for negative values and infinity.
; After that, we exploit the fact how IEEE754 floats are stored. For every exponent, one more bit is allowed to set.
; That is because bit 0 from the mantissa = 1/2, bit 1 = 1/4. In order to be sure the float with mantissa 1/2 is an
; integer, the normalized exponent has to be at least 1 (2^1 * (1 + 1/2) = 3. A mantissa with the 1/4 bit set should
; have an exponent of at least 2: 2^2 * (1 + 1/4) = 5. This algorithm is translated to assembly by just shifting the
; bits of the mantissa, removing one at a time, meanwhile decreasing the exponent until it's zero. Right after that,
; the mantissa should be set to 0 (no other bits left), in which case it is an integer.
; Arguments:
;  arg0: Input number
; Returns:
;  True if input number is positive integer
	ld	iy, 0
	add	iy, sp
	ld	hl, (iy + 3)
	ld	a, (iy + 6)
	add	hl, hl		; Get the entire exponent
	rla
	jr	c, .false	; Return false when negative
	inc	a
	jr	z, .false	; Infinity
	sub	a, 128		; At least exponent 127 + 1 from the "inc a"
	jr	c, .false
	jr	z, .test
	cp	a, 23		; At most 23 bits of HL can be set
	jr	nc, .true
	ld	b, a
.loop:
	add	hl, hl		; The next bit is allowed to be set
	djnz	.loop
.test:
	add	hl, bc		; Now every bit of hl should be reset
	or	a, a
	sbc	hl, bc
	jr	nz, .false
.true:
	ld	a, 1
	ret
.false:
	xor	a,a
	ret
