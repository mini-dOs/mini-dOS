#include <stdio.h>
#include <errno.h>

int remove(const char *path) {
    (void)path;
    errno = EROFS;
    return -1;
}
