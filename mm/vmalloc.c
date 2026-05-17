#include <asm/cpu.h>
#include <drivers/serial.h>
#include <kernel/kernel_info.h>
#include <mm/paging.h>
#include <mm/slab.h>
#include <mm/vmalloc.h>
#include <mm/vmm.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct vm_area {
    uint64_t        addr;
    uint64_t        size;
    bool            is_free;
    struct vm_area *next;
} vm_area_t;

static vm_area_t *head;

void vmalloc_init(void) {
    head = kmalloc(sizeof(vm_area_t));
    if (head == NULL) {
        serial_write("[vmalloc] vmalloc_init OOM\n");
        for (;;)
            hlt();
    }
    head->addr    = VMALLOC_START;
    head->size    = VMALLOC_END - VMALLOC_START;
    head->is_free = true;
    head->next    = NULL;
}

void *vmalloc(uint64_t size) {
    if (size == 0)
        return NULL;
    if (size > (VMALLOC_END - VMALLOC_START))
        return NULL;
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    // first-fit
    vm_area_t *cur = NULL;
    for (cur = head; cur; cur = cur->next) {
        if (cur->is_free == true && cur->size >= size)
            break;
    }

    if (cur == NULL)
        return NULL;

    // split
    bool split_done = false;
    if (cur->size > size) {
        vm_area_t *rest = kmalloc(sizeof(vm_area_t));
        if (rest == NULL) {
            serial_write("[vmalloc] node OOM\n");
            return NULL;
        }
        rest->addr    = cur->addr + size;
        rest->size    = cur->size - size;
        rest->is_free = true;
        rest->next    = cur->next;

        cur->next = rest;
        cur->size = size;

        split_done = true;
    }

    // mapping
    cur->is_free = false;
    int rc = vmm_alloc(cur->addr, cur->size, PAGE_RW | PAGE_NX);

    // rollback on mapping failure
    if (rc < 0) {
        cur->is_free = true;
        if (split_done) {
            vm_area_t *rest = cur->next;
            cur->size += rest->size;
            cur->next  = rest->next;
            kfree(rest);
        }
        return NULL;
    }

    return (void *)cur->addr;
}

void vfree(void *ptr) {
    if (ptr == NULL)
        return;

    vm_area_t *cur  = head;
    vm_area_t *prev = NULL;
    while (cur != NULL) {
        if (cur->addr == (uint64_t)ptr)
            break;
        prev = cur;
        cur  = cur->next;
    }

    if (cur == NULL) {
        serial_write("[vfree] not found\n");
        return;
    }
    if (cur->is_free) {
        serial_write("[vfree] double free\n");
        return;
    }

    // unmapping
    vmm_free(cur->addr, cur->size);
    cur->is_free = true;

    // coalesce
    if (cur->next != NULL && cur->next->is_free) {
        vm_area_t *n = cur->next;
        cur->size   += n->size;
        cur->next    = n->next;
        kfree(n);
    }
    if (prev != NULL && prev->is_free) {
        prev->size += cur->size;
        prev->next  = cur->next;
        kfree(cur);
    }
}

void vmalloc_dump(void) {
    serial_write("[vmalloc dump]\n");
    for (vm_area_t *p = head; p; p = p->next) {
        serial_write("  addr=");
        serial_write_hex64(p->addr);
        serial_write(" size=");
        serial_write_hex64(p->size);
        serial_write(p->is_free ? " free\n" : " used\n");
    }
}
