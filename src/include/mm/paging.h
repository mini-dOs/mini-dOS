#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE 4096

typedef uint64_t pte_t; // Page Table Entry
typedef uint64_t page_table_t[512]; // Page Table (512 entries)

#define PAGE_PRESENT    (1ULL << 0)
#define PAGE_RW         (1ULL << 1)
#define PAGE_USER       (1ULL << 2)
#define PAGE_GLOBAL     (1ULL << 8)
#define PAGE_NX         (1ULL << 63)

#define PAGE_ADDR_MASK  0x000FFFFFFFFFF000ULL

void paging_init(void);
void map_page(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags);

#endif // PAGING_H