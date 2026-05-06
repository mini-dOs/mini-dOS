#include <kernel_base.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <stdint.h>
#include <string.h>

// paging.h의 전역 변수 -> 모든 페이지 테이블의 기반
uint64_t* pml4_root;

static inline uint64_t make_entry(uint64_t phys, uint64_t flags) {
    return (phys & PAGE_ADDR_MASK) | flags;
}

int map_page(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags) {
    uint64_t pml4_i = (va >> 39) & 0x1FF;
    uint64_t pdpt_i = (va >> 30) & 0x1FF;
    uint64_t pd_i   = (va >> 21) & 0x1FF;
    uint64_t pt_i   = (va >> 12) & 0x1FF;

    // PML4 -> PDPT
    if (!(pml4[pml4_i] & PAGE_PRESENT)) {
        void* p = pmm_alloc(0);
        if (!p)
            return -1;

        uint64_t* pdpt = phys_to_virt((uintptr_t)p);
        memset(pdpt, 0, PAGE_SIZE);

        // pdpt is currently identity-mapped, so virtual == physical.
        // Use virt_to_phys for future higher-half transition.
        pml4[pml4_i] = make_entry(virt_to_phys(pdpt), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pdpt = (uint64_t*)phys_to_virt(pml4[pml4_i] & PAGE_ADDR_MASK);

    // PDPT -> PD
    if (!(pdpt[pdpt_i] & PAGE_PRESENT)) {
        void* p = pmm_alloc(0);
        if (!p)
            return -1;

        uint64_t* pd = phys_to_virt((uintptr_t)p);
        memset(pd, 0, PAGE_SIZE);

        pdpt[pdpt_i] = make_entry(virt_to_phys(pd), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pd = (uint64_t*)phys_to_virt(pdpt[pdpt_i] & PAGE_ADDR_MASK);

    // PD -> PT
    if (!(pd[pd_i] & PAGE_PRESENT)) {
        void* p = pmm_alloc(0);
        if (!p)
            return -1;

        uint64_t* pt = phys_to_virt((uintptr_t)p);
        memset(pt, 0, PAGE_SIZE);

        pd[pd_i] = make_entry(virt_to_phys(pt), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pt = (uint64_t*)phys_to_virt(pd[pd_i] & PAGE_ADDR_MASK);

    // 이미 매핑된 엔트리는 호출자가 결정하도록 EEXIST 반환
    if (pt[pt_i] & PAGE_PRESENT)
        return MAP_EEXIST;

    // PT -> Page
    pt[pt_i] = make_entry(pa, flags | PAGE_PRESENT);
    return 0;
}

uint64_t unmap_page(uint64_t* pml4, uint64_t va) {
    // 1. va를 PML4/PDPT/PD/PT 인덱스로 분해
    uint64_t pml4_i = (va >> 39) & 0x1FF;
    uint64_t pdpt_i = (va >> 30) & 0x1FF;
    uint64_t pd_i   = (va >> 21) & 0x1FF;
    uint64_t pt_i   = (va >> 12) & 0x1FF;

    // 2. 각 단계에서 PAGE_PRESENT 확인, 없으면 0 반환
    if (!(pml4[pml4_i] & PAGE_PRESENT))
        return 0;
    uint64_t* pdpt = phys_to_virt(pml4[pml4_i] & PAGE_ADDR_MASK);

    if (!(pdpt[pdpt_i] & PAGE_PRESENT))
        return 0;
    uint64_t* pd = phys_to_virt(pdpt[pdpt_i] & PAGE_ADDR_MASK);

    if ((!(pd[pd_i] & PAGE_PRESENT)) || (pd[pd_i] & PAGE_PS))
        return 0;
    uint64_t* pt = phys_to_virt(pd[pd_i] & PAGE_ADDR_MASK);

    if (!(pt[pt_i] & PAGE_PRESENT))
        return 0;
    // 3. PT 엔트리에서 PA 추출, 엔트리를 0으로 클리어
    uint64_t pa = pt[pt_i] & PAGE_ADDR_MASK;
    pt[pt_i] = 0;

    // 4. 추출한 PA 반환 (호출자가 pmm_free 여부 결정)
    return pa;
}

int map_page_2mb(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags) {
    uint64_t pml4_i = (va >> 39) & 0x1FF;
    uint64_t pdpt_i = (va >> 30) & 0x1FF;
    uint64_t pd_i   = (va >> 21) & 0x1FF;

    // PML4 -> PDPT
    if (!(pml4[pml4_i] & PAGE_PRESENT)) {
        void* p = pmm_alloc(0);
        if (!p)
            return -1;

        uint64_t* pdpt = phys_to_virt((uintptr_t)p);
        memset(pdpt, 0, PAGE_SIZE);

        pml4[pml4_i] = make_entry(virt_to_phys(pdpt), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pdpt = (uint64_t*)phys_to_virt(pml4[pml4_i] & PAGE_ADDR_MASK);

    // PDPT -> PD
    if (!(pdpt[pdpt_i] & PAGE_PRESENT)) {
        void* p = pmm_alloc(0);
        if (!p)
            return -1;

        uint64_t* pd = phys_to_virt((uintptr_t)p);
        memset(pd, 0, PAGE_SIZE);

        pdpt[pdpt_i] = make_entry(virt_to_phys(pd), PAGE_PRESENT | PAGE_RW);
    }

    uint64_t* pd = (uint64_t*)phys_to_virt(pdpt[pdpt_i] & PAGE_ADDR_MASK);

    // 이미 매핑된 엔트리는 호출자가 결정하도록 EEXIST 반환
    if (pd[pd_i] & PAGE_PRESENT)
        return MAP_EEXIST;

    pd[pd_i] = make_entry(pa, flags | PAGE_PRESENT);
    return 0;
}

uint64_t unmap_page_2mb(uint64_t* pml4, uint64_t va) {
    uint64_t pml4_i = (va >> 39) & 0x1FF;
    uint64_t pdpt_i = (va >> 30) & 0x1FF;
    uint64_t pd_i   = (va >> 21) & 0x1FF;

    if (!(pml4[pml4_i] & PAGE_PRESENT))
        return 0;
    uint64_t* pdpt = phys_to_virt(pml4[pml4_i] & PAGE_ADDR_MASK);

    if (!(pdpt[pdpt_i] & PAGE_PRESENT))
        return 0;
    uint64_t* pd = phys_to_virt(pdpt[pdpt_i] & PAGE_ADDR_MASK);

    // leaf 검증: PRESENT + PAGE_PS (4KB 매핑이면 무시)
    if (!(pd[pd_i] & PAGE_PRESENT) || !(pd[pd_i] & PAGE_PS))
        return 0;

    uint64_t pa = pd[pd_i] & PAGE_ADDR_MASK;
    pd[pd_i] = 0;
    return pa;
}
