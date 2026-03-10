#include <pic.h>
#include <stdint.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Port 0x80 is the POST diagnostic port; writing to it takes ~1-4 µs,
   which is the standard way to introduce an I/O delay on x86. */
static inline void io_wait(void) {
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

void pic_remap() {
    uint8_t a1 = inb(PIC1_DATA); // save masks
    uint8_t a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, 0x11); // start initialization
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    outb(PIC1_DATA, 0x20); // remap PIC1 to 0x20-0x27
    outb(PIC2_DATA, 0x28); // remap PIC2 to 0x28-0x2F
    io_wait();

    outb(PIC1_DATA, 0x04); // tell PIC1 about PIC2 at IRQ2
    outb(PIC2_DATA, 0x02); // tell PIC2 its cascade identity
    io_wait();

    outb(PIC1_DATA, 0x01); // set PIC1 to 8086 mode
    outb(PIC2_DATA, 0x01); // set PIC2 to 8086 mode
    io_wait();

    outb(PIC1_DATA, a1); // restore saved masks
    outb(PIC2_DATA, a2);
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20); // send EOI to slave PIC
    }

    outb(PIC1_COMMAND, 0x20); // send EOI to master PIC
}