#include <cpu.h>
#include <exception.h>
#include <serial.h>

static void divide_error(interrupt_frame_t *f) {
    serial_write("[#DE] Divide Error Exception\r\n");
    serial_write("RIP: ");
    serial_write_hex64(f->rip);
    serial_write("\r\n");

    while (1)
        hlt();
}

static void breakpoint(interrupt_frame_t *f)
{
    serial_write("#BP breakpoint\n");
}

static void double_fault(interrupt_frame_t *f)
{
    serial_write("[#DF] Double Fault\r\n");

    serial_write("RIP: ");
    serial_write_hex64(f->rip);
    serial_write("\r\n");

    serial_write("Error code: ");
    serial_write_hex64(f->err_code);
    serial_write("\r\n");

    serial_write("CS: ");
    serial_write_hex64(f->cs);
    serial_write("\r\n");

    serial_write("RFLAGS: ");
    serial_write_hex64(f->rflags);
    serial_write("\r\n");

    serial_write("System halted.\r\n");

    while (1)
        hlt();
}

static void general_protection_fault(interrupt_frame_t *f)
{
    serial_write("[#GP] General Protection Fault\r\n");

    serial_write("RIP: ");
    serial_write_hex64(f->rip);
    serial_write("\r\n");

    serial_write("Error code: ");
    serial_write_hex64(f->err_code);
    serial_write("\r\n");

    serial_write("CS: ");
    serial_write_hex64(f->cs);
    serial_write("\r\n");

    while (1)
        hlt();
}

static void page_fault(interrupt_frame_t *f)
{
    uint64_t addr;

    __asm__ volatile("mov %%cr2, %0" : "=r"(addr));

    serial_write("[#PF] Page Fault\r\n");
    serial_write("Fault address: ");
    serial_write_hex64(addr);
    serial_write("\r\n");

    serial_write("RIP: ");
    serial_write_hex64(f->rip);
    serial_write("\r\n");

    serial_write("Error code: ");
    serial_write_hex64(f->err_code);
    serial_write("\r\n");

    while (1)
        hlt();
}

void exception_init(void)
{
    interrupt_register(0, divide_error);
    interrupt_register(3, breakpoint);
    interrupt_register(8, double_fault);
    interrupt_register(13, general_protection_fault);
    interrupt_register(14, page_fault);
}