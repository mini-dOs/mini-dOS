#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

typedef struct _FILE FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define EOF       (-1)
#define BUFSIZ    1024
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2
#define FILENAME_MAX 256

typedef long fpos_t;

#endif
