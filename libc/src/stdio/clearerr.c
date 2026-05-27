#include <stdio.h>
#include "file_internal.h"

void clearerr(FILE *stream) {
    if (!stream) return;
    stream->flags &= ~(FILE_FLAG_EOF | FILE_FLAG_ERR);
}
