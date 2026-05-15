#include <math.h>
#include <stdint.h>

double fabs(double x) {
    union {
        double d;
        uint64_t u;
    } v = { x };
    v.u &= 0x7FFFFFFFFFFFFFFFULL;
    return v.d;
}
