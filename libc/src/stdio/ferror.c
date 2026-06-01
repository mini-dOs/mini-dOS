#include <stdio.h>
#include "file_internal.h"

int ferror(FILE *stream) {
    if (!stream) return 0;
    return (stream->flags & FILE_FLAG_ERR) != 0;
}
