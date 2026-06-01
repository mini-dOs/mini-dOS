#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "file_internal.h"

static libc_module_t modules[LIBC_MODULE_MAX];
static size_t module_count = 0;

void libc_register_module(const char *name, const void *base, size_t size) {
    if (module_count >= LIBC_MODULE_MAX) return;
    modules[module_count].name = name;
    modules[module_count].base = base;
    modules[module_count].size = size;
    module_count++;
}

static const char *basename_p(const char *p) {
    const char *b = p;
    for (const char *q = p; *q; q++) {
        if (*q == '/' || *q == '\\') b = q + 1;
    }
    return b;
}

const libc_module_t *libc_lookup_module(const char *name) {
    if (!name) return NULL;
    const char *qn = basename_p(name);
    for (size_t i = 0; i < module_count; i++) {
        if (!modules[i].name) continue;
        if (strcasecmp(modules[i].name, name) == 0) return &modules[i];
        const char *bn = basename_p(modules[i].name);
        if (strcasecmp(bn, qn) == 0) return &modules[i];
    }
    return NULL;
}
