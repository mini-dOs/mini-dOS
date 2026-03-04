#include "gdt.h"

extern void gdtr_load(uint64_t gdtr_address);

struct gdt_entry gdt[5];
struct gdt_ptr gdtr;

void gdtr_init(void) {
	// Define 10 Bytes GDTR Structure
	gdtr.limit = (sizeof(struct gdt_entry) * 5) - 1;	// 39 Bytes
	gdtr.base = (uint64_t)&gdt;

	/*
	 * Define each Segment Descriptor
	 * The value of the 0th Segment Descriptor is always all 0.
	 * All Segment Descriptor have a base address of 0 and a segment limit of 0xFFFFFFFF.
	 */
	// GDT[0]
	gdt_set_entry(0, 0, 0, 0, 0);
	// GDT[1]
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0xA0, 0x9A);
	// GDT[2]
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0xC0, 0x92);
	// GDT[3]
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xC0, 0xF2);
	// GDT[4]
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xA0, 0xFA);

	// GDTR load and segment register update
	gdtr_load((uint64_t)&gdtr);
}

void gdt_set_entry(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t attributes) {
	gdt[idx].base_low    = (base & 0xFFFF);
	gdt[idx].base_mid    = (base >> 16) & 0xFF;
	gdt[idx].base_high   = (base >> 24) & 0xFF;

	gdt[idx].limit_low   = (limit & 0xFFFF);
	gdt[idx].attributes  = (limit >> 16) & 0x0F;	// limit_high

	gdt[idx].attributes |= (attributes & 0xF0);
	gdt[idx].access      = (access);
}
