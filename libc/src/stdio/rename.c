#include <stdio.h>
#include <errno.h>

int rename(const char *oldpath, const char *newpath) {
    (void)oldpath;
    (void)newpath;
    errno = EROFS;
    return -1;
}
