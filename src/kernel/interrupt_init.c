#include <pic.h>
#include <idt.h>
#include <exception.h>
#include <irq.h>

void interrupt_subsystem_init(void)
{
    pic_remap();
    // Mask all IRQs
    for (int i = 0; i < 16; i++)
        pic_mask_irq(i);

    pic_unmask_irq(0);   // timer

    idt_init();
    exception_init();
    irq_init();
}