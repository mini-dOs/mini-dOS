#include <stdio.h>

int sprintf(char *str, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsprintf(str, fmt, ap);
    va_end(ap);
    return n;
}
