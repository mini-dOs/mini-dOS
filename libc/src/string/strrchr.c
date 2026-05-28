#include <stddef.h>
#include <string.h>

char *strrchr(const char *s, int c) {
    size_t i = 0;
    const char *last = NULL;
    while (1) {
        if ((unsigned char)s[i] == (unsigned char)c)
            last = &s[i];
        if (s[i] == '\0')
            break;
        i++;
    }
    return (char *)last;
}
