#include <asm/cpu.h>
#include <kernel/exception.h>
#include <asm/interrupt.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <drivers/serial.h>
#include <string.h>

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
    (void)f;
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

static void page_fault(interrupt_frame_t *f) {
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
    
	if (f->err_code & (1 << 0)) {  // bit 0이 1 -> Present
		// 현재는 구현 필요성이 낮음
		// 1. 유저 프로세스의 잘못된 주소 접근
		// 2. Copy-on-Write 페이지에 쓰기
		// 3. 커널 자체의 잘못된 접근
		while (1)
			hlt();
	} else {  // bit 0이 0 -> Not Present
		map_page(pml4_root, addr & ~0xFFF, (uint64_t)pmm_alloc(0), PAGE_RW);
		serial_write("[#PF] Page Allocated!\r\n");

		return;
	}
}

void exception_init(void)
{
    interrupt_register(0, divide_error);
    interrupt_register(3, breakpoint);
    interrupt_register(8, double_fault);
    interrupt_register(13, general_protection_fault);
    interrupt_register(14, page_fault);
}
