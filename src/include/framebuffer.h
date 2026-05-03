#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <multiboot.h>
#include <stdint.h>

uint8_t* fb_get_base();
uint32_t fb_get_pitch();
uint32_t fb_get_width();
uint32_t fb_get_height();
uint8_t fb_get_bpp();
void fb_paint_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_clear(uint32_t color);
void fb_init();

#endif
