#include <isr.h>
#include <interrupt.h>
#include <stdint.h>

void isr_handler(interrupt_frame_t *frame)
{
    interrupt_dispatch(frame);
}