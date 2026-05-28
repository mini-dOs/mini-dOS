#include <ctype.h>
#include <strings.h>

int strncasecmp(const char *s1, const char *s2, size_t n) {
    while (n) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2)
            return c1 - c2;
        if (c1 == 0)
            return 0;
        s1++;
        s2++;
        n--;
    }
    return 0;
}
