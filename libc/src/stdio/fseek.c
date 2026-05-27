#include <stdio.h>
#include "file_internal.h"

int fseek(FILE *stream, long offset, int whence) {
    if (!stream || stream->backend != FILE_BACKEND_MEMORY) return -1;

    long base;
    switch (whence) {
        case SEEK_SET: base = 0; break;
        case SEEK_CUR: base = (long)stream->mem.pos; break;
        case SEEK_END: base = (long)stream->mem.size; break;
        default: return -1;
    }

    long new_pos = base + offset;
    if (new_pos < 0 || (size_t)new_pos > stream->mem.size) return -1;
    stream->mem.pos = (size_t)new_pos;
    stream->flags &= ~FILE_FLAG_EOF;
    return 0;
}
