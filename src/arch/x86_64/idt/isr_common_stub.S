.intel_syntax noprefix

.extern interrupt_dispatcher

.global isr_common_stub
isr_common_stub:

    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi

    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call interrupt_dispatcher

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8

    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16

    iretq