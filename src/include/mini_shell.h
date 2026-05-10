#ifndef MINI_SHELL_H
#define MINI_SHELL_H

typedef struct system_cursor {
	uint32_t x;
	uint32_t y;
} system_cursor_t;

extern uint32_t system_font_color;
extern uint32_t system_bg_color;
extern uint32_t system_scale;
extern system_cursor_t system_cursor;

void shell_print_account(const char* account);
void shell_print_dir(const char* dir);
void shell_wait();
void shell_init();


#endif
