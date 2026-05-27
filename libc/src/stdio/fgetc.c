#include <stdio.h>
#include "file_internal.h"

int fgetc(FILE *stream) {
    if (!stream) return EOF;

    if (stream->backend == FILE_BACKEND_MEMORY) {
        if (stream->mem.pos >= stream->mem.size) {
            stream->flags |= FILE_FLAG_EOF;
            return EOF;
        }
        return stream->mem.base[stream->mem.pos++];
    }

    if (stream->backend == FILE_BACKEND_STREAM && stream->stream.read) {
        char c;
        int rc = stream->stream.read(stream->stream.ctx, &c, 1);
        if (rc <= 0) {
            if (rc == 0) stream->flags |= FILE_FLAG_EOF;
            else         stream->flags |= FILE_FLAG_ERR;
            return EOF;
        }
        return (unsigned char)c;
    }

    return EOF;
}
