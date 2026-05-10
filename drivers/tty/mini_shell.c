#include <drivers/console_driver.h>
#include <drivers/font.h>
#include <drivers/framebuffer.h>
#include <drivers/mini_shell.h>
#include <stdint.h>

uint32_t system_font_color;
uint32_t system_bg_color;
uint32_t system_scale;
system_cursor_t system_cursor;

void shell_print_account(const char* account) {
	console_puts(cursor.x, cursor.y, account, system_font_color, system_bg_color, system_scale);
}

void shell_print_dir(const char* dir) {
	console_puts(cursor.x, cursor.y, dir, system_font_color, system_bg_color, system_scale);
	console_puts(cursor.x, cursor.y, "$ ", system_font_color, system_bg_color, system_scale);
}

void shell_wait() {
	// 안내 문구 출력 위치 설정
	cursor.x = system_cursor.x;
	cursor.y += system_scale * FONT_HEIGHT;

	// 안내 문구 출력
	shell_print_account("dOS:");
	shell_print_dir("~");

	// 사용자 입력
	console_readline();
}

void shell_init() {
	system_font_color = 0x0085D597;
	system_bg_color = 0x001E1E1E;
	system_scale = 2;
	system_cursor.x = 0;
	system_cursor.y = 0;

	cursor.x = system_cursor.x;
	cursor.y = system_cursor.y;

	fb_clear(system_bg_color);
	console_setter(0x00FFFFFF, 0x001E1E1E, 2);
}
