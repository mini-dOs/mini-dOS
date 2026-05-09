#include <drivers/console_driver.h>
#include <drivers/font.h>
#include <drivers/framebuffer.h>
#include <drivers/keyboard.h>
#include <drivers/mini_shell.h>
#include <kernel/multiboot.h>
#include <stdint.h>

cursor_t cursor;

static char	g_line_buf[BUF_SIZE];
static uint16_t	g_line_len;
static uint32_t	g_font_color;
static uint32_t	g_bg_color;
static uint32_t	g_scale;

void console_readline() {
	char c;
	uint32_t line_end = fb_get_width() - (fb_get_width() % (g_scale * FONT_WIDTH));

	g_line_len = 0;

	do {
		c = keyboard_dequeue();

		if (c == '\0') continue;

		// 엔터 입력 시 버퍼 마지막에 '\0' 삽입
		if (c == '\n') {
			g_line_buf[g_line_len] = '\0';
			break;
		} else if (c == '\b') {
			if (g_line_len > 0) {
				if (cursor.x < system_cursor.x + g_scale * FONT_WIDTH) {
					cursor.x = line_end;
					// y도 사실 첫째줄인지 검사해야하는데 아직 스크롤이 없어서 고려 안 함
					cursor.y -= g_scale * FONT_HEIGHT;
				}
				cursor.x -= g_scale * FONT_WIDTH;
				
				console_putchar(' ', g_font_color, g_bg_color, g_scale);
				g_line_len--;
			}
		} // 오직 알파벳, 숫자, 특수문자, 공백만
		else if (c > 0x1F && c < 0x7F) {
			// line_buf가 가득차면 입력 안 받음
			if (g_line_len < BUF_SIZE - 1) {
				g_line_buf[g_line_len] = c;
				g_line_len++;

				if (cursor.x + g_scale * FONT_WIDTH - 1 >= fb_get_width()) {
					cursor.x = system_cursor.x;
					cursor.y += g_scale * FONT_HEIGHT;
				}

				console_putchar(c, g_font_color, g_bg_color, g_scale);
				cursor.x += g_scale * FONT_WIDTH;
			}
		}
	} while (1);
}

void console_putchar(char c, uint32_t font_color, uint32_t bg_color, uint32_t scale) {
	fb_write_char(cursor.x, cursor.y, c, font_color, bg_color, scale);
}

void console_puts(uint32_t x, uint32_t y, const char* str, uint32_t font_color, uint32_t bg_color, uint32_t scale) {
	cursor.x = x;
	cursor.y = y;

	while (*str) {
		if (cursor.x + scale * FONT_WIDTH - 1 >= fb_get_width()) {
			cursor.x = x;
			cursor.y += scale * FONT_HEIGHT;
		}

		console_putchar(*str, font_color, bg_color, scale);
		cursor.x += scale * FONT_WIDTH;
		str++;
	}
}

void console_setter(uint32_t font_color, uint32_t bg_color, uint32_t scale) {
	g_font_color = font_color;
	g_bg_color = bg_color;
	g_scale = scale;
}
