#include <interrupt.h>
#include <serial.h>
#include <pit.h>


static void timer_handler(interrupt_frame_t *f)
{
    serial_write(".");
}

void irq_init(void)
{
    pit_init(100);
    interrupt_register(32, timer_handler);
}