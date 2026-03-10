#include <exception.h>
#include <serial.h>

static void divide_error(interrupt_frame_t *f) {
    serial_write("[#DE] Divide Error Exception\r\n");
    serial_write("RIP: ");
    serial_write_hex64(f->rip);
    serial_write("\r\n");

    while (1)
        __asm__ volatile("hlt");
}

static void breakpoint(interrupt_frame_t *f)
{
    serial_write("#BP breakpoint\n");
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
        __asm__ volatile("hlt");
}

void exception_init(void)
{
    interrupt_register(0, divide_error);
    interrupt_register(3, breakpoint);
    interrupt_register(14, page_fault);
}