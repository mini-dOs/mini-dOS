#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <multiboot.h>
#include <stdint.h>

void framebuffer_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void framebuffer_clear(uint32_t color);
void framebuffer_init();

#endif
