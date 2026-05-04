#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <kernel_info.h>

extern uint64_t* pml4_root;

typedef uint64_t pte_t; // Page Table Entry
typedef uint64_t page_table_t[512]; // Page Table (512 entries)

#define PAGE_PRESENT    (1ULL << 0)
#define PAGE_RW         (1ULL << 1)
#define PAGE_USER       (1ULL << 2)
#define PAGE_PS         (1ULL << 7)
#define PAGE_GLOBAL     (1ULL << 8)
#define PAGE_NX         (1ULL << 63)

#define PAGE_ADDR_MASK  0x000FFFFFFFFFF000ULL

int map_page(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags);
void map_page_2mb(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags);

// 매핑 해제 후 해제된 PTE의 물리 주소를 반환 (없으면 0).
// 단일 코어 가정 — TLB shootdown 없음, 로컬 invlpg만 수행.
uint64_t unmap_page(uint64_t* pml4, uint64_t va);

static inline void invlpg(uint64_t va)
{
    asm volatile("invlpg (%0)" :: "r"(va) : "memory");
}

#endif // PAGING_H
