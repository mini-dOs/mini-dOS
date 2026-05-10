#ifndef SLAB_H
#define SLAB_H

#include <mm/paging.h>
#include <mm/vmm.h>

#define	KMEM_CACHE_COUNT 9	// 8B ~ 2KB (9개)

typedef struct slab slab_t;
typedef struct kmem_cache kmem_cache_t;

typedef struct slab {
	kmem_cache_t* kmem_cache;
	slab_t* next;	// kmem_cache에서 Linked List로 관리하기 위해 필요
	uint32_t free_count;
	void* free_list;	// slab 내부의 object를 Linked List로 관리하기 위해 필요
} slab_t;

typedef struct kmem_cache {
	uint64_t obj_size;	// 8B ~ 2KB (9개)
	uint64_t obj_capacity;	// cache가 담당하는 obj의 크기들로 쪼개질 수 있는 obj의 개수
	slab_t* full;
	slab_t* partial;
	slab_t* empty;		// 얘만 Linked List 아님
} kmem_cache_t;

extern kmem_cache_t kmem_cache_list[KMEM_CACHE_COUNT];

void* object_alloc(kmem_cache_t* kmem_cache);
void object_free(kmem_cache_t* kmem_cache, void* object);
void* kmalloc(uint64_t size);
void kfree(void* object);
void slab_init(kmem_cache_t* kmem_cache, slab_t* slab);
void kmem_cache_init();

#endif
