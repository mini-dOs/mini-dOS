#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX     0x7FFFFFFF

#define MB_CUR_MAX 1

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;

extern int    abs(int j);
extern double atof(const char *nptr);
extern int    atoi(const char *nptr);
extern void   *calloc(size_t nmemb, size_t size);
extern void   *malloc(size_t size);
extern void   *realloc(void *ptr, size_t size);
extern void   free(void *ptr);
extern void   exit(int status) __attribute__((noreturn));
extern int    system(const char *command);

#endif
