#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX     0x7FFFFFFF

#define MB_CUR_MAX 1

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;

extern void *malloc(size_t size);

#endif
