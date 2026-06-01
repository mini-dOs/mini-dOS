#include <stdio.h>
#include <stdlib.h>
#include "file_internal.h"

FILE *fopen(const char *path, const char *mode) {
    if (!path || !mode) return NULL;

    int want_write = 0;
    for (const char *m = mode; *m; m++) {
        if (*m == 'w' || *m == 'a' || *m == '+') { want_write = 1; break; }
    }
    if (want_write) return NULL;

    const libc_module_t *mod = libc_lookup_module(path);
    if (!mod) return NULL;

    FILE *f = malloc(sizeof(*f));
    if (!f) return NULL;
    f->backend  = FILE_BACKEND_MEMORY;
    f->flags    = FILE_FLAG_READ;
    f->mem.base = mod->base;
    f->mem.size = mod->size;
    f->mem.pos  = 0;
    return f;
}
