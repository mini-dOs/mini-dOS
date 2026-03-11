.intel_syntax noprefix
.code64

.global inb

inb:
	# dx(port: 2 bytes), di(first argument: 2 bytes)
	mov dx, di
	# read from dx(0x60) and store the data in al(1 byte)
	in al, dx

	ret
