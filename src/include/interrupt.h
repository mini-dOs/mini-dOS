#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>
#include <serial.h>

typedef struct interrupt_frame
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    uint64_t vector;
    uint64_t err_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;

    uint64_t rsp;
    uint64_t ss;
} interrupt_frame_t;

typedef void (*interrupt_handler_t)(interrupt_frame_t *);

void interrupt_register(uint8_t vector, interrupt_handler_t handler);
void interrupt_dispatch(interrupt_frame_t *frame);

#endif