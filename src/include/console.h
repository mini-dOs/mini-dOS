#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define BUF_SIZE	4096

typedef struct cursor {
	uint32_t x;
	uint32_t y;
} cursor_t;

extern cursor_t cursor;

void console_readline();
void console_putchar(uint32_t x, uint32_t y, char c, uint32_t color, uint32_t scale);
void console_puts(uint32_t x, uint32_t y, char* str, uint32_t color, uint32_t scale);
void console_setter(uint32_t color, uint32_t scale);

#endif
