#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Segment Descriptor Structure
struct gdt_entry {
	uint16_t	limit_low;
	uint16_t	base_low;
	uint8_t		base_mid;
	uint8_t		access;
	uint8_t		attributes;
	uint8_t		base_high;
} __attribute__((packed));

// GDTR Structure
struct gdt_ptr {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed));

void gdtr_init(void);
void gdt_set_entry(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t attributes);
void gdtr_load(uint64_t gdtr_address);

#endif
