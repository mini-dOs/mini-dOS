#include <early_alloc.h>
#include <stdint.h>
#include <mm/pmm_tmp.h>

// 임시 PMM: early allocator 기반
void* pmm_alloc_page(void) {
    void* ptr = early_alloc(4096, 4096); // 4KB 페이지 크기, 페이지 정렬

    // page alignment 보장 (중요)
    uintptr_t aligned = ((uintptr_t)ptr + 0xFFF) & ~0xFFF;

    return (void*)aligned;
}