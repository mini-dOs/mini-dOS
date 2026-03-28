#include <early_alloc.h>
#include <stdint.h>
#include <mm/pmm_tmp.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

// 임시 PMM: early allocator 기반
void* pmm_alloc_page(void) {
    return early_alloc(PAGE_SIZE, PAGE_SIZE); // 4KB 페이지 크기, 페이지 정렬
}