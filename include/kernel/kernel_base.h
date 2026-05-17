#ifndef KERNEL_BASE_H
#define KERNEL_BASE_H

#include <kernel/kernel_info.h>
#include <stdint.h>

extern char KERNEL_VMA[];
extern char _kernel_end[];

/**
 * 커널의 virtual base address (KERNEL_VMA)를 반환함
 */
static inline uintptr_t kernel_vma(void)
{
    return (uintptr_t)KERNEL_VMA;
}

/**
 * 커널의 virtual end address (_kernel_end)를 반환함 
 */
static inline uintptr_t kernel_vma_end(void)
{
    return (uintptr_t)_kernel_end;
}

/**
 * 커널 이미지의 virtual address를 physical address로 변환함 (KERNEL_VMA 기준)
 */
static inline uintptr_t kernel_virt_to_phys(void* addr)
{
    return (uintptr_t)addr - kernel_vma();
}

/**
 * 커널의 virtual address를 physical address로 변환함
 */
static inline uintptr_t virt_to_phys(void* addr)
{
    return (uintptr_t)addr - PAGE_OFFSET_BASE;
}

/**
 * physical address를 kernel virtual address로 변환함
 */
static inline void* phys_to_virt(uintptr_t addr)
{
    return (void*)(addr + PAGE_OFFSET_BASE);
}

#endif
