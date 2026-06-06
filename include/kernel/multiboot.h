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

// GRUB이 적재한 부트 모듈(예: doom1.wad). start/end는 물리 주소(정확값)이고
// 모듈 영역은 usable_regions에서 carve-out되어 PMM이 건드리지 않는다.
// 접근은 vmm_init이 깐 direct-map(phys_to_virt)으로 한다.
#define MAX_BOOT_MODULES 8

typedef struct boot_module {
	uint64_t start;		// 모듈 데이터 물리 시작 주소
	uint64_t end;		// 모듈 데이터 물리 끝 주소 (exclusive)
	char     name[64];	// cmdline 문자열의 basename (NUL 종료)
} boot_module_t;

extern memory_region_t* usable_regions;
extern fb_info_t fb_info;
extern uint32_t usable_region_count;
extern boot_module_t boot_modules[MAX_BOOT_MODULES];
extern uint32_t boot_module_count;

void multiboot_parse(void* mb_info);

#endif
