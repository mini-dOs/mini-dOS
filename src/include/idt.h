#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_SIZE 256

/* GDT selectors */
#define KERNEL_CS 0x08

#define IDT_INTERRUPT_GATE  0x8E // P=1, DPL=00, S=0, Type=1110 (64-bit Interrupt Gate) 
#define IDT_TRAP_GATE       0x8F // P=1, DPL=00, S=0, Type=1111 (64-bit Trap Gate)

/* 64-bit IDT Entry (16 bytes) */
typedef struct idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;        // bits 0~2: IST index
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) idt_entry_t;

/* IDTR register structure */
typedef struct idtr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;
    
/* Global IDT table */
extern idt_entry_t idt[IDT_SIZE];
extern idtr_t idtr;

/* IDT initialization */
void idt_init(void);

/* Set an IDT entry */
void idt_set_entry(uint8_t vector, void* handler, uint8_t type_attr, uint8_t ist);

/* Assembly: load IDTR */
void idt_load(idtr_t* idtr);

#endif