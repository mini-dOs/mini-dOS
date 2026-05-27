#include <stdio.h>
#include "file_internal.h"

int fputs(const char *s, FILE *stream) {
    if (!stream || stream->backend != FILE_BACKEND_STREAM || !stream->stream.write) return EOF;
    size_t n = 0;
    while (s[n]) n++;
    int rc = stream->stream.write(stream->stream.ctx, s, n);
    if (rc < 0) {
        stream->flags |= FILE_FLAG_ERR;
        return EOF;
    }
    return rc;
}
