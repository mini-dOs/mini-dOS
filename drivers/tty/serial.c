#include <asm/cpu.h>
#include <drivers/serial.h>
#include <stddef.h>

#define COM1 0x3F8

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

void serial_write_dec(uint64_t value)
{
    char buf[21];   // uint64_t max = 18,446,744,073,709,551,615 -> 20자리 + 여유분
    size_t i = 0;

    if (value == 0)
    {
        serial_write_char('0');
        return;
    }
    while (value > 0)
    {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0)
    {
        serial_write_char(buf[--i]);
    }
}

void serial_write_size(uint64_t size)
{
    const uint64_t KB = 1024;
    const uint64_t MB = 1024 * KB;
    const uint64_t GB = 1024 * MB;

    uint64_t unit = 1;
    const char *suffix = " B";

    if (size >= GB)
    {
        unit = GB;
        suffix = " GB";
    }
    else if (size >= MB)
    {
        unit = MB;
        suffix = " MB";
    }
    else if (size >= KB)
    {
        unit = KB;
        suffix = " KB";
    }

    uint64_t whole = size / unit;
    uint64_t remainder = size % unit; // overflow 방지를 위해 소수점을 위한 나머지 계산
    uint64_t frac = (remainder * 100) / unit;

    serial_write_dec(whole);

    if (unit != 1) // B는 소수점 없음
    {
        serial_write_char('.');

        if (frac < 10)
        {
            serial_write_char('0');
        }

        serial_write_dec(frac);
    }

    serial_write(suffix);
}
