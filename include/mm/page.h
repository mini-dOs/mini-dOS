#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

// Page Status
#define FREE	0
#define SLAB	1
#define PMM	2
#define VMM	3

// 64B 크기 보장
typedef struct page {
	uint8_t status;
	union {
		uint8_t order;	// kmalloc 할당 페이지 order
		uint32_t size;	// vmalloc 할당 크기
	};
	uint8_t reserved[55];	// union 크기에 따라 변경
} page_t;

extern page_t*	page;
extern uint64_t array_end_addr;
extern uint32_t	page_region;

void page_init();
void page_array_init();

#endif
