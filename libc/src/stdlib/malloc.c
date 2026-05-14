#include <stdlib.h>
#include <mm/slab.h>
#include <mm/vmalloc.h>

void *malloc(size_t size) {
    if (size <= 2048)
        return kmalloc(size);
    return vmalloc(size);
}
