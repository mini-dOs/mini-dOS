#include <stdio.h>
#include <stdlib.h>
#include "file_internal.h"

int fclose(FILE *stream) {
    if (!stream) return EOF;
    if (stream == stdin || stream == stdout || stream == stderr) return 0;
    free(stream);
    return 0;
}
