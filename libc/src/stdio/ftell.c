#include <stdio.h>
#include "file_internal.h"

long ftell(FILE *stream) {
    if (!stream || stream->backend != FILE_BACKEND_MEMORY) return -1;
    return (long)stream->mem.pos;
}
