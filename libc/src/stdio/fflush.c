#include <stdio.h>
#include "file_internal.h"

int fflush(FILE *stream) {
    (void)stream;
    return 0;
}
