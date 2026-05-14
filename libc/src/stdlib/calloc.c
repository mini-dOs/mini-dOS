#include <stdlib.h>
#include <string.h>

void *calloc(size_t nmemb, size_t size) {
    size_t total;
    if (__builtin_mul_overflow(nmemb, size, &total))
        return NULL;
    void *ptr = malloc(total);
    if (!ptr)
        return NULL;
    memset(ptr, 0, total);
    return ptr;
}
