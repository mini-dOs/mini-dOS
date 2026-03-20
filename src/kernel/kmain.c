#include <early_alloc.h>
#include <cpu.h>
#include <stdint.h>
#include <gdt.h>
#include <idt.h>
#include <interrupt_init.h>
#include <serial.h>
#include <multiboot.h>
#include <mm/paging.h>

/* Called from boot.s with Multiboot2 magic in RDI, info physical addr in RSI */
void kmain(uint32_t multiboot_magic, uint32_t multiboot_info) {
    init_arch_tables();
    serial_init();

    serial_write("Kernel start\r\n");

    if (multiboot_magic != 0x36d76289) {
        serial_write("Invalid Multiboot magic\r\n");
        while (1) hlt();
    }
    serial_write("Multiboot2 magic OK\r\n");

    init_early_alloc();
    multiboot_parse((void *)(uint64_t)multiboot_info);

    serial_write("Memory parsing done\r\n");

    paging_init();

    serial_write("Paging initialized\r\n");

    init_interrupt_subsystem();

    // Turn on the Interrupt Switch of CPU
    sti();

    serial_write("Interrupts enabled\r\n");

    while (1)
        hlt();
}
