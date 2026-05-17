#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <kernel/kernel_info.h>
#include <kernel/multiboot2.h>
#include <stdint.h>

typedef struct {
	uint64_t start;
	uint64_t end;
} memory_region_t;

typedef struct fb_info {
	uint64_t framebuffer_addr;	// 좌상단 픽셀의 물리 주소
	uint32_t framebuffer_pitch;	// 한 행의 align된 바이트 수
	uint32_t framebuffer_width;	// 한 행의 픽셀 수
	uint32_t framebuffer_height;	// 한 열의 픽셀 수
	uint8_t framebuffer_bpp;	// bits per pixel
	uint8_t framebuffer_type;
} fb_info_t;

extern memory_region_t* usable_regions;
extern fb_info_t fb_info;
extern uint32_t usable_region_count;

void multiboot_parse(void* mb_info);

#endif
