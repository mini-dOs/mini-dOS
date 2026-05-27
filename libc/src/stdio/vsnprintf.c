#include <stdio.h>
#include "file_internal.h"

struct snprintf_ctx {
    char  *buf;
    size_t size;
    size_t pos;
};

static void snprintf_emit(void *vctx, const char *s, size_t n) {
    struct snprintf_ctx *c = vctx;
    if (c->size > 0 && c->pos < c->size - 1) {
        size_t room = c->size - 1 - c->pos;
        size_t cp = n < room ? n : room;
        for (size_t i = 0; i < cp; i++) c->buf[c->pos + i] = s[i];
    }
    c->pos += n;
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list ap) {
    struct snprintf_ctx ctx = { str, size, 0 };
    int n = _libc_vformat(snprintf_emit, &ctx, fmt, ap);
    if (size > 0) {
        size_t term = ctx.pos < size - 1 ? ctx.pos : size - 1;
        str[term] = '\0';
    }
    return n;
}
