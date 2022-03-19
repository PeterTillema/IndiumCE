	section	.text
	public	__Z14in_degree_modev
	public	__Z10get_fix_nrv
	public	__Z7get_keyv
	public	__Z21get_norm_sci_end_modev

__Z14in_degree_modev:
	ld	iy, 0xD00080 ; flags
	xor	a, a
	bit	2, (iy)
	jr	z, .skip
	inc	a
.skip:
	ret

__Z10get_fix_nrv:
	ld	a, (0xD0250F) ; fmtDigits
	ret

__Z21get_norm_sci_end_modev:
	ld	a, (0xD0008A) ; fmtFlags
	and	a, 3
	ret

__Z7get_keyv:
	ld	hl, 0xF50012
	ld	de, basic_keys - 1
	ld	b, 7
.loop:
	ld	a, (hl)
	ld	c, b
	ld	b, 8
.loop2:
	add	a, a
	inc	de
	jr	c, .get_key
.next_key:
	djnz	.loop2
	ld	b, c
	inc	hl
	inc	hl
	djnz	.loop
.multiple_keys:
	xor	a, a
	ret
.get_key:
	ld	iyh, a
	ld	a, iyl
	or	a, a
	jr	nz, .multiple_keys
	ld	a, (de)
	ld	iyl, a
	jr	.next_key

basic_keys:
	db	23, 22, 21, 11, 12, 13, 14, 15
	db	31, 41, 51, 61, 71, 81, 91, 0
	db	32, 42, 52, 62, 72, 82, 92, 102
	db	33, 43, 53, 63, 73, 83, 93, 103
	db	0,  44, 54, 64, 74, 84, 94, 104
	db	0,  45, 55, 65, 75, 85, 95, 105
	db	0,  0,  0,  0,  34, 24, 26, 25
