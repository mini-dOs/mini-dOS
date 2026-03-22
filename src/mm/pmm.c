#include <pmm.h>

extern uint64_t _kernel_end;

static buddy_pmm_t pmm;


static void list_push(uint32_t order, block_t *blk) {
	struct block_
	pmm->next = pmm.free_list[order];	
}

static block_t *list_pop(uint32_t order) {
}

static int list_remove(uint32_t order, block_t *target) {
}

static inline uint64_t buddy_of(uint64_t addr, uint32_t order) {
}

void pmm_init(uint64_t mmap_addr, uint32_t mmap_len) {
	for (uint8_t i = 0; i < MAX_ORDER + 1; i++) {
		pmm.free_list[i] = NULL;
	}



	for (uint32_t i = 0; i < mmap_len; i++) {
		
	}

	total_pages = 
}

void *pmm_alloc(uint32_t order) {
}

void pmm_free(void *addr, uint32_t order) {
}
