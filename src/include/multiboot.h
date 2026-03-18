#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>
#include <multiboot2.h>

typedef struct {
    uint64_t start;
    uint64_t end;
} memory_region_t;

extern memory_region_t *usable_regions;
extern uint32_t usable_region_count;

void multiboot_parse(void *mb_info);

#endif