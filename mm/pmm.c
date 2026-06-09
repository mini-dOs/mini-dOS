#include <drivers/serial.h>
#include <kernel/config.h>
#include <kernel/kernel_base.h>
#include <kernel/multiboot.h>	// usable_region 사용을 위해
#include <mm/page.h>
#include <mm/pmm.h>
#include <stddef.h>
#include <stdint.h>

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
	// 0 ~ 1GB의 usable_region들
	for (uint32_t i = 0; i < usable_region_count; i++) {
		uint32_t page_num = (usable_regions[i].end - usable_regions[i].start) >> 12;
		uint64_t pivot_addr = usable_regions[i].start;

		// ---- DEBUG ----

		serial_write("Region #");
		serial_write_dec(i);

		// ---------------

		// 일반 usable_region; page_region은 page.h에 선언된 전역 변수로 page 구조체 배열이 들어갈 usable_region의 인덱스임
		if (i != page_region) {
			serial_write("\r\n");

			for (uint32_t j = 0; j < page_num; j++) {
				// 1GB 영역 내부라면
				if (pivot_addr < PMM_STEP_LIMIT) {
					pmm_free((void*)pivot_addr, 0);
					pmm.total_pages++;

					pivot_addr += PAGE_SIZE;
				} else {
					last_region = i;
					i = usable_region_count;	// for문 탈출 조건
					break;
				}
			}
		} // page 배열이 들어가는 usable_region
		else {
			serial_write(": page array region\r\n");
			for (uint32_t j = 0; j < page_num; j++) {
				// 1GB 영역 내부라면
				if (pivot_addr < PMM_STEP_LIMIT) {
					// 만약 현재 주소가 page 배열이 끝나는 주소보다 작다면
					if (pivot_addr < array_end_addr) {
						pivot_addr += PAGE_SIZE;
					} // 만약 현재 주소가 page 배열의 마지막 인덱스 이후라면
					else {
						pmm_free((void*)pivot_addr, 0);
						pmm.total_pages++;

						pivot_addr += PAGE_SIZE;
					}
				} else {
					last_region = i;
					i = usable_region_count;
					break;
				}
			}
		}
	}
}

void pmm_init_step2() {
	uint32_t page_num = (usable_regions[last_region].end - usable_regions[last_region].start) >> 12;
	uint64_t pivot_addr = usable_regions[last_region].start;

	// 현재 주소가 page 배열 내부라면 벗어날 때까지 현재 주소를 다음 페이지 주소로 변경
	// array_end_addr은 page.h에 선언된 전역 변수로 page 배열의 마지막 요소와 가장 가까운 다음 페이지 주소임 (4KB aligned)
	while (pivot_addr < array_end_addr) pivot_addr += PAGE_SIZE;

	// ---- DEBUG ----
	serial_write("Region #");
	serial_write_dec(last_region);
	serial_write("\r\n");
	// ---------------

	// 1GB ~ 하던 usable_region 마무리
	for (; pivot_addr < usable_regions[last_region].end; pivot_addr += PAGE_SIZE) {
		// 이미 진행했던 1GB 영역 내부라면
		if (pivot_addr < PMM_STEP_LIMIT) continue;	// 패스

		pmm_free((void*)pivot_addr, 0);
		pmm.total_pages++;
	}

	// 나머지 usable_region
	for (uint32_t i = last_region + 1; i < usable_region_count; i++) {
		// ---- DEBUG ----
		serial_write("Region #");
		serial_write_dec(i);
		serial_write("\r\n");
		// ---------------

		page_num = (usable_regions[i].end - usable_regions[i].start) >> 12;
		pivot_addr = usable_regions[i].start;

		for (uint32_t j = 0; j < page_num; j++) {
			pmm_free((void*)pivot_addr, 0);
			pmm.total_pages++;

			pivot_addr += PAGE_SIZE;
		}
	}
}

void pmm_get_stats(uint64_t *total_pages, uint64_t *free_pages) {
	if (total_pages) *total_pages = pmm.total_pages;
	if (free_pages)  *free_pages  = pmm.free_pages;
}
