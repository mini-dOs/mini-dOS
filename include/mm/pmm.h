#ifndef PMM_H
#define PMM_H

#include <kernel/kernel_info.h>
#include <stdint.h>

// free block을 연결 리스트로 관리하기 위한 노드
typedef struct block {
	struct block *next;
} block_t;

typedef struct buddy_pmm {
	block_t *free_list[MAX_ORDER + 1];	// order 0부터 order 10까지 각 크기별 free block 리스트
	uint64_t total_pages;			// 전체 페이지 수
	uint64_t free_pages;			// 남은 페이지 수
} buddy_pmm_t;

void *pmm_alloc(uint32_t order);
void pmm_free(void *addr, uint32_t order);
void pmm_init_step1();
void pmm_init_step2();

// 버디 할당기 통계(전체/남은 페이지 수)를 돌려준다. NULL 인자는 무시.
void pmm_get_stats(uint64_t *total_pages, uint64_t *free_pages);

#endif
