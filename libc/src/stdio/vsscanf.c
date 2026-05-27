#include <stdio.h>
#include <stddef.h>

static int is_ws(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static void skip_ws(const char **s) {
    while (is_ws(**s)) (*s)++;
}

int vsscanf(const char *str, const char *fmt, va_list ap) {
    int count = 0;
    const char *s = str;

    while (*fmt) {
        if (is_ws(*fmt)) { skip_ws(&s); fmt++; continue; }

        if (*fmt != '%') {
            if (*s != *fmt) return count;
            s++; fmt++;
            continue;
        }
        fmt++;

        int suppress = 0;
        if (*fmt == '*') { suppress = 1; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') fmt++;

        char conv = *fmt;
        if (conv) fmt++;

        if (conv != 'c' && conv != '[' && conv != 'n' && conv != '%') skip_ws(&s);

        switch (conv) {
            case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': {
                int sign = 0;
                if (conv == 'd' || conv == 'i' || conv == 'u') {
                    if (*s == '+') s++;
                    else if (*s == '-') { sign = 1; s++; }
                }
                int base;
                if (conv == 'd' || conv == 'u') base = 10;
                else if (conv == 'o') base = 8;
                else if (conv == 'x' || conv == 'X') {
                    base = 16;
                    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
                } else {
                    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; s += 2; }
                    else if (s[0] == '0') base = 8;
                    else base = 10;
                }
                const char *start = s;
                unsigned long val = 0;
                while (*s) {
                    int d;
                    if (*s >= '0' && *s <= '9') d = *s - '0';
                    else if (*s >= 'a' && *s <= 'f') d = *s - 'a' + 10;
                    else if (*s >= 'A' && *s <= 'F') d = *s - 'A' + 10;
                    else break;
                    if (d >= base) break;
                    val = val * base + (unsigned long)d;
                    s++;
                }
                if (s == start) return count;
                if (!suppress) {
                    int *p = va_arg(ap, int *);
                    *p = sign ? -(int)val : (int)val;
                    count++;
                }
                break;
            }
            case 's': {
                const char *start = s;
                while (*s && !is_ws(*s)) s++;
                if (s == start) return count;
                if (!suppress) {
                    char *p = va_arg(ap, char *);
                    const char *q = start;
                    while (q < s) *p++ = *q++;
                    *p = '\0';
                    count++;
                }
                break;
            }
            case 'c': {
                if (!*s) return count;
                if (!suppress) {
                    char *p = va_arg(ap, char *);
                    *p = *s;
                    count++;
                }
                s++;
                break;
            }
            case '%': {
                if (*s != '%') return count;
                s++;
                break;
            }
            case 0:
                return count;
            default:
                return count;
        }
    }
    return count;
}
