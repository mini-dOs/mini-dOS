#include <pmm.h>

extern uint64_t _kernel_end;

static buddy_pmm_t pmm;


static void list_push(uint32_t order, block_t *blk) {
	blk->next = pmm.free_list[order];	
	pmm.free_list[order] = blk;
}

static block_t *list_pop(uint32_t order) {
	block_t* blk = pmm.free_list[order];

	if (blk == NULL) return NULL;

	pmm.free_list[order] = blk->next;
	return blk;
}

static int list_remove(uint32_t order, block_t *target) {
	block_t* blk = pmm.free_list[order];

	if (blk == NULL) return 0;
	if (blk == target) {
		pmm.free_list[order] = blk->next;
		return 1;
	}

	while (blk->next != NULL) {
		if (blk->next == target) {
			blk->next = blk->next->next;
			return 1;
		}
		else blk = blk->next;
	}
	return 0;
}

static inline uint64_t buddy_of(uint64_t addr, uint32_t order) {
	return addr ^ (PAGE_SIZE << order);
}

void *pmm_alloc(uint32_t order) {
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

void pmm_free(void *addr, uint32_t order) {
	uint32_t local_order = order;	

	block_t* left_blk = (block_t*)addr;	// 지금은 왼쪽 블록이 아닐수도 있음
	block_t* right_blk = (block_t*)buddy_of((uint64_t)left_blk, local_order);	// 지금은 오른쪽 블록이 아닐수도 있음
	
	while (local_order < MAX_ORDER && list_remove(local_order, right_blk)) {
		local_order++;
		left_blk ^= PAGE_SIZE << (local_order);
		right_blk = (block_t*)buddy_of((uint64_t)left_blk, local_order);
	}
}

void pmm_init(uint64_t mmap_addr, uint32_t mmap_len) {
	for (uint32_t i = 0; i < MAX_ORDER + 1; i++) {
		pmm.free_list[i] = NULL;
	}



	for (uint32_t i = 0; i < mmap_len; i++) {
		
	}

	total_pages = 
}
