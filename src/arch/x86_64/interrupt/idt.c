#include <idt.h>
#include <stdint.h>

__attribute__((aligned(16))) idt_entry_t idt[IDT_SIZE];
idtr_t idtr;

extern void *isr_stub_table[256];	// Defined in src/arch/x86_64/idt/isr_table.c

static inline uint8_t idt_gate_type_for_vector(uint8_t vector)
{
    /* breakpoint(#BP, 3), overflow(#OF, 4) = trap gate */
    switch (vector) {
        case 3:
        case 4:
            return IDT_TRAP_GATE;
        default:
            return IDT_INTERRUPT_GATE;
    }
}

/* 
 * Initialize every entry in the IDT.
 * 
 * The x86 architecture defines the IDT with 256 possible vectors (0–255).
 * Even if only a subset of them are currently used (e.g., CPU exceptions or
 * hardware interrupts), the table itself must still contain 256 entries.
 * 
 * Therefore we iterate over the full IDT size and assign a stub handler
 * for each vector so that any unexpected interrupt will still have a valid
 * entry instead of causing undefined behavior.
 */
void idt_init(void){
    for (uint16_t i = 0; i < IDT_SIZE; i++) {
        idt_set_entry(
            (uint8_t)i, 
            isr_stub_table[i], 
            idt_gate_type_for_vector((uint8_t)i), 
            0
        );
    }

    idtr.base = (uint64_t)&idt;
    idtr.limit = sizeof(idt) - 1;

    idt_load(&idtr);
}

/* 
   127                                                                             96
 * --------------------------------------------------------------------------------
 * |                                                                               |
 * |                                Reserved                                       |
 * |                                                                               |
 *  --------------------------------------------------------------------------------
 * 95                                                                              64
 *  --------------------------------------------------------------------------------
 * |                                                                               |
 * |                               Offset 63..32                                   |
 * |                                                                               |
 *  --------------------------------------------------------------------------------
 * 63                               48 47      46  44   42    39             34    32
 *  --------------------------------------------------------------------------------
 * |                                  |       |  D  |   |     |      |   |   |     |
 * |       Offset 31..16              |   P   |  P  | 0 |Type |0 0 0 | 0 | 0 | IST |
 * |                                  |       |  L  |   |     |      |   |   |     |
 *  --------------------------------------------------------------------------------
 * 31                                   16 15                                      0
 *  --------------------------------------------------------------------------------
 * |                                      |                                        |
 * |          Segment Selector            |                 Offset 15..0           |
 * |                                      |                                        |
 *  --------------------------------------------------------------------------------
*/
void idt_set_entry(uint8_t vector, void* handler, uint8_t type_attr, uint8_t ist) {
    uint64_t handler_addr = (uint64_t)handler;

    /*   0x0001 0010 0011 0100 0101 0110 0111 1000   1001 1010 1011 1100 1101 1110 1111 0000
     * & 0x0000 0000 0000 0000 0000 0000 0000 0000   0000 0000 0000 0000 1111 1111 1111 1111
     *   0x0000 0000 0000 0000 0000 0000 0000 0000   0000 0000 0000 0000 0101 0110 0111 1000
     */
    idt[vector].offset_low = handler_addr & 0xFFFF; 
    idt[vector].selector = KERNEL_CS; // Kernel code segment selector
    idt[vector].ist = ist & 0x7; // 0000 0111 (only lower 3 bits are valid for IST index)
    idt[vector].type_attr = type_attr;
    
    /*   0x0000 0000 0000 0000 0001 0010 0011 0100   0101 0110 0111 1000 1001 1010 1011 1100
     * & 0x0000 0000 0000 0000 0000 0000 0000 0000   0000 0000 0000 0000 1111 1111 1111 1111
     *   0x0000 0000 0000 0000 0000 0000 0000 0000   0000 0000 0000 0000 1001 1010 1011 1100
     */
    idt[vector].offset_mid = (handler_addr >> 16) & 0xFFFF;

    /*   0x0000 0000 0000 0000 0000 0000 0000 0000   0001 0010 0011 0100 0101 0110 0111 1000
     * & 0x0000 0000 0000 0000 0000 0000 0000 0000   1111 1111 1111 1111 1111 1111 1111 1111
     *   0x0000 0000 0000 0000 0000 0000 0000 0000   0001 0010 0011 0100 0101 0110 0111 1000
     */
    idt[vector].offset_high = (handler_addr >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0; // Must be zero for 64-bit IDT entries
}