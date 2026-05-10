#include <kernel/kernel_base.h>	// phys_to_virt
#include <kernel/kernel_info.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/slab.h>
#include <mm/vmm.h>
#include <stddef.h>

kmem_cache_t kmem_cache_list[KMEM_CACHE_COUNT];	// KMEM_CACHE_COUNT는 9 (mm/slab.h)

void* object_alloc(kmem_cache_t* kmem_cache) {
	slab_t* slab;

	if (kmem_cache->partial == NULL) {
		if (kmem_cache->empty == NULL) {	// 빈 slab이 하나도 없을 경우
			slab = (slab_t*)phys_to_virt((uintptr_t)pmm_alloc(0));	// 4KB 페이지 할당

			slab_init(kmem_cache, slab);
		} else {	// empty slab이라도 있을 경우
			slab = kmem_cache->empty;
		}
	} else {
		slab = kmem_cache->partial;
	}

	/* 실제 object 할당 */
	void* object = slab->free_list;
	slab->free_list = (void*)*((uintptr_t*)slab->free_list);
	slab->free_count--;

	// object가 2KB인 경우에는 object_capacity가 1이기 때문에 발생 가능
	// empty -> full이 된 경우
	if (kmem_cache->partial == NULL && slab->free_count == 0) {
		slab->next = kmem_cache->full;
		kmem_cache->full = slab;
		kmem_cache->empty = NULL;
	} // partial -> full이 된 경우
	else if (slab->free_count == 0) {
		slab_t* prev = kmem_cache->partial;

		// slab을 partial list에서 제거
		if (prev != slab) {
			while (prev->next != slab) prev = prev->next;

			prev->next = prev->next->next;
		} else {
			kmem_cache->partial = prev->next;
		}
		
		// full에 추가
		slab->next = kmem_cache->full;
		kmem_cache->full = slab;
	} // empty -> partial이 된 경우
	else if (slab->free_count == kmem_cache->obj_capacity - 1) {
		// partial에 추가
		slab->next = kmem_cache->partial;
		kmem_cache->partial = slab;
		kmem_cache->empty = NULL;
	}

	return object;
}

void object_free(kmem_cache_t* kmem_cache, void* object) {
	// object의 slab 주소
	slab_t* slab = (slab_t*)((uintptr_t)object & ~(0xFFF));	// 포인터를 정수 타입으로 변환 후 비트 연산 (포인터는 비트 연산이 안 됨)

	/* 실제 object 반환 */
	*((uintptr_t*)object) = (uintptr_t)slab->free_list;
	slab->free_list = (void*)object;
	slab->free_count++;

	// object가 2KB인 경우에는 object_capacity가 1이기 때문에 발생 가능
	// full -> empty가 된 경우
	if (kmem_cache->obj_capacity == 1) {
		slab_t* prev = kmem_cache->full;

		// slab을 full list에서 제거
		if (prev != slab) {
			while (prev->next != slab) prev = prev->next;

			prev->next = prev->next->next;
		} else {
			kmem_cache->full = prev->next;
		}

		// empty가 없을 경우
		if (kmem_cache->empty == NULL) kmem_cache->empty = slab;
		// empty가 있을 경우
		else pmm_free((void*)virt_to_phys((void*)slab), 0);	// 4KB 페이지 반환
	} // partial -> empty가 된 경우
	else if (slab->free_count == kmem_cache->obj_capacity) {
		slab_t* prev = kmem_cache->partial;

		// slab을 partial list에서 제거
		if (prev != slab) {
			while (prev->next != slab) prev = prev->next;

			prev->next = prev->next->next;
		} else {
			kmem_cache->partial = prev->next;
		}

		// empty가 없을 경우
		if (kmem_cache->empty == NULL) kmem_cache->empty = slab;
		// empty가 있을 경우
		else pmm_free((void*)virt_to_phys((void*)slab), 0);	// 4KB 페이지 반환
	} // full -> partial이 된 경우
	else if (slab->free_count == 1) {
		slab_t* prev = kmem_cache->full;

		// slab을 full list에서 제거
		if (prev != slab) {
			while (prev->next != slab) prev = prev->next;

			prev->next = prev->next->next;
		} else {
			kmem_cache->full = prev->next;
		}

		// partial에 추가
		slab->next = kmem_cache->partial;
		kmem_cache->partial = slab;
	}
}

void* kmalloc(uint64_t size) {
	void* object = NULL;

	int i = 0;
	for (i = 0; i < KMEM_CACHE_COUNT; i++) {
		if (size <= (8ULL << i)) break;
	}

	// 8B ~ 2KB 담당
	if (i < KMEM_CACHE_COUNT) {
		object = object_alloc(&kmem_cache_list[i]);
	} // 2KB 이상은 직접 pmm_alloc으로 할당
	else {
	}

	return object;
}

void kfree(void* object) {

}

void slab_init(kmem_cache_t* kmem_cache, slab_t* slab) {
	uint64_t obj_size = kmem_cache->obj_size;
	uint64_t obj_capacity = kmem_cache->obj_capacity;
	uint64_t header_size = (uint64_t)sizeof(slab_t);

	slab->kmem_cache = kmem_cache;
	slab->next = NULL;
	slab->free_count = obj_capacity;
	if (header_size % obj_size == 0)	// header가 obj 크기와 딱 맞을 경우
		slab->free_list = (uint8_t*)slab + header_size;
	else	// header가 obj 크기와 다를 경우; (header_size / obj_size + 1)을 통해 header가 obj보다 작을 경우도 산정
		slab->free_list = (uint8_t*)slab + obj_size * (header_size / obj_size + 1);

	void* prev = slab->free_list;

	for (uint64_t i = 0; i < slab->kmem_cache->obj_capacity - 1; i++) {
		*((uintptr_t*)prev) = (uintptr_t)((uint8_t*)prev + slab->kmem_cache->obj_size);
		prev = (void*)*((uintptr_t*)prev);
	}
	*((uintptr_t*)prev) = 0;
}

void kmem_cache_init() {
	uint64_t sizes[KMEM_CACHE_COUNT] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};	// 9개

	for (int i = 0; i < KMEM_CACHE_COUNT; i++) {
		kmem_cache_list[i].obj_size = sizes[i];
		// 패딩을 위한 로직
		if (sizeof(slab_t) % sizes[i] == 0)
			kmem_cache_list[i].obj_capacity = (PAGE_SIZE - sizeof(slab_t)) / sizes[i];
		else
			kmem_cache_list[i].obj_capacity = (PAGE_SIZE - (sizes[i] * (sizeof(slab_t) / sizes[i] + 1))) / sizes[i];
		kmem_cache_list[i].full = NULL;
		kmem_cache_list[i].partial = NULL;
		kmem_cache_list[i].empty = NULL;
	}
}
