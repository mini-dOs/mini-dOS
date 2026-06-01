#include <asm/interrupt.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <kernel/pit.h>

static void timer_handler(interrupt_frame_t *f) {
	(void)f;
	// serial_write("."); 제거
	pit_tick();
}

void irq_init(void) {
	pit_init(1000);
	/* 100 -> 1000으로 변경
	 * 1000으로 변경할 경우 timer_handler의 serial_write가
	 * 부하를 일으킬 것으로 예상되어 serial_write 제거
	 */

	interrupt_register(32, timer_handler);
	interrupt_register(33, keyboard_handler);
}
