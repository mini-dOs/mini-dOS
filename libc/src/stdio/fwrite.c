#include <stdio.h>
#include "file_internal.h"

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!ptr || !stream || size == 0 || nmemb == 0) return 0;

    if (stream->backend == FILE_BACKEND_STREAM && stream->stream.write) {
        size_t total = size * nmemb;
        int rc = stream->stream.write(stream->stream.ctx, ptr, total);
        if (rc < 0) { stream->flags |= FILE_FLAG_ERR; return 0; }
        return (size_t)rc / size;
    }

    stream->flags |= FILE_FLAG_ERR;
    return 0;
}
