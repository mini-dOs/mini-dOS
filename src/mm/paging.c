#include <early_alloc.h>
#include <kernel_base.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <multiboot.h>
#include <stdint.h>
#include <string.h>

// paging.h의 전역 변수 -> 모든 페이지 테이블의 기반
uint64_t* pml4_root;

static inline uint64_t make_entry(uint64_t phys, uint64_t flags) 
{
    return (phys & PAGE_ADDR_MASK) | flags;
}

static inline void load_cr3(uint64_t pml4_phys) 
{
    asm volatile("mov %0, %%cr3" :: "r"(pml4_phys) : "memory");
}

void paging_init(void)
{
    pml4_root = phys_to_virt((uintptr_t)pmm_alloc(0));
    memset(pml4_root, 0, PAGE_SIZE);

    uint64_t start = (uint64_t)kernel_vma();
    uint64_t end   = (uint64_t)kernel_vma_end() + EARLY_ALLOC_SIZE; // 커널과 early_alloc 영역 모두 매핑

    // page align
    start &= ~(PAGE_SIZE - 1);
    end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
        map_page(pml4_root, addr, kernel_virt_to_phys((void*)addr), PAGE_RW);
    }

    for (uint32_t i = 0; i < usable_region_count; i++) {
        uint64_t usable_start = usable_regions[i].start & ~(PAGE_2MB - 1); // 2MB 내림 정렬
        uint64_t usable_end = (usable_regions[i].end + PAGE_2MB - 1) & ~(PAGE_2MB - 1); // 2MB 올림 정렬

        for (uint64_t phys_addr = usable_start; phys_addr < usable_end; phys_addr += PAGE_2MB) {
            uint64_t va = (uint64_t)phys_to_virt(phys_addr);
            map_page_2mb(pml4_root, va, phys_addr, PAGE_RW | PAGE_PS);
        }
    }

    // CR3 switch (must use physical address)
    load_cr3(virt_to_phys(pml4_root));
}

void map_page(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags)
{
    uint64_t pml4_i = (va >> 39) & 0x1FF;
    uint64_t pdpt_i = (va >> 30) & 0x1FF;
    uint64_t pd_i   = (va >> 21) & 0x1FF;
    uint64_t pt_i   = (va >> 12) & 0x1FF;

    // PML4 -> PDPT
    if (!(pml4[pml4_i] & PAGE_PRESENT)) {
        uint64_t* pdpt = phys_to_virt((uintptr_t)pmm_alloc(0));
        memset(pdpt, 0, PAGE_SIZE);

        // pdpt is currently identity-mapped, so virtual == physical.
        // Use virt_to_phys for future higher-half transition.
        pml4[pml4_i] = make_entry(virt_to_phys(pdpt), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pdpt = (uint64_t*)phys_to_virt(pml4[pml4_i] & PAGE_ADDR_MASK);

    // PDPT -> PD
    if (!(pdpt[pdpt_i] & PAGE_PRESENT)) {
        uint64_t* pd = phys_to_virt((uintptr_t)pmm_alloc(0));
        memset(pd, 0, PAGE_SIZE);

        pdpt[pdpt_i] = make_entry(virt_to_phys(pd), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pd = (uint64_t*)phys_to_virt(pdpt[pdpt_i] & PAGE_ADDR_MASK);

    // PD -> PT
    if (!(pd[pd_i] & PAGE_PRESENT)) {
        uint64_t* pt = phys_to_virt((uintptr_t)pmm_alloc(0));
        memset(pt, 0, PAGE_SIZE);

        pd[pd_i] = make_entry(virt_to_phys(pt), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pt = (uint64_t*)phys_to_virt(pd[pd_i] & PAGE_ADDR_MASK);

    // PT -> Page
    pt[pt_i] = make_entry(pa, flags | PAGE_PRESENT);
}

void map_page_2mb(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags)
{
    uint64_t pml4_i = (va >> 39) & 0x1FF;
    uint64_t pdpt_i = (va >> 30) & 0x1FF;
    uint64_t pd_i   = (va >> 21) & 0x1FF;

    // PML4 -> PDPT
    if (!(pml4[pml4_i] & PAGE_PRESENT)) {
        uint64_t* pdpt = phys_to_virt((uintptr_t)pmm_alloc(0));
        memset(pdpt, 0, PAGE_SIZE);

        pml4[pml4_i] = make_entry(virt_to_phys(pdpt), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pdpt = (uint64_t*)phys_to_virt(pml4[pml4_i] & PAGE_ADDR_MASK);

    // PDPT -> PD
    if (!(pdpt[pdpt_i] & PAGE_PRESENT)) {
        uint64_t* pd = phys_to_virt((uintptr_t)pmm_alloc(0));
        memset(pd, 0, PAGE_SIZE);

        pdpt[pdpt_i] = make_entry(virt_to_phys(pd), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pd = (uint64_t*)phys_to_virt(pdpt[pdpt_i] & PAGE_ADDR_MASK);

    // 이미 매핑된 엔트리가 있으면 건너뜀
    if (pd[pd_i] & PAGE_PRESENT)
        return;

    pd[pd_i] = make_entry(pa, flags | PAGE_PRESENT);
}
