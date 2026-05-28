#include <stddef.h>
#include <string.h>

char *strchr(const char *s, int c) {
    size_t i = 0;
    while (1) {
        if ((unsigned char)s[i] == (unsigned char)c)
            return (char *)(s + i);
        if (s[i] == '\0')
            return NULL;
        i++;
    }
}
