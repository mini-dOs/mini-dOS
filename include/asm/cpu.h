#ifndef CPU_H
#define CPU_H

#include <stdint.h>

/* Enable CPU interrupts by setting the interrupt flag (IF). */
static inline void sti(void) {
    __asm__ volatile("sti");
}

/* Disable CPU interrupts by clearing the interrupt flag (IF). */
static inline void cli(void) {
    __asm__ volatile("cli");
}

/* Halt the CPU until the next external interrupt occurs. */
static inline void hlt(void) {
    __asm__ volatile("hlt");
}

/* Write a byte to the specified I/O port. */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* Read a byte from the specified I/O port. */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Introduce a short delay by writing to port 0x80 (POST diagnostic port). */
static inline void io_wait(void) {
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

#endif