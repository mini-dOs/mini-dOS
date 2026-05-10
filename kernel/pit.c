#include <stdint.h>
#include <asm/cpu.h>

#define PIT_BASE_FREQ 1193182

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40

static uint16_t pit_compute_divisor(uint32_t freq)
{
    if (freq == 0)
        return 0;

    uint32_t divisor = PIT_BASE_FREQ / freq;

    if (divisor == 0)
        divisor = 1;

    if (divisor > 65535)
        divisor = 65535;

    return (uint16_t)divisor;
}

static void pit_set_divisor(uint16_t divisor)
{
    outb(PIT_COMMAND, 0x36);

    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);
}

void pit_set_frequency(uint32_t freq)
{
    uint16_t divisor = pit_compute_divisor(freq);

    if (divisor == 0)
        return;

    pit_set_divisor(divisor);
}

void pit_init(uint32_t freq)
{
    pit_set_frequency(freq);
}