#include <string.h>

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

void *memset(void *dest, int c, size_t n) {
    unsigned char *d = dest;

    for (size_t i = 0; i < n; i++) {
        d[i] = (unsigned char)c;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1;
    const unsigned char *b = s2;

    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return a[i] - b[i];
        }
    }

    return 0;
}