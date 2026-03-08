#include <idt.h>
#include <stdint.h>

idt_entry_t idt[IDT_SIZE];
idtr_t idtr;

extern void* isr_stub_table[];	// Defined in src/arch/x86_64/idt/isr_stubs.S

void idt_set_entry(uint8_t vector, void (*handler)(void))
{
    uint64_t addr = (uint64_t)handler;

    idt[vector].offset_low  = addr & 0xFFFF;
    idt[vector].selector    = KERNEL_CS;
    idt[vector].ist         = 0;
    idt[vector].type_attr   = IDT_INTERRUPT_GATE;

    idt[vector].offset_mid  = (addr >> 16) & 0xFFFF;
    idt[vector].offset_high = (addr >> 32) & 0xFFFFFFFF;

    idt[vector].zero = 0;
}

void idt_init(void)
{
    for (int i = 0; i < IDT_SIZE; i++)
    {
        idt_set_entry(i, isr_stub_table[i]);
    }

    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint64_t)&idt;

    idt_load(&idtr);
}