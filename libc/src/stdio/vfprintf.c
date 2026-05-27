#include <stdio.h>
#include "file_internal.h"

struct vfprintf_ctx {
    FILE *stream;
    int   error;
};

static void vfprintf_emit(void *vctx, const char *s, size_t n) {
    struct vfprintf_ctx *c = vctx;
    FILE *f = c->stream;
    if (f->backend == FILE_BACKEND_STREAM && f->stream.write) {
        int rc = f->stream.write(f->stream.ctx, s, n);
        if (rc < 0) c->error = 1;
    }
}

int vfprintf(FILE *stream, const char *fmt, va_list ap) {
    if (!stream) return -1;
    struct vfprintf_ctx ctx = { stream, 0 };
    int n = _libc_vformat(vfprintf_emit, &ctx, fmt, ap);
    if (ctx.error) {
        stream->flags |= FILE_FLAG_ERR;
        return -1;
    }
    return n;
}
