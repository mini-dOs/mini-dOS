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
