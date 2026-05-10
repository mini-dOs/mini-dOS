#include <kernel/kernel_info.h>
#include <asm/cpu.h>
#include <drivers/serial.h>
#include <kernel/early_alloc.h>
#include <kernel/kernel_base.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <kernel/multiboot.h>
#include <stdint.h>
#include <string.h>

void vmm_init(void)
{
    void* p = pmm_alloc(0);
    if (!p) {
        serial_write("[vmm.c] vmm_init OOM allocating PML4\n");
        for (;;)
            hlt();
    }

    pml4_root = phys_to_virt((uintptr_t)p);
    memset(pml4_root, 0, PAGE_SIZE);

    uint64_t start = (uint64_t)kernel_vma();
    uint64_t end   = (uint64_t)kernel_vma_end() + EARLY_ALLOC_SIZE; // 커널과 early_alloc 영역 모두 매핑

    // page align
    start &= ~(PAGE_SIZE - 1);
    end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
        if (map_page(pml4_root, addr, kernel_virt_to_phys((void*)addr), PAGE_RW) < 0) {
            serial_write("[vmm.c] vmm_init OOM mapping kernel image\n");
            for (;;)
                hlt();
        }
    }

    for (uint32_t i = 0; i < usable_region_count; i++) {
        uint64_t usable_start = usable_regions[i].start & ~(PAGE_2MB - 1); // 2MB 내림 정렬
        uint64_t usable_end = (usable_regions[i].end + PAGE_2MB - 1) & ~(PAGE_2MB - 1); // 2MB 올림 정렬

        for (uint64_t phys_addr = usable_start; phys_addr < usable_end; phys_addr += PAGE_2MB) {
            uint64_t va = (uint64_t)phys_to_virt(phys_addr);
            // 인접 region이 2MB round-up으로 같은 페이지에 떨어지면 EEXIST 발생 → 정상
            int rc = map_page_2mb(pml4_root, va, phys_addr, PAGE_RW | PAGE_PS);
            if (rc == MAP_ENOMEM) {
                serial_write("[vmm.c] vmm_init OOM mapping direct-map\n");
                for (;;)
                    hlt();
            }
        }
    }

    // CR3 switch (must use physical address)
    asm volatile("mov %0, %%cr3" :: "r"(virt_to_phys(pml4_root)) : "memory");
}

int vmm_map_phys(uint64_t va, uint64_t pa, uint64_t size, uint64_t flags)
{
    if (flags & PAGE_PS) {
        // 2MB
        if ((va | pa) & (PAGE_2MB - 1))
            return -1;
        size = (size + PAGE_2MB - 1) & ~(PAGE_2MB - 1);

        for (uint64_t offset = 0; offset < size; offset += PAGE_2MB) {
            if (map_page_2mb(pml4_root, va + offset, pa + offset, flags) < 0) {
                for (uint64_t roll = 0; roll < offset; roll += PAGE_2MB) {
                    unmap_page_2mb(pml4_root, va + roll);
                    invlpg(va + roll);
                }
                return -1;
            } 
            invlpg(va + offset);
        }
    } else {
        // 4KB
        if ((va | pa) & (PAGE_SIZE - 1))
            return -1;
        size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        for (uint64_t offset = 0; offset < size; offset += PAGE_SIZE) {
            if (map_page(pml4_root, va + offset, pa + offset, flags) < 0) {
                for (uint64_t roll = 0; roll < offset; roll += PAGE_SIZE) {
                    unmap_page(pml4_root, va + roll);
                    invlpg(va + roll);
                }
                return -1;
            } 
            invlpg(va + offset);
        }
    }

    return 0;
}

int vmm_alloc(uint64_t va, uint64_t size, uint64_t flags)
{
    uint64_t page_size = (flags & PAGE_PS) ? PAGE_2MB : PAGE_SIZE;
    uint32_t order     = (flags & PAGE_PS) ? 9 : 0;

    if (va & (page_size - 1))
        return -1;

    size = (size + page_size - 1) & ~(page_size - 1);

    for (uint64_t offset = 0; offset < size; offset += page_size) {
        void* p = pmm_alloc(order);
        int rc = -1;

        if (p) {
            uint64_t pa = (uint64_t)p;
            rc = (page_size == PAGE_2MB)
                ? map_page_2mb(pml4_root, va + offset, pa, flags)
                : map_page(pml4_root, va + offset, pa, flags);
            if (rc < 0)
                pmm_free(p, order);
        }

        if (rc < 0) {
            for (uint64_t roll = 0; roll < offset; roll += page_size) {
                uint64_t roll_pa = (page_size == PAGE_2MB)
                    ? unmap_page_2mb(pml4_root, va + roll)
                    : unmap_page(pml4_root, va + roll);
                invlpg(va + roll);
                if (roll_pa)
                    pmm_free((void*)roll_pa, order);
            }
            return -1;
        }
        invlpg(va + offset);
    }
    return 0;
}

static uint64_t walk_step(uint64_t va) {
    uint64_t pml4_i = (va >> 39) & 0x1FF;
    uint64_t pdpt_i = (va >> 30) & 0x1FF;
    uint64_t pd_i   = (va >> 21) & 0x1FF;

    if (!(pml4_root[pml4_i] & PAGE_PRESENT))
        return PAGE_SIZE;
    
    uint64_t* pdpt = phys_to_virt(pml4_root[pml4_i] & PAGE_ADDR_MASK);
    if (!(pdpt[pdpt_i] & PAGE_PRESENT))
        return PAGE_SIZE;

    uint64_t* pd = phys_to_virt(pdpt[pdpt_i] & PAGE_ADDR_MASK);
    if ((pd[pd_i] & PAGE_PRESENT) && (pd[pd_i] & PAGE_PS))
        return PAGE_2MB;

    return PAGE_SIZE;
}

void vmm_unmap(uint64_t va, uint64_t size)
{
    uint64_t end = va + size;

    while (va < end) {
        uint64_t step = walk_step(va);

        if (step == PAGE_2MB)
            unmap_page_2mb(pml4_root, va);
        else
            unmap_page(pml4_root, va);

        invlpg(va);
        va += step;
    }
}

void vmm_free(uint64_t va, uint64_t size)
{
    uint64_t end = va + size;

    while (va < end) {
        uint64_t step = walk_step(va);
        uint64_t pa;
        uint32_t order;

        if (step == PAGE_2MB) {
            pa = unmap_page_2mb(pml4_root, va);
            order = 9;
        } else {
            pa = unmap_page(pml4_root, va);
            order = 0;
        }

        invlpg(va);
        if (pa)
            pmm_free((void*)pa, order);
        va += step;
    }
}
