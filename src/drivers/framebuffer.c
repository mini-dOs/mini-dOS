#include <font.h>
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

// getter
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

// 문자 출력
void fb_write_char(uint32_t x, uint32_t y, char c, uint32_t color, uint32_t scale) {
	// 헷갈려서 메모해 놓음
	// i == y (가로 줄; 행)
	// j == x (세로 줄; 열)
	uint8_t ch_row;

	// 한 문자를 출력하기 위한 for문

	// 각 행(가로줄)
	for (int i = 0; i < FONT_HEIGHT; i++) {
		ch_row = font[(uint8_t)c][i];
		// 의 각 픽셀들을
		for (int j = 0; j < FONT_WIDTH; j++) {
			// 비트가 1이면 색칠하는데
			if (ch_row & (1 << (FONT_WIDTH - j - 1))) {
				// scale만큼 반복하여 가로 세로 색칠
				for (uint32_t s_i = 0; s_i < scale; s_i++) {
					for (uint32_t s_j = 0; s_j < scale; s_j++) {
						fb_paint_pixel(x + j * scale + s_j, y + i * scale + s_i, color);
					}
				}
			}
		}
	}
}

void fb_write(uint32_t x, uint32_t y, const char* str, uint32_t color, uint32_t scale) {
	uint32_t str_x = x;
	uint32_t str_y = y;

	while (*str) {
		// 만약 글자가 화면 오른쪽을 넘어가면
		if (str_x + FONT_WIDTH * scale > fb_width) {
			str_x = 0;
			str_y += FONT_HEIGHT * scale;
		}

		fb_write_char(str_x, str_y, *str, color, scale);

		str_x += FONT_WIDTH * scale;
		str++;	// 다음 글자
	}
}

// 픽셀 색칠
void fb_paint_pixel(uint32_t x, uint32_t y, uint32_t color) {
	uint8_t* ptr = fb_base + (x * (fb_bpp / 8)) + (y * fb_pitch);

	// code-review에 따른 수정
	switch (fb_bpp) {
		case 32:
			*(uint32_t*)ptr = color;
			break;
		case 24:
			ptr[0] = color & (0xFF);		// BLUE
			ptr[1] = (color >> 8) & (0xFF);		// GREEN
			ptr[2] = (color >> 16) & (0xFF);	// RED
			break;
		case 16:
			*(uint16_t*)ptr = (uint16_t)color;
			break;
		default:
			serial_write("fb_bpp: ");
			serial_write_dec(fb_bpp);
			serial_write(" (ERROR FROM fb_paint_pixel)\r\n");
	}

	// 추가 설명 주석

	/*
	 * |           32bit           |
	 * | 8bit | 8bit | 8bit | 8bit |
	 * 만약 color(uint32_t)를 ptr에 저장한다면
	 * 시스템이 리틀 엔디안이기 때문에 가장 앞의 8bit
	 * 즉, ptr[0]에 BLUE가 저장됨
	 * 같은 원리로 ptr[1]에는 GREEN, ptr[2]에는 RED가 저장됨
	 *
	 * 헷갈릴 수 있는 개념(내가 헷갈린)
	 * case 24의 경우 color와의 비트 연산이 헷갈릴 수 있음
	 * 리틀 엔디안이 고려되는 것은 ptr[0],[1],[2]의 위치이고
	 * color를 shift할 때 shift하는 양(>> '이거')은 그대로 읽으면 됨
	 */
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
		case 24:
			for (uint32_t y = 0; y < fb_height; y++) {
				for (uint32_t x = 0; x <fb_width; x++) {
					fb_paint_pixel(x, y, color);
				}
			}
			break;
		case 16:
			for (uint32_t y = 0; y < fb_height; y++) {
				uint16_t* row = fb_base + (y * fb_pitch);
				for (uint32_t x = 0; x < fb_width; x++)
					row[x] = (uint16_t)color;
			}
			break;
		default:
			serial_write("fb_bpp: ");
			serial_write_dec(fb_bpp);
			serial_write(" (ERROR FROM fb_clear)\r\n");
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
