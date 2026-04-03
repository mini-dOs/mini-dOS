#include <kernel_base.h>
#include <config.h>
#include <mm/pmm.h>
#include <multiboot.h>	// usable_region 사용을 위해
#include <serial.h>
#include <stddef.h>
#include <stdint.h>

#ifdef PMM_DEBUG
	#define	PMM_DEBUG_MSG(s)	serial_write(s)
	#define	PMM_DEBUG_DEC(n)	serial_write_dec(n)
	#define	PMM_DEBUG_NEWLINE()	serial_write("\r\n")
#else
	#define	PMM_DEBUG_MSG(s)	((void)0)
	#define	PMM_DEBUG_DEC(n)	((void)0)
	#define	PMM_DEBUG_NEWLINE()	((void)0)
#endif

uint32_t last_region;

static buddy_pmm_t pmm;


static void list_push(uint32_t order, block_t* blk) {
	block_t* vblk = (block_t*)phys_to_virt((uintptr_t)blk);
	vblk->next = pmm.free_list[order];
	pmm.free_list[order] = blk;
}

static block_t *list_pop(uint32_t order) {
	block_t* blk = pmm.free_list[order];

	if (blk == NULL) return NULL;

	block_t* vblk = (block_t*)phys_to_virt((uintptr_t)blk);
	pmm.free_list[order] = vblk->next;
	return blk;
}

static int list_remove(uint32_t order, block_t* target) {
	block_t* blk = pmm.free_list[order];

	if (blk == NULL) return 0;

	block_t* vblk = (block_t*)phys_to_virt((uintptr_t)blk);
	if (blk == target) {
		pmm.free_list[order] = vblk->next;
		return 1;
	}

	while (vblk->next != NULL) {
		if (vblk->next == target) {
			block_t* vtarget = (block_t*)phys_to_virt((uintptr_t)target);
			vblk->next = vtarget->next;
			return 1;
		}
		blk = vblk->next;
		vblk = (block_t*)phys_to_virt((uintptr_t)blk);
	}
	return 0;
}

static inline uint64_t buddy_of(uint64_t addr, uint32_t order) {
	return addr ^ (PAGE_SIZE << order);
}

void* pmm_alloc(uint32_t order) {
	block_t* blk = NULL;
	for (uint32_t i = order; i < MAX_ORDER + 1; i++) {
		blk = list_pop(i);

		if (blk == NULL)
			continue;
		else {
			while (i > order) {
				i--;
				block_t* split = (block_t*)((uint64_t)blk + (PAGE_SIZE << i));
				list_push(i, split);
			}
			pmm.free_pages -= (1 << order);
			break;
		}
	}

	return (void*)blk;
}

void pmm_free(void* addr, uint32_t order) {
	uint32_t local_order = order;	

	block_t* pivot = (block_t*)addr;
	block_t* buddy = (block_t*)buddy_of((uint64_t)pivot, local_order);
	
	while (local_order < MAX_ORDER && list_remove(local_order, buddy)) {
		pivot = (block_t*)((uint64_t)pivot & ~((uint64_t)PAGE_SIZE << local_order));
		buddy = (block_t*)buddy_of((uint64_t)pivot, local_order + 1);
		local_order++;
	}

	list_push(local_order, pivot);
	pmm.free_pages += (1 << order);
}

void pmm_init_step1() {
	PMM_DEBUG_NEWLINE();
	PMM_DEBUG_MSG("[pmm_init_step1] start");
	PMM_DEBUG_NEWLINE();
	PMM_DEBUG_MSG("Usable Region Num: ");
	PMM_DEBUG_DEC(usable_region_count);
	PMM_DEBUG_NEWLINE();

	uint64_t page_num = 0;

	for (uint32_t i = 0; i < usable_region_count; i++) {
		PMM_DEBUG_MSG("Region #");
		PMM_DEBUG_DEC(i);
		PMM_DEBUG_NEWLINE();

		page_num = (usable_regions[i].end - usable_regions[i].start) >> 12;
		
		uint64_t pivot_addr = usable_regions[i].start;

		for (uint32_t j = 0; j < page_num; j++) {
			if (pivot_addr < PMM_STEP_LIMIT) {
				pmm_free((void*)pivot_addr, 0);
				pmm.total_pages++;

				pivot_addr += PAGE_SIZE;
			} else {
				last_region = i;
				i = usable_region_count;
				break;
			}
		}

		PMM_DEBUG_MSG("Sum of Total Page: ");
		PMM_DEBUG_DEC(pmm.total_pages);
		PMM_DEBUG_NEWLINE();
	}

	PMM_DEBUG_MSG("[pmm_init_step1] done");
	PMM_DEBUG_NEWLINE();
	PMM_DEBUG_NEWLINE();
}

void pmm_init_step2() {
	PMM_DEBUG_NEWLINE();
	PMM_DEBUG_MSG("[pmm_init_step2] start");
	PMM_DEBUG_NEWLINE();
	PMM_DEBUG_MSG("Region #");
	PMM_DEBUG_DEC(last_region);
	PMM_DEBUG_NEWLINE();

	serial_write_hex64(usable_regions[last_region].start);
	PMM_DEBUG_NEWLINE();
	serial_write_hex64(usable_regions[last_region].end);
	PMM_DEBUG_NEWLINE();
	uint64_t page_num = (usable_regions[last_region].end - usable_regions[last_region].start) >> 12;
	PMM_DEBUG_MSG(">");
	PMM_DEBUG_NEWLINE();
	PMM_DEBUG_DEC(page_num);
	PMM_DEBUG_NEWLINE();
		
	uint64_t last_pivot_addr = usable_regions[last_region].start;
	serial_write_hex64(last_pivot_addr);
	PMM_DEBUG_NEWLINE();

	for (uint32_t i = 0; i < page_num; i++) {
		if (last_pivot_addr <= PMM_STEP_LIMIT) {
			last_pivot_addr += PAGE_SIZE;
			// PMM_DEBUG_MSG(".");
		} else {
			pmm_free((void*)last_pivot_addr, 0);
			pmm.total_pages++;

			last_pivot_addr += PAGE_SIZE;
		}
	}

	PMM_DEBUG_MSG("Sum of Total Page: ");
	PMM_DEBUG_DEC(pmm.total_pages);
	PMM_DEBUG_NEWLINE();

	for (uint32_t i = last_region + 1; i < usable_region_count; i++) {
		PMM_DEBUG_MSG("Region #");
		PMM_DEBUG_DEC(i);
		PMM_DEBUG_NEWLINE();

		page_num = (usable_regions[i].end - usable_regions[i].start) >> 12;
		
		uint64_t pivot_addr = usable_regions[i].start;

		for (uint32_t j = 0; j < page_num; j++) {
			pmm_free((void*)pivot_addr, 0);
			pmm.total_pages++;

			pivot_addr += PAGE_SIZE;
		}

		PMM_DEBUG_MSG("Sum of Total Page: ");
		PMM_DEBUG_DEC(pmm.total_pages);
		PMM_DEBUG_NEWLINE();
	}

	PMM_DEBUG_MSG("[pmm_init_step2] done");
	PMM_DEBUG_NEWLINE();
}
