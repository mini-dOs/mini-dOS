#include <serial.h>

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void serial_init(void)
{
    outb(COM1 + 1, 0x00); /* disable interrupts */
    outb(COM1 + 3, 0x80); /* DLAB on */
    outb(COM1 + 0, 0x03); /* divisor low (38400 baud @ 115200 clock) */
    outb(COM1 + 1, 0x00); /* divisor high */
    outb(COM1 + 3, 0x03); /* 8 bits, no parity, one stop */
    outb(COM1 + 2, 0xC7); /* enable FIFO, clear, 14-byte threshold */
    outb(COM1 + 4, 0x0B); /* IRQs enabled, RTS/DSR set */
}

void serial_write_char(char c)
{
    while ((inb(COM1 + 5) & 0x20) == 0)
    {
    }
    outb(COM1, (uint8_t)c);
}

void serial_write(const char *s)
{
    for (; *s; s++)
    {
        serial_write_char(*s);
    }
}

void serial_write_hex64(uint64_t value)
{
    serial_write("0x");
    for (int i = 15; i >= 0; i--)
    {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        if (nibble < 10)
        {
            serial_write_char((char)('0' + nibble));
        }
        else
        {
            serial_write_char((char)('A' + nibble - 10));
        }
    }
}
