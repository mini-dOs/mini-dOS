#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>  // size_t

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#endif