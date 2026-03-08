#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

/*
 * interrupt_frame_t — stack layout built by isr_stub.S + isr_common_stub.S
 *
 * Stack at entry to isr_common_stub (rsp → vector):
 *   rsp+ 0  vector       pushed by ISR_NOERR/ISR_ERR stub
 *   rsp+ 8  error_code   0 (NOERR) or CPU-pushed (ERR)
 *   rsp+16  rip          \
 *   rsp+24  cs            > CPU iretq frame (same-privilege / kernel-only)
 *   rsp+32  rflags       /
 *
 * NOTE: rsp and ss are NOT present because all interrupts are handled
 * at ring 0 → ring 0 (same privilege).  iretq handles this correctly.
 *
 * isr_common_stub pushes 15 GPRs (rax first → r15 last = new rsp).
 * Struct member offset 0 maps to the last-pushed register (r15).
 */
typedef struct interrupt_frame
{
    /* isr_common_stub: pushed last→first (r15 at lowest addr = rsp) */
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;           /* offset 112 */

    /* ISR stub */
    uint64_t vector;        /* offset 120 */
    uint64_t error_code;    /* offset 128 */

    /* CPU iretq frame (kernel-only: same privilege, no rsp/ss) */
    uint64_t rip;           /* offset 136 */
    uint64_t cs;            /* offset 144 */
    uint64_t rflags;        /* offset 152 */

} interrupt_frame_t;

typedef void (*interrupt_handler_t)(interrupt_frame_t *);

void register_interrupt_handler(int vector, interrupt_handler_t handler);
void interrupt_dispatcher(interrupt_frame_t *frame);
void exception_dispatch(interrupt_frame_t *frame);
void divide_by_zero_handler(interrupt_frame_t *frame);

#endif