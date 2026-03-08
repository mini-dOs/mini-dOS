#include <stdint.h>
#include <gdt.h>
#include <idt.h>
#include <interrupt.h>
#include <serial.h>

static void trigger_divide_by_zero(void) {
    __asm__ volatile(
        "mov $1, %%rax\n\t"
        "xor %%rdx, %%rdx\n\t"
        "xor %%rcx, %%rcx\n\t"
        "div %%rcx\n\t"
        :
        :
        : "rax", "rdx", "rcx"
    );
}

/* Called from boot.s with Multiboot2 magic in RDI, info physical addr in RSI */
void kmain(uint32_t multiboot_magic, uint32_t multiboot_info) {
    (void)multiboot_info; /* use later for memory map, cmdline, etc. */

    gdt_init();
    idt_init();
    serial_init();
    
    serial_write("Hello from x86_64 kernel!\r\n");

    if (multiboot_magic == 0x36d76289) {
        serial_write("Multiboot2 magic OK.\r\n");
    }

    register_interrupt_handler(0, divide_by_zero_handler);

    trigger_divide_by_zero();

    while (1)
        __asm__ volatile("hlt");
}
