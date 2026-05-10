#include <config.h>
#include <cpu.h>
#include <early_alloc.h>
#include <framebuffer.h>
#include <gdt.h>
#include <idt.h>
#include <interrupt_init.h>
#include <mini_shell.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/slab.h>
#include <mm/vmalloc.h>
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

    vmm_init();
    serial_write("VMM initialized\r\n");

    pmm_init_step2();

    kmem_cache_init();
    vmalloc_init();
    serial_write("VMALLOC initialized\r\n");

    // ── vmalloc smoke test ─────────────────────────────
    serial_write("\r\n[vmalloc test] start\r\n");
    vmalloc_dump();

    void *a = vmalloc(0x1000);   // 4KB
    void *b = vmalloc(0x4000);   // 16KB
    void *c = vmalloc(0x1000);   // 4KB
    serial_write("a="); serial_write_hex64((uint64_t)a); serial_write("\r\n");
    serial_write("b="); serial_write_hex64((uint64_t)b); serial_write("\r\n");
    serial_write("c="); serial_write_hex64((uint64_t)c); serial_write("\r\n");
    vmalloc_dump();

    // 실제 매핑 동작 확인 (#PF 안 나야 정상)
    *(volatile uint64_t *)a = 0xCAFEBABEULL;
    *(volatile uint64_t *)b = 0xDEADBEEFULL;
    serial_write("write OK\r\n");

    // 가운데 free → 같은 자리 재사용 확인
    vfree(b);
    serial_write("after vfree(b)\r\n");
    vmalloc_dump();

    void *d = vmalloc(0x4000);
    serial_write("d="); serial_write_hex64((uint64_t)d); serial_write("\r\n");
    if (d == b) serial_write("first-fit OK (d == b)\r\n");
    else        serial_write("first-fit FAIL\r\n");

    // 모두 해제 → 노드 1개로 회복
    vfree(a);
    vfree(c);
    vfree(d);
    serial_write("after all vfree\r\n");
    vmalloc_dump();
    serial_write("[vmalloc test] end\r\n\r\n");
    // ────────────────────────────────────────────────────

    interrupt_subsystem_init();
    fb_init();		// GOP framebuffer
    fb_clear(0x001E1E1E);
    fb_write(170, 270, "Hello mini-dOS!", 0x00B4B4B4, 0x001E1E1E, 4);

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

    shell_init();

    while (1)
      shell_wait();
}
