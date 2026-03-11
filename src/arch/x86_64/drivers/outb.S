.intel_syntax noprefix
.code64

.global outb

outb:
	# dx(port: 2 bytes), di(first argument: 2 byte)
	mov dx, di

	# al(data: 1 byte), sil(second argument: 1 byte)
	mov al, sil

	out dx, al

	ret
