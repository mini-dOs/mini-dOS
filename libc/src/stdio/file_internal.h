#ifndef _LIBC_STDIO_FILE_INTERNAL_H
#define _LIBC_STDIO_FILE_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#define FILE_FLAG_EOF    0x01
#define FILE_FLAG_ERR    0x02
#define FILE_FLAG_READ   0x04
#define FILE_FLAG_WRITE  0x08

typedef enum {
    FILE_BACKEND_MEMORY = 1,
    FILE_BACKEND_STREAM = 2,
} file_backend_t;

typedef int (*stream_write_fn)(void *ctx, const char *buf, size_t n);
typedef int (*stream_read_fn)(void *ctx, char *buf, size_t n);

struct _FILE {
    file_backend_t backend;
    unsigned int   flags;

    union {
        struct {
            const uint8_t *base;
            size_t         size;
            size_t         pos;
        } mem;

        struct {
            stream_write_fn write;
            stream_read_fn  read;
            void           *ctx;
        } stream;
    };
};

#define LIBC_MODULE_MAX 8

typedef struct {
    const char *name;
    const void *base;
    size_t      size;
} libc_module_t;

const libc_module_t *libc_lookup_module(const char *name);

typedef void (*libc_emit_fn)(void *ctx, const char *s, size_t n);
int _libc_vformat(libc_emit_fn emit, void *ctx, const char *fmt, __builtin_va_list ap);

#endif
