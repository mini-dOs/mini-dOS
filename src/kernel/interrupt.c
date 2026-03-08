#include <interrupt.h>
#include <stdint.h>
#include <serial.h>

#define MAX_INTERRUPTS 256

static interrupt_handler_t handlers[MAX_INTERRUPTS];

static const char *exception_names[32] = {
    "#DE Divide Error",
    "#DB Debug",
    "#NMI Non-Maskable Interrupt",
    "#BP Breakpoint",
    "#OF Overflow",
    "#BR Bound Range Exceeded",
    "#UD Invalid Opcode",
    "#NM Device Not Available",
    "#DF Double Fault",
    "Coprocessor Segment Overrun",
    "#TS Invalid TSS",
    "#NP Segment Not Present",
    "#SS Stack Segment Fault",
    "#GP General Protection Fault",
    "#PF Page Fault",
    "Reserved (15)",
    "#MF x87 FPU Error",
    "#AC Alignment Check",
    "#MC Machine Check",
    "#XF SIMD FP Exception",
    "#VE Virtualization Exception",
    "#CP Control Protection",
    "Reserved (22)",
    "Reserved (23)",
    "Reserved (24)",
    "Reserved (25)",
    "Reserved (26)",
    "Reserved (27)",
    "#HV Hypervisor Injection",
    "#VC VMM Communication",
    "#SX Security Exception",
    "Reserved (31)",
};

void exception_dispatch(interrupt_frame_t *frame)
{
    const char *name = (frame->vector < 32) ? exception_names[frame->vector] : "Unknown";

    serial_write("\r\n[EXCEPTION] ");
    serial_write(name);
    serial_write("\r\n");
    serial_write("Vector:     "); serial_write_hex64(frame->vector);     serial_write("\r\n");
    serial_write("Error Code: "); serial_write_hex64(frame->error_code); serial_write("\r\n");
    serial_write("RIP:        "); serial_write_hex64(frame->rip);        serial_write("\r\n");
    serial_write("CS:         "); serial_write_hex64(frame->cs);         serial_write("\r\n");
    serial_write("RFLAGS:     "); serial_write_hex64(frame->rflags);     serial_write("\r\n");

    while (1)
        __asm__ volatile("hlt");
}

void register_interrupt_handler(int vector, interrupt_handler_t handler)
{
    handlers[vector] = handler;
}

void interrupt_dispatcher(interrupt_frame_t *frame)
{
    if (handlers[frame->vector])
        handlers[frame->vector](frame);
    else if (frame->vector < 32)
        exception_dispatch(frame);
}

void divide_by_zero_handler(interrupt_frame_t *frame)
{
    (void)frame;
    serial_write("\r\n==========================================\r\n");
    serial_write("EXCEPTION: Divide by Zero (#DE)\r\n");
    serial_write("==========================================\r\n");
    serial_write("Vector:      "); serial_write_hex64(frame->vector); serial_write("\r\n");
    serial_write("Error Code:  "); serial_write_hex64(frame->error_code); serial_write("\r\n");
    serial_write("\r\n");
    serial_write("RIP:         "); serial_write_hex64(frame->rip); serial_write("\r\n");
    serial_write("CS:          "); serial_write_hex64(frame->cs); serial_write("\r\n");
    serial_write("RFLAGS:      "); serial_write_hex64(frame->rflags); serial_write("\r\n");
    serial_write("\r\n");
    serial_write("RAX:         "); serial_write_hex64(frame->rax); serial_write("\r\n");
    serial_write("RBX:         "); serial_write_hex64(frame->rbx); serial_write("\r\n");
    serial_write("RCX:         "); serial_write_hex64(frame->rcx); serial_write("\r\n");
    serial_write("RDX:         "); serial_write_hex64(frame->rdx); serial_write("\r\n");
    serial_write("RSI:         "); serial_write_hex64(frame->rsi); serial_write("\r\n");
    serial_write("RDI:         "); serial_write_hex64(frame->rdi); serial_write("\r\n");
    serial_write("RBP:         "); serial_write_hex64(frame->rbp); serial_write("\r\n");
    serial_write("R8:          "); serial_write_hex64(frame->r8); serial_write("\r\n");
    serial_write("R9:          "); serial_write_hex64(frame->r9); serial_write("\r\n");
    serial_write("R10:         "); serial_write_hex64(frame->r10); serial_write("\r\n");
    serial_write("R11:         "); serial_write_hex64(frame->r11); serial_write("\r\n");
    serial_write("R12:         "); serial_write_hex64(frame->r12); serial_write("\r\n");
    serial_write("R13:         "); serial_write_hex64(frame->r13); serial_write("\r\n");
    serial_write("R14:         "); serial_write_hex64(frame->r14); serial_write("\r\n");
    serial_write("R15:         "); serial_write_hex64(frame->r15); serial_write("\r\n");
    serial_write("==========================================\r\n");
    serial_write("System halted.\r\n");
    while (1)
        __asm__ volatile("hlt");
}
