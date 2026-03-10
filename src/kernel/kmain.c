#include <cpu.h>
#include <stdint.h>
#include <gdt.h>
#include <idt.h>
#include <interrupt_init.h>
#include <serial.h>

/* Called from boot.s with Multiboot2 magic in RDI, info physical addr in RSI */
void kmain(uint32_t multiboot_magic, uint32_t multiboot_info) {
    (void)multiboot_info; /* use later for memory map, cmdline, etc. */

    gdt_init();
    serial_init();
    interrupt_subsystem_init();
    sti();
    
    serial_write("Hello from x86_64 kernel!\r\n");

    if (multiboot_magic == 0x36d76289) {
        serial_write("Multiboot2 magic OK.\r\n");
    }


    serial_write("Interrupts enabled\r\n");

    while (1)
        hlt();
}
