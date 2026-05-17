#ifndef PIC_H
#define PIC_H

#include <asm/cpu.h>
#include <stdint.h>

void pic_remap(void);
void pic_send_eoi(unsigned char irq);
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

#endif
