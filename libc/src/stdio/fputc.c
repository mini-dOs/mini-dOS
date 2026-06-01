#include <stdio.h>
#include "file_internal.h"

int fputc(int c, FILE *stream) {
    if (!stream || stream->backend != FILE_BACKEND_STREAM || !stream->stream.write) return EOF;
    char ch = (char)c;
    int rc = stream->stream.write(stream->stream.ctx, &ch, 1);
    if (rc < 0) {
        stream->flags |= FILE_FLAG_ERR;
        return EOF;
    }
    return (unsigned char)ch;
}
