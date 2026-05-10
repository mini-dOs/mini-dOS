#include <asm/idt.h>
#include <asm/interrupt.h>
#include <stdint.h>
#include <drivers/serial.h>
#include <asm/pic.h>

static interrupt_handler_t interrupt_handlers[IDT_SIZE];

void interrupt_register(uint8_t vector, interrupt_handler_t handler) {
    interrupt_handlers[vector] = handler;
}

void interrupt_dispatch(interrupt_frame_t *frame) {
    uint8_t vector = (uint8_t)frame->vector;

    interrupt_handler_t handler = interrupt_handlers[vector];

    if (vector >= 32 && vector < 48) {
        pic_send_eoi(vector - 32);
    }

    if (handler) {
        handler(frame);
    } else {
        serial_write("Unhandled interrupt: ");
        serial_write_hex64(frame->vector);
        serial_write("\n");
    }
}
