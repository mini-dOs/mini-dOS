#include <stdio.h>
#include "file_internal.h"

int feof(FILE *stream) {
    if (!stream) return 0;
    return (stream->flags & FILE_FLAG_EOF) != 0;
}
