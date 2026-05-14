#include <ctype.h>
#include <stddef.h>
#include <strings.h>

int strcasecmp(const char *s1, const char *s2) {
    while (1) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2)
            return c1 - c2;
        if (c1 == 0)
            return 0;
        s1++;
        s2++;
    }
}
