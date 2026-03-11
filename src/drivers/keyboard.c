#include <interrupt.h>
#include <serial.h>
#include <i8042.h>

void keyboard_handler(interrupt_frame_t *f) {
	// -Wall -Wextra 경고 옵션 때문에 컴파일 에러가 나는 것을 방지
	// 인자로 받았기 때문에 언급은 해야 함
	// interrupt.h에서 선언한 인터럽트 핸들러 규격 때문에 무조건 넣어야 함
	(void)f;	// 대신 아무것도 안 하도록 결과값이 없는 void형으로 형 변환

	uint8_t scancode = i8042_read_data();

	serial_write("\r\n[Keyboard Interrupt! Scancode: ");
	serial_write_hex64(scancode);
	serial_write("]\r\n");

	// pic_send_eoi() 함수가 자동으로 처리해줌
	// 그래서 outb(0x20, 0x20)를 따로 해줄 필요가 없음
	// outb(0x20, 0x20);
}
