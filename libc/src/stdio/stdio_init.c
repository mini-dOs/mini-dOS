#include <stdio.h>
#include <stddef.h>
#include <drivers/serial.h>
#include "file_internal.h"

static int serial_write_cb(void *ctx, const char *s, size_t n) {
    (void)ctx;
    for (size_t i = 0; i < n; i++) serial_write_char(s[i]);
    return (int)n;
}

static struct _FILE _stdin_obj = {
    .backend = FILE_BACKEND_STREAM,
    .flags   = FILE_FLAG_READ,
    .stream  = { NULL, NULL, NULL },
};

static struct _FILE _stdout_obj = {
    .backend = FILE_BACKEND_STREAM,
    .flags   = FILE_FLAG_WRITE,
    .stream  = { serial_write_cb, NULL, NULL },
};

static struct _FILE _stderr_obj = {
    .backend = FILE_BACKEND_STREAM,
    .flags   = FILE_FLAG_WRITE,
    .stream  = { serial_write_cb, NULL, NULL },
};

FILE *stdin  = &_stdin_obj;
FILE *stdout = &_stdout_obj;
FILE *stderr = &_stderr_obj;

void libc_stdio_init(void) {
}
