#include <framebuffer.h>
#include <kernel_base.h>
#include <mm/paging.h>
#include <serial.h>
#include <stdint.h>

static uint8_t*	fb_base;
static uint32_t	fb_pitch;
static uint32_t	fb_width;
static uint32_t	fb_height;
static uint8_t	fb_bpp;

uint8_t* fb_get_base() {
	return fb_base;
}

uint32_t fb_get_pitch() {
	return fb_pitch;
}

uint32_t fb_get_width() {
	return fb_width;
}

uint32_t fb_get_height() {
	return fb_height;
}

uint8_t fb_get_bpp() {
	return fb_bpp;
}

void fb_paint_pixel(uint32_t x, uint32_t y, uint32_t color) {
	uint8_t* ptr = fb_base + (x * (fb_bpp / 8)) + (y * fb_pitch);

	// code-review에 따른 수정
	switch (fb_bpp) {
		case 32:
			*(uint32_t*)ptr = color;
			break;
		case 16:
			*(uint16_t*)ptr = (uint16_t)color;
			break;
	}
}

void fb_clear(uint32_t color) {
	// code-review에 따른 수정
	switch (fb_bpp) {
		case 32:
			for (uint32_t y = 0; y < fb_height; y++) {
				uint32_t* row = fb_base + (y * fb_pitch);
				for (uint32_t x = 0; x < fb_width; x++)
					row[x] = color;
			}
			break;
		case 16:
			for (uint32_t y = 0; y < fb_height; y++) {
				uint16_t* row = fb_base + (y * fb_pitch);
				for (uint32_t x = 0; x < fb_width; x++)
					row[x] = (uint16_t)color;
			}
			break;
	}
}

void fb_init() {
	fb_base		= (uint8_t*)phys_to_virt((uintptr_t)fb_info.framebuffer_addr);
	fb_pitch	= fb_info.framebuffer_pitch;
	fb_width	= fb_info.framebuffer_width;
	fb_height	= fb_info.framebuffer_height;
	fb_bpp		= fb_info.framebuffer_bpp;

	// 0x0000'0000'C000'0000이 fb_base에 저장된 프레임 버퍼의 주소인데
	// 아직 usable_regions에 없음
	// 그래서 kmain에서 계속 Page Fault -> Page Alloc
	// 그렇기 때문에 kmain에서 framebuffer_init을 호출하여 사용하기 전에
	// 여기서 프레임버퍼 물리 주소 범위를 map_page_2mb로 명시적으로 매핑해야 함
	// 크기는 fb_width * fb_height만큼이고 2MB 단위로 올림해야 함
	
	uint64_t display_size = fb_pitch * fb_height;
	// fb_width * (fb_bpp / 8) * fb_height로 구해보려했지만
	// 그럴 경우, 하드웨어에서 성능을 위해 추가한 패딩을 놓칠 수 있음
	// 그래서 패딩까지 포함한 fb_pitch로 계산
	uint64_t size_2mb_align = (display_size + 0x1FFFFF) & ~0x1FFFFF;

	for (uint64_t offset = 0; offset < size_2mb_align; offset += 0x200000) {
		map_page_2mb(pml4_root, (uint64_t)(fb_base + offset), fb_info.framebuffer_addr + offset, PAGE_RW | PAGE_PS);
	}
}
