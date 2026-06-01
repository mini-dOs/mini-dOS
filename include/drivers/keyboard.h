#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <asm/interrupt.h>
#include <stdint.h>

char keyboard_dequeue();
void keyboard_handler(interrupt_frame_t *f);

// DOOM용 raw 키 이벤트 폴링
// 이벤트가 있으면 1을 반환하며 out 파라미터를 채우고, 없으면 0
int keyboard_poll_event(uint8_t *scancode, uint8_t *pressed, uint8_t *extended);

#endif
