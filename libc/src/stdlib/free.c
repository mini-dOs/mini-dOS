#include <stdlib.h>
#include <kernel/kernel_info.h>
#include <mm/slab.h>
#include <mm/vmalloc.h>

void free(void *ptr) {
  if (!ptr) return;

  if ((uintptr_t)ptr >= VMALLOC_START && (uintptr_t)ptr < VMALLOC_END)
      vfree(ptr);
  else
      kfree(ptr);
}
