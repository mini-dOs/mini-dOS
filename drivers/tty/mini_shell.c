#include <asm/cpu.h>
#include <drivers/console_driver.h>
#include <drivers/font.h>
#include <drivers/framebuffer.h>
#include <drivers/mini_shell.h>
#include <doomgeneric_minidos.h>
#include <kernel/pit.h>
#include <mm/pmm.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t system_font_color;
uint32_t system_bg_color;
uint32_t system_scale;
system_cursor_t system_cursor;

// 우분투 터미널 팔레트(Tango)에 맞춘 프롬프트 색.
#define PROMPT_USER_COLOR 0x008AE234  // user@host: 굵은 초록
#define PROMPT_PATH_COLOR 0x00729FCF  // 경로: 굵은 파랑
#define PROMPT_SEP_COLOR  0x00FFFFFF  // ':' '$' 구분자: 흰색

void shell_print_account(const char* account) {
	console_puts(cursor.x, cursor.y, account, PROMPT_USER_COLOR, system_bg_color, system_scale);
}

void shell_print_dir(const char* dir) {
	console_puts(cursor.x, cursor.y, ":", PROMPT_SEP_COLOR, system_bg_color, system_scale);
	console_puts(cursor.x, cursor.y, dir, PROMPT_PATH_COLOR, system_bg_color, system_scale);
	console_puts(cursor.x, cursor.y, "$ ", PROMPT_SEP_COLOR, system_bg_color, system_scale);
}

// 명령어 출력용: 새 줄로 내려 한 줄을 그린다.
static void shell_println(const char* s) {
	cursor.x = system_cursor.x;
	cursor.y += system_scale * FONT_HEIGHT;
	console_puts(cursor.x, cursor.y, s, system_font_color, system_bg_color, system_scale);
}

// 한 칸(폭 max_chars 글자)에 문자열을 그린다.
// bash `help`처럼 칸을 넘치면 줄을 바꾸지 않고 끝을 '>'로 잘라낸다.
static void shell_put_col(uint32_t x, uint32_t y, const char* s, uint32_t max_chars) {
	char buf[128];
	if (max_chars > sizeof(buf) - 1) max_chars = sizeof(buf) - 1;

	if (strlen(s) > max_chars) {
		uint32_t keep = max_chars - 1;  // 마지막 칸은 '>' 표시용
		for (uint32_t i = 0; i < keep; i++) buf[i] = s[i];
		buf[keep] = '>';
		buf[keep + 1] = '\0';
		s = buf;
	}
	console_puts(x, y, s, system_font_color, system_bg_color, system_scale);
}

// bash `help`처럼 한 줄에 좌/우 두 칸을 그린다.
// 둘째 칸은 화면 폭 절반(글자 격자에 맞춤) 위치에서 시작하고,
// 각 칸은 폭을 넘으면 '>'로 잘려 다음 줄로 넘어가지 않는다.
static void shell_println_2col(const char* left, const char* right) {
	uint32_t glyph_w   = system_scale * FONT_WIDTH;
	uint32_t total_col = fb_get_width() / glyph_w;
	uint32_t col2_cell = total_col / 2;
	uint32_t col2_x    = col2_cell * glyph_w;

	uint32_t left_max  = col2_cell - 1;             // 둘째 칸과 한 글자 띄움
	uint32_t right_max = total_col - col2_cell - 1;  // 화면 끝과 한 글자 띄움

	cursor.x = system_cursor.x;
	cursor.y += system_scale * FONT_HEIGHT;

	uint32_t row_y = cursor.y;
	shell_put_col(system_cursor.x, row_y, left, left_max);
	if (right && *right)
		shell_put_col(col2_x, row_y, right, right_max);
}

static const char* skip_spaces(const char* p) {
	while (*p == ' ' || *p == '\t') p++;
	return p;
}

// 첫 단어가 cmd와 정확히 일치하면 인자 시작 포인터를 반환, 아니면 NULL.
static const char* match_cmd(const char* line, const char* cmd) {
	size_t n = strlen(cmd);
	if (strncmp(line, cmd, n) != 0) return (void*)0;
	if (line[n] != '\0' && line[n] != ' ' && line[n] != '\t') return (void*)0;
	return skip_spaces(line + n);
}

static void cmd_help(void) {
	shell_println("These shell commands are defined internally.  Type `help' to see this list.");
	shell_println("");

	shell_println_2col(" clear            - clear the screen",          " poweroff         - shut down (QEMU/Bochs only)");
	shell_println_2col(" doom             - run DOOM",                   " reboot           - restart the machine");
	shell_println_2col(" echo [arg ...]   - print arguments",           " uname [-a]       - print system name");
	shell_println_2col(" free [-m|-g]     - show memory usage",         " uptime           - show time since boot");
	shell_println_2col(" help             - show this list",            " whoami           - print current user");
}

