#include <asm/interrupt.h>
#include <drivers/i8042.h>
#include <drivers/serial.h>

#define ESC		27
#define CTRL		0
#define ALT		0
#define L_SHIFT		0
#define R_SHIFT		0
#define CAPS		0
#define	F1		0
#define F2		0
#define F3		0
#define F4		0
#define F5		0
#define F6		0
#define F7		0
#define F8		0
#define F9		0
#define F10		0
#define Num_Lock	0
#define Scroll_Lock	0
#define Home		0
#define Up		0
#define Page_Up		0
#define Left		0
#define Num_Lock_5	0
#define Right		0
#define End		0
#define Down		0
#define Page_Down	0
#define Insert		0
#define Delete		0
#define F11		0
#define F12		0
#define Other		0

typedef struct ring_buffer {
	char buf[4096];
	uint16_t head;
	uint16_t tail;
} ring_buffer_t;

static ring_buffer_t rb;

static uint8_t shift_pressed	= 0;
static uint8_t caps_lock	= 0;

// === DOOM용 raw 키 이벤트 큐 ===
// shell이 쓰는 위 ASCII 큐(rb)와 별개.
// 누름/뗌 양쪽 + 확장키(0xE0)까지 모두 저장
typedef struct key_event {
	uint8_t scancode;	// 최상위 비트인 release 비트를 떼고 순수한 7-bit 크기의 스캔코드
	uint8_t pressed;	// 1 = 눌림(down), 0 = 뗌(up)
	uint8_t extended;	// 1 = 직전에 0xE0 prefix가 온 키 (방향키/RCtrl 등)
} key_event_t;

// 2의 거듭제곱(& 마스크용). 폴링 지연 대비 넉넉한 버퍼 깊이 — 자판 수와 무관
#define KEY_EVENT_QUEUE_SIZE 256
static key_event_t ev_buf[KEY_EVENT_QUEUE_SIZE];
static uint16_t ev_head = 0;
static uint16_t ev_tail = 0;

// 다음 인터럽트의 스캔코드가 확장(0xE0)임을 표시
static uint8_t extended_pending = 0;

static void key_event_enqueue(uint8_t scancode, uint8_t pressed, uint8_t extended) {
	uint16_t next = (ev_tail + 1) & (KEY_EVENT_QUEUE_SIZE - 1);
	if (next == ev_head) return;	// 가득 참 → 버림

	ev_buf[ev_tail].scancode = scancode;
	ev_buf[ev_tail].pressed  = pressed;
	ev_buf[ev_tail].extended = extended;
	ev_tail = next;
}

int keyboard_poll_event(uint8_t *scancode, uint8_t *pressed, uint8_t *extended) {
	if (ev_head == ev_tail) return 0;	// 빈 큐

	*scancode = ev_buf[ev_head].scancode;
	*pressed  = ev_buf[ev_head].pressed;
	*extended = ev_buf[ev_head].extended;
	ev_head = (ev_head + 1) & (KEY_EVENT_QUEUE_SIZE - 1);
	return 1;
}
// 키보드 회사에서 어떤 키를 누르면 어떤 신호를 보내겠다는 규약이 정해져 있음
// Keyboard Scan Codes: Set 1
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

static const char keyboard_map_shift[] = {
	0, ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	CTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	L_SHIFT, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', R_SHIFT,
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

static void keyboard_enqueue(char c) {
	// 가득 찬 버퍼 체크
	if (((rb.tail + 1) & 0xFFF) == rb.head) return;
	
	rb.buf[rb.tail] = c;
	
	rb.tail = (rb.tail + 1) & 0xFFF;	// if (rb.tail & 0x1000) rb.tail = 0;
}

char keyboard_dequeue() {
	// 빈 버퍼 체크
	if (rb.head == rb.tail) return '\0';

	char c = rb.buf[rb.head];

	rb.head = (rb.head + 1) & 0xFFF;	// if (rb.head & 0x1000) rb.head = 0;
	
	return c;
}

// ISR (Interrupt Service Routine)
void keyboard_handler(interrupt_frame_t *f) {
	// -Wall -Wextra 경고 옵션 때문에 컴파일 에러가 나는 것을 방지
	// 인자로 받았기 때문에 언급은 해야 함
	// interrupt.h에서 선언한 인터럽트 핸들러 규격 때문에 무조건 넣어야 함
	(void)f;	// 대신 아무것도 안 하도록 결과값이 없는 void형으로 형 변환

	uint8_t code = inb(0x60);	// 0x60 포트에서 스캔코드 추출

	// 지금 입력된 키가 확장 키임을 알려주는 코드(0xE0) → 다음 바이트가 진짜 키라는 사실만 표시하고 끝
	// 이렇게 하는 이유는 확장 키가 기존의 스캔 코드를 중복하여 사용하기 떄문
	// 왼쪽 방향키 = E0 4B
	// 오른쪽 숫자 키패드 4 = 4B
	if (code == 0xE0) {
		extended_pending = 1;
		return;
	}

	uint8_t pressed  = !(code & 0x80);	// (code & 0x80)이 1이면 떼진 것(up)
	uint8_t scancode = code & 0x7F;		// release 비트를 제거한 순수 7-bit 데이터 코드
	uint8_t extended = extended_pending;	// 위에서 E0가 인식됐으면 확장키라는 상태를 저장
	extended_pending = 0;

	// raw 이벤트 큐 — 모든 키, 누름/뗌 양쪽 다 적재
	key_event_enqueue(scancode, pressed, extended);

	// 일반 ASCII 경로 — 확장키(방향키 등)는 제외하고 shell의 기존 동작 유지를 위한 경로. 확장키는 DOOM에서 사용(방향키 등)
	if (extended)
		return;

	// 키가 떼졌을 때
	if (!pressed) {
		// 그 키가 L_SHIFT나 R_SHIFT일 때
		if (scancode == 0x2A || scancode == 0x36)
			shift_pressed = 0;
		return;
	}

	// 눌린 키가 L_SHIFT나 R_SHIFT일 때
	if (scancode == 0x2A || scancode == 0x36) {
		shift_pressed = 1;
		return;
	} // 눌린 키가 CAPS_LOCK일 때
	else if (scancode == 0x3A) {
		caps_lock ^= 1;
		return;
	}

	char c;
	if (shift_pressed ^ caps_lock)
		c = keyboard_map_shift[scancode];
	else
		c = keyboard_map[scancode];

	// ASCII 코드만 출력
	if (c > 0) {
		keyboard_enqueue(c);
		char buf[2] = {c, 0};
		serial_write(buf);
	}
}
