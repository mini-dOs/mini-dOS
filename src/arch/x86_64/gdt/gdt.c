#include <stdint.h>
#include <stddef.h>
#include <gdt.h>

struct GDT_ENTRY	gdt_entry[7];	// GDT
struct TSS_ENTRY	tss_entry;	// TSS == *GDT[5:6]
struct GDTR		gdtr;		// GDTR == *GDT

static uint8_t fault_stack[4096] __attribute((aligned(16)));	// IST1
static uint8_t debug_stack[4096] __attribute((aligned(16)));	// IST2

extern void gdtr_load(uint64_t gdtr_address);
extern void tss_load(uint16_t tss_address);

void init_arch_tables(void) {
	// Define 10 Bytes GDTR Structure
	gdtr.limit = (sizeof(struct GDT_ENTRY) * 7) - 1;	// 39 Bytes
	gdtr.base = (uint64_t)&gdt_entry;

	setup_gdt();
	setup_tss();
	load_cpu_registers();
}

void setup_gdt(void) {
	/*
	 * Define each Segment Descriptor
	 * The value of the 0th Segment Descriptor is always all 0.
	 * All Segment Descriptor have a base address of 0 and a segment limit of 0xFFFFFFFF.
	 */

	// GDT[0]
	gdt_set_entry(0, 0, 0, 0 ,0);
	// GDT[1]
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0xA0, 0x9A);
	// GDT[2]
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0xC0, 0x92);
	// GDT[3]
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xC0, 0xF2);
	// GDT[4]
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xA0, 0xFA);
}

void setup_tss(void) {
	// GDT[5:6] (TSS Descriptor)
	tss_set_entry(5, (uint64_t)&tss_entry, sizeof(struct TSS_ENTRY) - 1, 0x00, 0x89);

	// Initialize TSS Structure
	for (size_t i = 0; i < sizeof(struct TSS_ENTRY); i++) {
		((char*)&tss_entry)[i] = 0;
	}

	// Fill real stack address into TSS Structure's Entries
	tss_entry.ist1 = (uint64_t)fault_stack + sizeof(fault_stack);
	tss_entry.ist2 = (uint64_t)debug_stack + sizeof(debug_stack);
}

void load_cpu_registers(void) {
	// GDTR load and segment register update
	gdtr_load((uint64_t)&gdtr);

	// Register the TSS descriptor (GDT index 5) into the CPU's Task Register
	tss_load(0x28);	// The 5th Descriptor in GDT. 5 * 8(Bytes) = 40 = 0x28
}

void gdt_set_entry(int idx, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access) {
	gdt_entry[idx].base_low    = (base & 0xFFFF);
	gdt_entry[idx].base_mid    = (base >> 16) & 0xFF;
	gdt_entry[idx].base_high   = (base >> 24) & 0xFF;

	gdt_entry[idx].limit_low   = (limit & 0xFFFF);
	gdt_entry[idx].attributes  = (limit >> 16) & 0x0F;	// limit_high

	gdt_entry[idx].attributes |= (flags & 0xF0);
	gdt_entry[idx].access      = (access);
}

void tss_set_entry(int idx, uint64_t base, uint32_t limit, uint8_t flags, uint8_t access) {
	gdt_entry[idx].base_low		= (base & 0xFFFF);
	gdt_entry[idx].base_mid		= (base >> 16) & 0xFF;
	gdt_entry[idx].base_high	= (base >> 24) & 0xFF;
	gdt_entry[idx + 1].limit_low	= (base >> 32) & 0xFFFF;
	gdt_entry[idx + 1].base_low	= (base >> 48) & 0xFFFF;

	gdt_entry[idx].limit_low	= (limit & 0xFFFF);
	gdt_entry[idx].attributes	= (limit >> 16) & 0x0F;

	gdt_entry[idx].attributes      |= (flags & 0xF0);
	gdt_entry[idx].access		= (access);

	gdt_entry[idx + 1].base_mid	= 0;
	gdt_entry[idx + 1].access	= 0;
	gdt_entry[idx + 1].attributes	= 0;
	gdt_entry[idx + 1].base_high	= 0;
}
