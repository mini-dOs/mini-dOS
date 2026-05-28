#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

static const double pow10_table[23] = {
    1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
    1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
    1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22,
};

static double pow10_positive(int exp) {
    double result = 1.0;
    while (exp >= 22) {
        result *= 1e22;
        exp -= 22;
    }
    if (exp > 0)
        result *= pow10_table[exp];
    return result;
}

double atof(const char *nptr) {
    const char *p = nptr;
    int negative = 0;
    uint64_t mantissa = 0;
    int frac_digits = 0;
    int extra_int_digits = 0;
    int seen_digit = 0;

    while (*p != '\0' && isspace((unsigned char)*p))
        p++;

    if (*p == '-' || *p == '+') {
        negative = (*p == '-');
        p++;
    }

    while (isdigit((unsigned char)*p)) {
        seen_digit = 1;
        if (mantissa < ((uint64_t)-1 - 9) / 10)
            mantissa = mantissa * 10 + (*p - '0');
        else
            extra_int_digits++;
        p++;
    }

    if (*p == '.') {
        p++;
        while (isdigit((unsigned char)*p)) {
            seen_digit = 1;
            if (mantissa < ((uint64_t)-1 - 9) / 10) {
                mantissa = mantissa * 10 + (*p - '0');
                frac_digits++;
            }
            p++;
        }
    }

    if (!seen_digit)
        return 0.0;

    int exp_value = 0;
    if (*p == 'e' || *p == 'E') {
        p++;
        int exp_negative = 0;
        if (*p == '-' || *p == '+') {
            exp_negative = (*p == '-');
            p++;
        }
        while (isdigit((unsigned char)*p)) {
            if (exp_value < 10000)
                exp_value = exp_value * 10 + (*p - '0');
            p++;
        }
        if (exp_negative)
            exp_value = -exp_value;
    }

    int total_exp = exp_value + extra_int_digits - frac_digits;
    double result;
    if (total_exp >= 0)
        result = (double)mantissa * pow10_positive(total_exp);
    else
        result = (double)mantissa / pow10_positive(-total_exp);

    return negative ? -result : result;
}
