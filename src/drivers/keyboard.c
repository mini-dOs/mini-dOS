#include <interrupt.h>
#include <serial.h>
#include <i8042.h>

#define ESC	27
#define CTRL	0
#define ALT	0
#define L_SHIFT	0
#define R_SHIFT	0
#define CAPS	0
#define	F1	0
#define F2	0
#define F3	0
#define F4	0
#define F5	0
#define F6	0
#define F7	0
#define F8	0
#define F9	0
#define F10	0
#define Num_Lock	0
#define Scroll_Lock	0
#define Home	0
#define Up	0
#define Page_Up	0
#define Left	0
#define Num_Lock_5	0
#define Right	0
#define End	0
#define Down	0
#define Page_Down	0
#define Insert	0
#define Delete	0
#define F11	0
#define F12	0
#define Other	0

static const char keyboard_map[] = {
	0, ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	CTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	L_SHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', R_SHIFT,
	'*',	/* Numpad '*' */
	ALT,
	' ',	/* Space Bar */
	CAPS,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
	Num_Lock,
	Scroll_Lock,
	Home,
	Up,
	Page_Up,
	'-',	/* Numpad '-' */
	Left,
	Num_Lock_5,
	Right,
	'+',
	End,
	Down,
	Page_Down,
	Insert,
	Delete,
	0, 0, 0,
	F11,
	F12,
	Other,
};



void keyboard_handler(interrupt_frame_t *f) {
	// -Wall -Wextra 경고 옵션 때문에 컴파일 에러가 나는 것을 방지
	// 인자로 받았기 때문에 언급은 해야 함
	// interrupt.h에서 선언한 인터럽트 핸들러 규격 때문에 무조건 넣어야 함
	(void)f;	// 대신 아무것도 안 하도록 결과값이 없는 void형으로 형 변환

	uint8_t scancode = inb(0x60);

	if (scancode & 0x80) {
		return;
	}

	char c = keyboard_map[scancode];

	if (c > 0) {
		char buf[2] = {c, 0};
		serial_write(buf);
	}

	// pic_send_eoi() 함수가 자동으로 처리해줌
	// 그래서 outb(0x20, 0x20)를 따로 해줄 필요가 없음
	// outb(0x20, 0x20);
}
