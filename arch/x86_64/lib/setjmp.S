// x86_64 setjmp / longjmp (freestanding, 시그널 마스크 없음)
//
// jmp_buf 레이아웃 (libc/include/setjmp.h):
//   0:rbx 8:rbp 16:r12 24:r13 32:r14 40:r15 48:rsp 56:rip
//
// SysV AMD64 호출 규약: 1번째 인자=%rdi, 2번째=%esi, 반환값=%eax.
// 콜리-세이브 레지스터(rbx,rbp,r12-r15)와 호출자 복귀 지점(rsp,rip)만
// 저장하면 longjmp가 setjmp 호출 직후 상태로 스택을 되감을 수 있다.

	.text

	.globl setjmp
	.type  setjmp, @function
setjmp:
	movq %rbx,  0(%rdi)
	movq %rbp,  8(%rdi)
	movq %r12, 16(%rdi)
	movq %r13, 24(%rdi)
	movq %r14, 32(%rdi)
	movq %r15, 40(%rdi)
	leaq 8(%rsp), %rax		// ret 후의 rsp (복귀 주소 pop된 상태)
	movq %rax, 48(%rdi)
	movq (%rsp), %rax		// 복귀 주소 = setjmp 호출 다음 명령
	movq %rax, 56(%rdi)
	xorl %eax, %eax			// 직접 호출 시 0 반환
	ret
	.size setjmp, .-setjmp

	.globl longjmp
	.type  longjmp, @function
longjmp:
	movq  0(%rdi), %rbx
	movq  8(%rdi), %rbp
	movq 16(%rdi), %r12
	movq 24(%rdi), %r13
	movq 32(%rdi), %r14
	movq 40(%rdi), %r15
	movq 48(%rdi), %rsp
	movl %esi, %eax			// longjmp의 val을 setjmp 반환값으로
	testl %eax, %eax
	jnz 1f
	incl %eax			// val==0 이면 1로 보정 (C 표준)
1:
	movq 56(%rdi), %rdx
	jmp *%rdx			// setjmp 호출 지점으로 복귀
	.size longjmp, .-longjmp

	.section .note.GNU-stack,"",@progbits
