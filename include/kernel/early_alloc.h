#ifndef EARLY_ALLOC_H
#define EARLY_ALLOC_H

#include <stdint.h>
#include <stddef.h>

#define EARLY_ALLOC_SIZE 0x100000

void early_alloc_init(void);
void *early_alloc(size_t size, size_t align);

#endif
