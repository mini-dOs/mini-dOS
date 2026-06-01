#include <stdio.h>
#include <string.h>
#include "file_internal.h"

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!ptr || !stream || size == 0 || nmemb == 0) return 0;

    size_t total = size * nmemb;

    if (stream->backend == FILE_BACKEND_MEMORY) {
        size_t remain = stream->mem.size - stream->mem.pos;
        size_t to_copy = total < remain ? total : remain;
        memcpy(ptr, stream->mem.base + stream->mem.pos, to_copy);
        stream->mem.pos += to_copy;
        if (to_copy < total) stream->flags |= FILE_FLAG_EOF;
        return to_copy / size;
    }

    if (stream->backend == FILE_BACKEND_STREAM && stream->stream.read) {
        int rc = stream->stream.read(stream->stream.ctx, ptr, total);
        if (rc < 0) { stream->flags |= FILE_FLAG_ERR; return 0; }
        if ((size_t)rc < total) stream->flags |= FILE_FLAG_EOF;
        return (size_t)rc / size;
    }

    return 0;
}
