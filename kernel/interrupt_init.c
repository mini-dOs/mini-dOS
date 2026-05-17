#include <asm/idt.h>
#include <asm/irq.h>
#include <asm/pic.h>
#include <drivers/i8042.h>
#include <kernel/exception.h>

void interrupt_subsystem_init(void)
{
    pic_remap();
    // Mask all IRQs
    for (int i = 0; i < 16; i++)
        pic_mask_irq(i);

    i8042_init();

    idt_init();
    exception_init();
    irq_init();

    pic_unmask_irq(0);   // timer
    pic_unmask_irq(1);   // keyboard
}
