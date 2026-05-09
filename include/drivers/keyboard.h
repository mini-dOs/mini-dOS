#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <asm/interrupt.h>

char keyboard_dequeue();
void keyboard_handler(interrupt_frame_t *f);

#endif
