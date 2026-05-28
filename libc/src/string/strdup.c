#include <string.h>
#include <stdlib.h>

char *strdup(const char *s) {
    size_t len = strlen(s);
    char *dup = malloc(len + 1);
    if (!dup)
        return NULL;
    memcpy(dup, s, len + 1);
    return dup;
}
