#include <console.h>
#include <framebuffer.h>
#include <keyboard.h>
#include <multiboot.h>
#include <stdint.h>

cursor_t cursor;

static char	g_line_buf[BUF_SIZE];
static uint16_t	g_line_len;
static uint32_t	g_color;
static uint32_t	g_scale;

void console_readline() {
	char c;

	do {
		c = keyboard_dequeue();

		if (c == '\0') continue;

		if (c == '\n') break;
		else if (c == '\b') {
		} // 오직 알파벳, 숫자, 특수문자, 공백만
		else if (c > 0x1F && c < 0x7F) {
			// line_buf가 가득차면 입력 안 받음
			if (g_line_len < BUF_SIZE - 1) {
				g_line_buf[g_line_len] = c;
				g_line_len++;
				console_putchar(cursor.x, cursor.y, c, g_color, g_scale);
			}
		}
	} while (1);
}

void console_putchar(uint32_t x, uint32_t y, char c, uint32_t color, uint32_t scale) {
	
}

void console_puts(uint32_t x, uint32_t y, char* str, uint32_t color, uint32_t scale) {
}

void console_setter(uint32_t color, uint32_t scale) {
	g_color = color;
	g_scale = scale;
}
