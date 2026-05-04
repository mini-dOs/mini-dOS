#include <early_alloc.h>
#include <kernel_base.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <multiboot.h>
#include <stdint.h>
#include <string.h>

void vmm_init(void)
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
    asm volatile("mov %0, %%cr3" :: "r"(virt_to_phys(pml4_root)) : "memory");
}

// 단일 코어 가정 — invlpg는 paging.h의 inline 사용, TLB shootdown 없음.

int vmm_map_phys(uint64_t va, uint64_t pa, uint64_t size, uint64_t flags)
{
    // TODO:
    // 1. (flags & PAGE_PS)면 2MB 경로, 아니면 4KB 경로 선택
    //    - 2MB 경로: va/pa/size가 PAGE_2MB 정렬인지 검증, 아니면 -1
    //    - 4KB 경로: size를 PAGE_SIZE 단위로 올림 정렬
    // 2. 루프: map_page / map_page_2mb 호출 (반환값 0이 아니면 롤백 후 -1)
    // 3. 각 페이지마다 invlpg(va)
    // 4. 성공 0, 실패 -1
    (void)va; (void)pa; (void)size; (void)flags;
    return -1;
}

int vmm_alloc(uint64_t va, uint64_t size, uint64_t flags)
{
    // TODO:
    // 1. size를 페이지 크기 단위로 올림 정렬
    // 2. 페이지 단위 루프:
    //    - pmm_alloc(order)로 프레임 확보 (실패 시 지금까지 매핑 롤백 후 -1)
    //    - map_page(pml4_root, va, pa, flags) 호출
    //    - invlpg(va)
    // 3. 성공 0, 실패 -1
    (void)va; (void)size; (void)flags;
    return -1;
}

void vmm_unmap(uint64_t va, uint64_t size)
{
    // TODO: 호출자 소유 PA를 가진 매핑 해제 (pmm_free 호출하지 않음)
    // 1. size를 페이지 크기 단위로 올림 정렬
    // 2. 루프: unmap_page(pml4_root, va) 호출 (반환값 PA는 버림)
    // 3. 각 페이지마다 invlpg(va)
    (void)va; (void)size;
}

void vmm_free(uint64_t va, uint64_t size)
{
    // TODO: VMM 소유 매핑 해제 + 물리 프레임 반환
    // 1. size를 페이지 크기 단위로 올림 정렬
    // 2. 루프:
    //    - pa = unmap_page(pml4_root, va)
    //    - pa != 0이면 pmm_free((void*)pa, 0)
    //    - invlpg(va)
    (void)va; (void)size;
}
