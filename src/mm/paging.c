#include <kernel_base.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <stdint.h>
#include <string.h>

// paging.h의 전역 변수 -> 모든 페이지 테이블의 기반
uint64_t* pml4_root;

static inline uint64_t make_entry(uint64_t phys, uint64_t flags)
{
    return (phys & PAGE_ADDR_MASK) | flags;
}

// TODO: pmm_alloc 실패(NULL) 시 -1 반환하도록 OOM 경로 처리.
//       지금은 시그니처만 맞추기 위해 항상 0 반환.
int map_page(uint64_t* pml4, uint64_t va, uint64_t pa, uint64_t flags)
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
    return 0;
}

uint64_t unmap_page(uint64_t* pml4, uint64_t va)
{
    // TODO:
    // 1. va를 PML4/PDPT/PD/PT 인덱스로 분해
    // 2. 각 단계에서 PAGE_PRESENT 확인, 없으면 0 반환
    // 3. PT 엔트리에서 PA 추출, 엔트리를 0으로 클리어
    // 4. 추출한 PA 반환 (호출자가 pmm_free 여부 결정)
    // (선택) 빈 PT/PD/PDPT 회수는 추후 — 지금은 leaf만 처리
    (void)pml4; (void)va;
    return 0;
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
