#include <early_alloc.h>
#include <cpu.h>

extern uint8_t _kernel_end[];            // End of kernel image (from linker)

static uint8_t *early_current;  // Next allocation position
static uint8_t *early_end;      // Allocation limit

// Round address up to the next multiple of 'align' (power of two)
static uintptr_t align_up(uintptr_t addr, size_t align) {
    return (addr + align - 1) & ~(align - 1);
}

void init_early_alloc(void) {
    early_current = _kernel_end;
    early_end = early_current + EARLY_ALLOC_SIZE; // Reserve 1MB for early allocations
}

// Simple bump allocator: linear allocation, no free
void *early_alloc(size_t size, size_t align) {
    uintptr_t curr = (uintptr_t)early_current;

    // Ensure returned address satisfies alignment
    curr = align_up(curr, align);

    uintptr_t next = curr + size;

    // Stop if out of reserved memory
    if (next > (uintptr_t)early_end) {
        hlt();
    }

    early_current = (uint8_t *)next;
    
    return (void *)curr;
}
