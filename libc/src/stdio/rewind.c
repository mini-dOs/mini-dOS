#include <stdio.h>
#include "file_internal.h"

void rewind(FILE *stream) {
    if (!stream) return;
    if (stream->backend == FILE_BACKEND_MEMORY) {
        stream->mem.pos = 0;
    }
    stream->flags &= ~(FILE_FLAG_EOF | FILE_FLAG_ERR);
}