static void cmd_clear(void) {
	fb_clear(system_bg_color);
	cursor.x = system_cursor.x;
	cursor.y = system_cursor.y;
}

static void cmd_echo(const char* args) {
	shell_println(args);
}

static void cmd_uname(const char* args) {
	if (strncmp(args, "-a", 2) == 0)
		shell_println("mini-dOS 0.1 x86_64 dOS");
	else
		shell_println("mini-dOS");
}

static void cmd_whoami(void) {
	shell_println("dOS");
}

// PIT는 1000Hz(irq.c)라 1 tick = 1ms.
static void cmd_uptime(void) {
	uint64_t ms = pit_get_ticks();
	uint64_t s  = ms / 1000;
	char buf[64];
	snprintf(buf, sizeof(buf), "up %llu min %llu sec (%llu ms)",
	         s / 60, s % 60, ms);
	shell_println(buf);
}

// 우분투 `free`처럼 메모리 사용량을 표로 출력한다.
// 버디 통계는 페이지(4KiB) 단위 → 기본 KiB, '-m'이면 MiB.
static void cmd_free(const char* args) {
	uint64_t total = 0, freep = 0;
	pmm_get_stats(&total, &freep);
	uint64_t used = total - freep;

	uint64_t unit = 1;          // 기본 KiB
	if (strncmp(args, "-m", 2) == 0) unit = 1024;
	else if (strncmp(args, "-g", 2) == 0) unit = 1024 * 1024;

	char buf[80];
	snprintf(buf, sizeof(buf), "       %12s %12s %12s", "total", "used", "free");
	shell_println(buf);
	snprintf(buf, sizeof(buf), "%-5s %12llu %12llu %12llu",
	         "Mem:",
	         total * 4 / unit, used * 4 / unit, freep * 4 / unit);
	shell_println(buf);
}

// 8042 키보드 컨트롤러 reset — QEMU와 대부분의 실기에서 동작.
// Makefile의 -no-reboot 때문에 QEMU는 재부팅 대신 종료한다.
static void cmd_reboot(void) {
	cli();
	while (inb(0x64) & 0x02) { /* 입력 버퍼 비우기 */ }
	outb(0x64, 0xFE);
	for (;;) hlt();
}

// QEMU/Bochs 전용 IO 포트 shortcut.
// run-debug에는 -no-shutdown이 있어 QEMU 창이 멈춘 채로 남는다.
static void cmd_poweroff(void) {
	cli();
	outw(0x604,  0x2000);  // QEMU 2.0+ (piix/q35) ACPI shutdown
	outw(0xB004, 0x2000);  // Bochs / 옛 QEMU 폴백
	for (;;) hlt();
}

void shell_wait() {
	// 안내 문구 출력 위치 설정
	cursor.x = system_cursor.x;
	cursor.y += system_scale * FONT_HEIGHT;

	// 안내 문구 출력
	shell_print_account("dOS");
	shell_print_dir("~");

	// 사용자 입력
	console_readline();
	const char* line = console_get_line();
	const char* args;

	if (line[0] == '\0') return;
	else if ((args = match_cmd(line, "help"))     != (void*)0) cmd_help();
	else if ((args = match_cmd(line, "clear"))    != (void*)0) cmd_clear();
	else if ((args = match_cmd(line, "echo"))     != (void*)0) cmd_echo(args);
	else if ((args = match_cmd(line, "uname"))    != (void*)0) cmd_uname(args);
	else if ((args = match_cmd(line, "whoami"))   != (void*)0) cmd_whoami();
	else if ((args = match_cmd(line, "uptime"))   != (void*)0) cmd_uptime();
	else if ((args = match_cmd(line, "free"))     != (void*)0) cmd_free(args);
	else if ((args = match_cmd(line, "reboot"))   != (void*)0) cmd_reboot();
	else if ((args = match_cmd(line, "poweroff")) != (void*)0) cmd_poweroff();
	else if (strcmp(line, "doom") == 0) { doom_run(); cmd_clear(); }	// DOOM 종료 후 화면(배경색·커서) 복원
	else shell_println("command not found");
}

void shell_init() {
	system_font_color = 0x00D3D7CF;  // 우분투 기본 전경색(연회색): 명령 출력에 사용
	system_bg_color = 0x001E1E1E;
	system_scale = 2;
	system_cursor.x = 0;
	system_cursor.y = 0;

	cursor.x = system_cursor.x;
	cursor.y = system_cursor.y;

	fb_clear(system_bg_color);
	console_setter(0x00FFFFFF, 0x001E1E1E, 2);
}
