#include <stdint.h>

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void serial_init(void) {
    outb(COM1 + 1, 0x00); // disable interrupts
    outb(COM1 + 3, 0x80); // DLAB on
    outb(COM1 + 0, 0x03); // divisor low  (38400 baud if base clock 115200)
    outb(COM1 + 1, 0x00); // divisor high
    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop
    outb(COM1 + 2, 0xC7); // enable FIFO, clear, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

static void serial_write_char(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0) { }
    outb(COM1, (uint8_t)c);
}
static void serial_write(const char* s) {
    for (; *s; s++) serial_write_char(*s);
}

/* Called from boot.s with Multiboot2 magic in RDI, info physical addr in RSI */
void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info) {
    (void)multiboot_info; /* use later for memory map, cmdline, etc. */
    serial_init();
    serial_write("Hello from x86_64 kernel!\r\n");
    if (multiboot_magic == 0x36d76289) {
        serial_write("Multiboot2 magic OK.\r\n");
    }
    for (;;) __asm__ __volatile__("hlt");
}
