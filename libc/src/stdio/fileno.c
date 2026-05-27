#include <stdio.h>
#include "file_internal.h"

int fileno(FILE *stream) {
    if (!stream) return -1;
    if (stream == stdin)  return 0;
    if (stream == stdout) return 1;
    if (stream == stderr) return 2;
    return 3;
}
