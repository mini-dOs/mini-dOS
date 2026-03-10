#include <pic.h>
#include <idt.h>
#include <exception.h>
#include <irq.h>

void interrupt_subsystem_init(void)
{
    idt_init();
    pic_remap();
    exception_init();
    irq_init();
}