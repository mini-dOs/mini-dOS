#include <stdint.h>
#include <stddef.h>
#include <gdt.h>

extern void gdtr_load(uint64_t gdtr_address);
extern void tss_load(uint16_t tss_address);

struct GDT_ENTRY	gdt_entry[7];
struct TSS_ENTRY	tss_entry;
struct GDTR		gdtr;

/* Dedicated stack for #DF (vector 8) via IST1 */
#define DOUBLE_FAULT_STACK_SIZE 4096
static uint8_t double_fault_stack[DOUBLE_FAULT_STACK_SIZE] __attribute__((aligned(16)));

void gdt_init(void) {
	// Define 10 Bytes GDTR Structure
	gdtr.limit = (sizeof(struct GDT_ENTRY) * 7) - 1;	// 39 Bytes
	gdtr.base = (uint64_t)&gdt_entry;

	/*
	 * Define each Segment Descriptor
	 * The value of the 0th Segment Descriptor is always all 0.
	 * All Segment Descriptor have a base address of 0 and a segment limit of 0xFFFFFFFF.
	 */
	// GDT[0]
	gdt_set_entry(0, 0, 0, 0, 0);
	// GDT[1]
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xA0);
	// GDT[2]
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xC0);
	// GDT[3]
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xF2, 0xC0);
	// GDT[4]
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xFA, 0xA0);
	// GDT[5] == TSS[0]
	// GDT[6] == TSS[1]
	gdt_tss_descriptor_set_entry(5, (uint64_t)&tss_entry, sizeof(struct TSS_ENTRY) - 1, 0x89, 0x00);

	/* Zero TSS, then set IST1 = top of double_fault_stack.
	 * Order matters: tss_set_entry() must run before ist1 assignment.
	 * TSS descriptor (GDT[5:6]) already points to &tss_entry above,
	 * but the CPU reads IST1 only at interrupt time, so setting it
	 * before tss_load() is sufficient. */
	tss_set_entry(&tss_entry);
	tss_entry.ist1 = (uint64_t)(double_fault_stack + DOUBLE_FAULT_STACK_SIZE);


	// GDTR load and segment register update
	gdtr_load((uint64_t)&gdtr);

	// Register the TSS descriptor (GDT index 5) into the CPU's Task Register
	tss_load(0x28);	// The 5th Descriptor in GDT. 5 * 8(Bytes) = 40 = 0x28
}

void gdt_set_entry(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t attributes) {
	gdt_entry[idx].base_low    = (base & 0xFFFF);
	gdt_entry[idx].base_mid    = (base >> 16) & 0xFF;
	gdt_entry[idx].base_high   = (base >> 24) & 0xFF;

	gdt_entry[idx].limit_low   = (limit & 0xFFFF);
	gdt_entry[idx].attributes  = (limit >> 16) & 0x0F;	// limit_high

	gdt_entry[idx].attributes |= (attributes & 0xF0);
	gdt_entry[idx].access      = (access);
}

void tss_set_entry(struct TSS_ENTRY* tss_entry_ptr) {
	for (size_t i = 0; i < sizeof(struct TSS_ENTRY); i++) {
		((char*)tss_entry_ptr)[i] = 0;
	}
}

void gdt_tss_descriptor_set_entry(int idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t attributes) {
	gdt_entry[idx].base_low		= (base & 0xFFFF);
	gdt_entry[idx].base_mid		= (base >> 16) & 0xFF;
	gdt_entry[idx].base_high	= (base >> 24) & 0xFF;
	gdt_entry[idx + 1].limit_low	= (base >> 32) & 0xFFFF;
	gdt_entry[idx + 1].base_low	= (base >> 48) & 0xFFFF;

	gdt_entry[idx].limit_low	= (limit & 0xFFFF);
	gdt_entry[idx].attributes	= (limit >> 16) & 0x0F;

	gdt_entry[idx].attributes      |= (attributes & 0xF0);
	gdt_entry[idx].access		= (access);

	gdt_entry[idx + 1].base_mid	= 0;
	gdt_entry[idx + 1].access	= 0;
	gdt_entry[idx + 1].attributes	= 0;
	gdt_entry[idx + 1].base_high	= 0;
}
