#include <config.h>
#include <cpu.h>
#include <early_alloc.h>
#include <framebuffer.h>
#include <gdt.h>
#include <idt.h>
#include <interrupt_init.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/slab.h>
#include <multiboot.h>
#include <serial.h>
#include <stdint.h>

/* Called from boot.s with Multiboot2 magic in RDI, info physical addr in RSI */
void kmain(uint32_t multiboot_magic, uint32_t multiboot_info) {
    serial_init();
    serial_write("Kernel start\r\n");
    
    arch_tables_init();

    if (multiboot_magic != 0x36d76289) {
        serial_write("Invalid Multiboot magic\r\n");
        while (1) hlt();
    }
    serial_write("Multiboot2 magic OK\r\n");

    early_alloc_init();
    
    multiboot_parse((void *)(uint64_t)multiboot_info);
    serial_write("Memory parsing done\r\n");
    
    pmm_init_step1();

    paging_init();
    serial_write("Paging initialized\r\n");

    pmm_init_step2();

    interrupt_subsystem_init();
    framebuffer_init();		// VESA/VBE framebuffer
    serial_write("framebuffer_addr: ");
    serial_write_hex64(fb_info.framebuffer_addr);
    serial_write("\r\n");
    framebuffer_clear(0x00FFFFFF);

    // Turn on the Interrupt Switch of CPU
    sti();

    serial_write("Interrupts enabled\r\n");

    static volatile uint64_t mapped_probe = 0x1122334455667788ULL;
    serial_write("[Test] mapped access start\r\n");
    mapped_probe ^= 0x55AA55AA55AA55AAULL;
    serial_write("[Test] mapped value = ");
    serial_write_hex64(mapped_probe);
    serial_write("\r\n");

    // serial_write("[Test] trigger page fault\r\n");
    // volatile uint64_t *bad = (volatile uint64_t *)0x40000000ULL; // 1GiB
    // *bad = 0xDEADBEEFCAFEBABEULL; // 의도적 #PF (err_code=0x2)
    // serial_write("[TEST] unreachable\r\n");

    while (1)
        hlt();
}
