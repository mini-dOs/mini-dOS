#include <pic.h>
#include <idt.h>
#include <exception.h>
#include <irq.h>
#include <i8042.h>

void init_interrupt_subsystem(void)
{
    pic_remap();
    // Mask all IRQs
    for (int i = 0; i < 16; i++)
        pic_mask_irq(i);

    pic_unmask_irq(0);   // timer
    pic_unmask_irq(1);   // keyboard

    idt_init();
    exception_init();
    irq_init();

    i8042_init();
}
