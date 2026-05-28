#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/kernel_info.h>
#include <mm/slab.h>
#include <mm/vmalloc.h>

static size_t usable_size(void *ptr) {
    // vmalloc 영역: vm_area 노드에서 size 조회
    if ((uintptr_t)ptr >= VMALLOC_START && (uintptr_t)ptr < VMALLOC_END)
        return (size_t)vmalloc_usable_size(ptr);

    // slab 영역: object가 속한 slab은 페이지 정렬되어 있고 헤더가 페이지 선두에 위치
    slab_t *slab = (slab_t *)((uintptr_t)ptr & ~(uintptr_t)0xFFF);
    return (size_t)slab->kmem_cache->obj_size;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL)
        return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    size_t old = usable_size(ptr);

    // 기존 블록 안에 들어가면 그대로 재사용
    if (size <= old)
        return ptr;

    // 확장: 새로 할당해 복사. 실패 시 원본은 유지
    void *new_ptr = malloc(size);
    if (new_ptr == NULL)
        return NULL;

    memcpy(new_ptr, ptr, old);
    free(ptr);
    return new_ptr;
}
