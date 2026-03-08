.intel_syntax noprefix
.code64

.global gdtr_load

.section .text

gdtr_load:
	lgdt [rdi]

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	push 0x08

	lea rax, [rip + .reload_cs]
	push rax

	retfq

.reload_cs:
	ret

.section .note.GNU-stack,"",%progbits
