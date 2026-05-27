#include "file_internal.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define FLAG_LEFT   0x01
#define FLAG_PLUS   0x02
#define FLAG_SPACE  0x04
#define FLAG_ZERO   0x08
#define FLAG_HASH   0x10

typedef enum {
    LEN_NONE,
    LEN_HH, LEN_H,
    LEN_L, LEN_LL,
    LEN_Z, LEN_T,
} len_mod_t;

static size_t emit_char(libc_emit_fn emit, void *ctx, char c) {
    emit(ctx, &c, 1);
    return 1;
}

static size_t emit_repeat(libc_emit_fn emit, void *ctx, char c, size_t n) {
    char buf[32];
    size_t total = n;
    while (n > 0) {
        size_t chunk = n > sizeof(buf) ? sizeof(buf) : n;
        for (size_t i = 0; i < chunk; i++) buf[i] = c;
        emit(ctx, buf, chunk);
        n -= chunk;
    }
    return total;
}

static size_t format_uint(uintmax_t value, unsigned base, int upper, char *out) {
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[32];
    size_t n = 0;
    if (value == 0) tmp[n++] = '0';
    while (value > 0) {
        tmp[n++] = digits[value % base];
        value /= base;
    }
    for (size_t i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    return n;
}

int _libc_vformat(libc_emit_fn emit, void *ctx, const char *fmt, va_list ap) {
    size_t total = 0;

    while (*fmt) {
        if (*fmt != '%') {
            const char *start = fmt;
            while (*fmt && *fmt != '%') fmt++;
            size_t n = (size_t)(fmt - start);
            emit(ctx, start, n);
            total += n;
            continue;
        }
        fmt++;

        unsigned flags = 0;
        for (;;) {
            switch (*fmt) {
                case '-': flags |= FLAG_LEFT;  fmt++; continue;
                case '+': flags |= FLAG_PLUS;  fmt++; continue;
                case ' ': flags |= FLAG_SPACE; fmt++; continue;
                case '0': flags |= FLAG_ZERO;  fmt++; continue;
                case '#': flags |= FLAG_HASH;  fmt++; continue;
            }
            break;
        }

        int width = 0;
        if (*fmt == '*') {
            width = va_arg(ap, int);
            if (width < 0) { flags |= FLAG_LEFT; width = -width; }
            fmt++;
        } else {
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
        }

        int precision = -1;
        if (*fmt == '.') {
            fmt++;
            precision = 0;
            if (*fmt == '*') {
                precision = va_arg(ap, int);
                fmt++;
            } else {
                while (*fmt >= '0' && *fmt <= '9') {
                    precision = precision * 10 + (*fmt - '0');
                    fmt++;
                }
            }
        }

        len_mod_t len = LEN_NONE;
        if (*fmt == 'h') {
            fmt++;
            if (*fmt == 'h') { len = LEN_HH; fmt++; }
            else len = LEN_H;
        } else if (*fmt == 'l') {
            fmt++;
            if (*fmt == 'l') { len = LEN_LL; fmt++; }
            else len = LEN_L;
        } else if (*fmt == 'z') { len = LEN_Z; fmt++; }
        else if (*fmt == 't') { len = LEN_T; fmt++; }

        char conv = *fmt;
        if (conv) fmt++;

        char numbuf[32];
        size_t numlen = 0;
        const char *prefix = "";
        size_t prefix_len = 0;

        switch (conv) {
            case 'd': case 'i': {
                intmax_t v;
                switch (len) {
                    case LEN_LL: v = va_arg(ap, long long); break;
                    case LEN_L:  v = va_arg(ap, long); break;
                    case LEN_Z:  v = (intmax_t)va_arg(ap, size_t); break;
                    case LEN_T:  v = (intmax_t)va_arg(ap, ptrdiff_t); break;
                    case LEN_H:  v = (short)va_arg(ap, int); break;
                    case LEN_HH: v = (signed char)va_arg(ap, int); break;
                    default:     v = va_arg(ap, int); break;
                }
                uintmax_t uv;
                if (v < 0) {
                    prefix = "-"; prefix_len = 1;
                    uv = (uintmax_t)(-(v + 1)) + 1;
                } else {
                    uv = (uintmax_t)v;
                    if (flags & FLAG_PLUS) { prefix = "+"; prefix_len = 1; }
                    else if (flags & FLAG_SPACE) { prefix = " "; prefix_len = 1; }
                }
                numlen = format_uint(uv, 10, 0, numbuf);
                if (precision >= 0) flags &= ~FLAG_ZERO;
                break;
            }
            case 'u': case 'o': case 'x': case 'X': {
                uintmax_t uv;
                switch (len) {
                    case LEN_LL: uv = va_arg(ap, unsigned long long); break;
                    case LEN_L:  uv = va_arg(ap, unsigned long); break;
                    case LEN_Z:  uv = va_arg(ap, size_t); break;
                    case LEN_T:  uv = (uintmax_t)va_arg(ap, ptrdiff_t); break;
                    case LEN_H:  uv = (unsigned short)va_arg(ap, unsigned int); break;
                    case LEN_HH: uv = (unsigned char)va_arg(ap, unsigned int); break;
                    default:     uv = va_arg(ap, unsigned int); break;
                }
                unsigned base = (conv == 'o') ? 8 : (conv == 'u') ? 10 : 16;
                int upper = (conv == 'X');
                numlen = format_uint(uv, base, upper, numbuf);
                if ((flags & FLAG_HASH) && uv != 0) {
                    if (conv == 'x') { prefix = "0x"; prefix_len = 2; }
                    else if (conv == 'X') { prefix = "0X"; prefix_len = 2; }
                    else if (conv == 'o') { prefix = "0"; prefix_len = 1; }
                }
                if (precision >= 0) flags &= ~FLAG_ZERO;
                break;
            }
            case 'c': {
                char c = (char)va_arg(ap, int);
                int pad = width > 1 ? width - 1 : 0;
                if (!(flags & FLAG_LEFT)) total += emit_repeat(emit, ctx, ' ', (size_t)pad);
                total += emit_char(emit, ctx, c);
                if (flags & FLAG_LEFT) total += emit_repeat(emit, ctx, ' ', (size_t)pad);
                continue;
            }
            case 's': {
                const char *s = va_arg(ap, const char *);
                if (!s) s = "(null)";
                size_t slen = 0;
                while (s[slen] && (precision < 0 || slen < (size_t)precision)) slen++;
                int pad = width > (int)slen ? width - (int)slen : 0;
                if (!(flags & FLAG_LEFT)) total += emit_repeat(emit, ctx, ' ', (size_t)pad);
                emit(ctx, s, slen); total += slen;
                if (flags & FLAG_LEFT) total += emit_repeat(emit, ctx, ' ', (size_t)pad);
                continue;
            }
            case 'p': {
                uintmax_t uv = (uintmax_t)(uintptr_t)va_arg(ap, void *);
                prefix = "0x"; prefix_len = 2;
                numlen = format_uint(uv, 16, 0, numbuf);
                break;
            }
            case '%': {
                total += emit_char(emit, ctx, '%');
                continue;
            }
            case 0: {
                return (int)total;
            }
            default: {
                total += emit_char(emit, ctx, '%');
                total += emit_char(emit, ctx, conv);
                continue;
            }
        }

        size_t zeros = 0;
        if (precision >= 0 && (size_t)precision > numlen) zeros = (size_t)precision - numlen;
        size_t content = prefix_len + zeros + numlen;
        size_t spaces = (size_t)width > content ? (size_t)width - content : 0;

        if (!(flags & FLAG_LEFT)) {
            if (flags & FLAG_ZERO) {
                emit(ctx, prefix, prefix_len); total += prefix_len;
                total += emit_repeat(emit, ctx, '0', spaces + zeros);
                emit(ctx, numbuf, numlen); total += numlen;
            } else {
                total += emit_repeat(emit, ctx, ' ', spaces);
                emit(ctx, prefix, prefix_len); total += prefix_len;
                total += emit_repeat(emit, ctx, '0', zeros);
                emit(ctx, numbuf, numlen); total += numlen;
            }
        } else {
            emit(ctx, prefix, prefix_len); total += prefix_len;
            total += emit_repeat(emit, ctx, '0', zeros);
            emit(ctx, numbuf, numlen); total += numlen;
            total += emit_repeat(emit, ctx, ' ', spaces);
        }
    }

    return (int)total;
}
