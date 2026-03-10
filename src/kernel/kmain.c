#include "pic.h"
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

static void bp_handler(interrupt_frame_t *f) {
    serial_write("[#BP] vector=");
    serial_write_hex64(f->vector);
    serial_write(" rip=");
    serial_write_hex64(f->rip);
    serial_write("\r\n");
}

/* Called from boot.s with Multiboot2 magic in RDI, info physical addr in RSI */
void kmain(uint32_t multiboot_magic, uint32_t multiboot_info) {
    (void)multiboot_info; /* use later for memory map, cmdline, etc. */

    gdt_init();
    pic_remap();
    idt_init();
    serial_init();
    
    serial_write("Hello from x86_64 kernel!\r\n");

    if (multiboot_magic == 0x36d76289) {
        serial_write("Multiboot2 magic OK.\r\n");
    }

    interrupt_register(3, bp_handler);

    __asm__ volatile("int3");
    serial_write("int3 return OK\r\n");

    while (1)
        __asm__ volatile("hlt");
}
