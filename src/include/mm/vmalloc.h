#ifndef VMALLOC_H
#define VMALLOC_H

#include <stdint.h>

void vmalloc_init(void);
void *vmalloc(uint64_t size);
void vfree(void *ptr);

// debug: 시리얼로 vmalloc 영역 노드 리스트 출력
void vmalloc_dump(void);

#endif // VMALLOC_H