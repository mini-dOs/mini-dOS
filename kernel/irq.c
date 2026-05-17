#include <asm/interrupt.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <kernel/pit.h>

static void timer_handler(interrupt_frame_t *f)
{
    (void)f;
    serial_write(".");
}

void irq_init(void)
{
    pit_init(100);

    interrupt_register(32, timer_handler);
    interrupt_register(33, keyboard_handler);
}
