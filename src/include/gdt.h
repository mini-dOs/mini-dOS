#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include <stddef.h>

// Segment Descriptor Structure
struct GDT_ENTRY {
	uint16_t	limit_low;
	uint16_t	base_low;
	uint8_t		base_mid;
	uint8_t		access;
	uint8_t		attributes;
	uint8_t		base_high;
} __attribute__((packed));

struct TSS_ENTRY {
	uint32_t reserved1;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved2;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved3;
	uint16_t reserved4;
	uint16_t iopb;
} __attribute__((packed));

// GDTR Structure
struct GDTR {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed));

void arch_tables_init(void);
void setup_gdt(void);
void setup_tss(void);
void load_cpu_registers(void);
void gdt_set_entry(int idx, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access);
void tss_set_entry(int idx, uint64_t base, uint32_t limit, uint8_t flags, uint8_t access);
extern void gdtr_load(uint64_t gdtr_address);
extern void tss_load(uint16_t tss_address);

#endif
