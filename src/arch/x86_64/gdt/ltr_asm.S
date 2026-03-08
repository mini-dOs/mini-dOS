.intel_syntax noprefix
.code64

.global tss_load

.section .text

tss_load:
	ltr di

	ret

.section .note.GNU-stack,"",%progbits
