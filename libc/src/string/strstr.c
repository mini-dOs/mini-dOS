#include <string.h>

char *strstr(const char *haystack, const char *needle) {
    size_t i, j;
    if (*needle == '\0')
        return (char *)haystack;
    for (i = 0; haystack[i] != '\0'; i++) {
        for (j = 0; needle[j] != '\0'; j++) {
            if (haystack[i + j] != needle[j])
                break;
        }
        if (needle[j] == '\0')
            return (char *)(haystack + i);
    }
    return NULL;
}
