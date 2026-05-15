#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

int atoi(const char *nptr) {
    int negative = 0;
    int overflow = 0;
    const char *p = nptr;
    unsigned long acc = 0;

    while (*p != '\0' && isspace((unsigned char)*p))
        p++;
    
    if (*p == '-' || *p == '+') {
        negative = (*p == '-');
        p++;
    }
    
    unsigned long cutoff = negative ? (unsigned long)LONG_MIN
                                    : (unsigned long)LONG_MAX;
    unsigned long cutlim = cutoff % 10;
    cutoff /= 10;

    for (; *p != '\0' && isdigit((unsigned char)*p); p++) {
        unsigned int digit = *p - '0';
        if (overflow)
            continue;
        if (acc > cutoff || (acc == cutoff && digit > cutlim)) {
            overflow = 1;
            continue;
        }
        acc = acc * 10 + digit;
    }

    long result;
    if (overflow)
        result = negative ? LONG_MIN : LONG_MAX;
    else if (negative && acc == (unsigned long)LONG_MIN)
        result = LONG_MIN;
    else
        result = negative ? -(long)acc : (long)acc;

    return (int)result;
}
