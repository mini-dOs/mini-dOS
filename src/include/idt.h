#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_SIZE 256

#define KERNEL_CS 0x08	// GDT index 1. 1 * 8(Bytes) = 8 = 0x08

#define IDT_INTERRUPT_GATE 0x8E	// P=1, DPL=00, S=0, Type=1110 (64-bit Interrupt Gate)
#define IDT_TRAP_GATE      0x8F // P=1, DPL=00, S=0, Type=1111 (64-bit Trap Gate)

typedef struct idt_entry
{
    uint16_t offset_low;   // Lower 16 bits of handler function address
    uint16_t selector;     // Code segment selector
    uint8_t  ist;          // Interrupt Stack Table offset
    uint8_t  type_attr;    // Type and attributes
    uint16_t offset_mid;   // Middle 16 bits of handler function address
    uint32_t offset_high;  // Higher 32 bits of handler function address
    uint32_t zero;         // Reserved, set to zero
} __attribute__((packed)) idt_entry_t;

typedef struct idtr
{
    uint16_t limit;        // Size of the IDT
    uint64_t base;         // Base address of the IDT
} __attribute__((packed)) idtr_t;

extern idt_entry_t idt[IDT_SIZE];
extern idtr_t idtr;

/* IDT setup */
void idt_init(void);
void idt_set_entry(uint8_t vector, void (*handler)(void));

/* assembly */
void idt_load(idtr_t* idtr);

#endif