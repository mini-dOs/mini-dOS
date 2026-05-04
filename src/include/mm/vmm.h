#ifndef VMM_H
#define VMM_H

#include <stdint.h>

void vmm_init(void);
int  vmm_map(uint64_t va, uint64_t pa, uint64_t size, uint64_t flags);
void vmm_unmap(uint64_t va, uint64_t size);

#endif // VMM_H
