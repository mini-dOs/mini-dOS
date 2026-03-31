#include <early_alloc.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <stdint.h>
#include <string.h>

// paging.h의 전역 변수 -> 모든 페이지 테이블의 기반
uint64_t* pml4_root;

// linker symbols → address 자체
extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

// 현재는 identity mapping (VA == PA)
static inline uint64_t virt_to_phys(void* vaddr) {
    return (uint64_t)vaddr;
}

static inline uint64_t make_entry(uint64_t phys, uint64_t flags) {
    return (phys & PAGE_ADDR_MASK) | flags;
}

static inline void load_cr3(uint64_t pml4_phys) {
    asm volatile("mov %0, %%cr3" :: "r"(pml4_phys) : "memory");
}

void paging_init(void) {
    pml4_root = pmm_alloc(0);
    memset(pml4_root, 0, PAGE_SIZE);

    uint64_t start = (uint64_t)_kernel_start;
    uint64_t end   = (uint64_t)_kernel_end + EARLY_ALLOC_SIZE;	// early_alloc.h

    // page align
    start &= ~(PAGE_SIZE - 1);
    end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
        map_page(pml4_root, addr, addr, PAGE_RW);
    }

    // CR3 switch (must use physical address)
    load_cr3(virt_to_phys(pml4_root));
}

void map_page(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags) {
    uint64_t pml4_i = (va >> 39) & 0x1FF;
    uint64_t pdpt_i = (va >> 30) & 0x1FF;
    uint64_t pd_i   = (va >> 21) & 0x1FF;
    uint64_t pt_i   = (va >> 12) & 0x1FF;

    // PML4 -> PDPT
    if (!(pml4[pml4_i] & PAGE_PRESENT)) {
        uint64_t* pdpt = pmm_alloc(0);
        memset(pdpt, 0, PAGE_SIZE);

        // pdpt is currently identity-mapped, so virtual == physical.
        // Use virt_to_phys for future higher-half transition.
        pml4[pml4_i] = make_entry(virt_to_phys(pdpt), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pdpt = (uint64_t*)(pml4[pml4_i] & PAGE_ADDR_MASK);

    // PDPT -> PD
    if (!(pdpt[pdpt_i] & PAGE_PRESENT)) {
        uint64_t* pd = pmm_alloc(0);
        memset(pd, 0, PAGE_SIZE);

        pdpt[pdpt_i] = make_entry(virt_to_phys(pd), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pd = (uint64_t*)(pdpt[pdpt_i] & PAGE_ADDR_MASK);

    // PD -> PT
    if (!(pd[pd_i] & PAGE_PRESENT)) {
        uint64_t* pt = pmm_alloc(0);
        memset(pt, 0, PAGE_SIZE);

        pd[pd_i] = make_entry(virt_to_phys(pt), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pt = (uint64_t*)(pd[pd_i] & PAGE_ADDR_MASK);

    // PT -> Page
    pt[pt_i] = make_entry(pa, flags | PAGE_PRESENT);
}
