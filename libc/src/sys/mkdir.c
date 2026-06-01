#include <sys/stat.h>
#include <errno.h>

int mkdir(const char *path, mode_t mode) {
    (void)path;
    (void)mode;
    errno = EROFS;
    return -1;
}
