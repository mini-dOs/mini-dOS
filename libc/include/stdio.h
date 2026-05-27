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

extern int    printf(const char *fmt, ...);
extern int    fprintf(FILE *stream, const char *fmt, ...);
extern int    sprintf(char *str, const char *fmt, ...);
extern int    snprintf(char *str, size_t size, const char *fmt, ...);
extern int    vprintf(const char *fmt, va_list ap);
extern int    vfprintf(FILE *stream, const char *fmt, va_list ap);
extern int    vsprintf(char *str, const char *fmt, va_list ap);
extern int    vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
extern int    sscanf(const char *str, const char *fmt, ...);
extern int    vsscanf(const char *str, const char *fmt, va_list ap);

extern int    puts(const char *s);
extern int    putchar(int c);
extern int    fputs(const char *s, FILE *stream);
extern int    fputc(int c, FILE *stream);
extern int    putc(int c, FILE *stream);

extern int    fgetc(FILE *stream);
extern int    getc(FILE *stream);
extern char  *fgets(char *s, int size, FILE *stream);

extern FILE  *fopen(const char *path, const char *mode);
extern int    fclose(FILE *stream);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int    fseek(FILE *stream, long offset, int whence);
extern long   ftell(FILE *stream);
extern void   rewind(FILE *stream);
extern int    fflush(FILE *stream);
extern int    feof(FILE *stream);
extern int    ferror(FILE *stream);
extern void   clearerr(FILE *stream);
extern int    fileno(FILE *stream);
extern void   setbuf(FILE *stream, char *buf);
extern int    setvbuf(FILE *stream, char *buf, int mode, size_t size);

extern int    remove(const char *path);
extern int    rename(const char *oldpath, const char *newpath);

extern void   perror(const char *s);

extern void   libc_stdio_init(void);
extern void   libc_register_module(const char *name, const void *base, size_t size);

#endif
