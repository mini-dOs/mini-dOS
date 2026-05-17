#include <drivers/serial.h>
#include <kernel/kernel_base.h>
#include <kernel/multiboot.h>
#include <mm/page.h>
#include <stdint.h>

page_t* page;
uint64_t array_size;
uint64_t array_end_addr;
uint32_t page_region;	// 페이지 배열이 들어있는 usable_region 인덱스

void page_init() {
	uint32_t total_page_num = usable_regions[usable_region_count - 1].end >> 12;	// 페이지 개수
	array_size = total_page_num << 6;	// 바이트 단위

	// 페이지 구조체 크기 확인용
	serial_write("Page Struct Size: ");
	serial_write_dec(sizeof(page_t));
	serial_write("\r\n");
	// 페이지 구조체 크기 확인용

	for (int i = 0; i < usable_region_count; i++) {
		uint64_t region_size = usable_regions[i].end - usable_regions[i].start;	// 페이지 개수

		if (region_size >= array_size) {
			page = (page_t*)phys_to_virt(usable_regions[i].start);
			// page 크기는 64B 보장 & total_page_num * 64가 4KB 배수가 아닐 경우를 대비한 올림 정렬
			array_end_addr = (usable_regions[i].start + array_size + 0xFFF) & ~0xFFF;
			page_region = i;
			break;
		}
	}
}

void page_array_init() {
	uint32_t page_num;
	uint64_t pivot_addr;

	// usable_regions[0] ~ page 배열이 없는 usable_region 정보 초기화
	for (uint32_t i = 0; i < page_region; i++) {
		page_num = (usable_regions[i].end - usable_regions[i].start) >> 12;
		pivot_addr = usable_regions[i].start;

		for (uint32_t j = 0; j < page_num; j++) {
			page[pivot_addr >> 12].status = FREE;
			page[pivot_addr >> 12].order = 0;

			pivot_addr += PAGE_SIZE;
		}
	}

	// page 배열이 있는 usable_region
	page_num = (usable_regions[page_region].end - usable_regions[page_region].start) >> 12;
	pivot_addr = usable_regions[page_region].start;

	for (uint32_t i = 0; i < page_num; i++) {
		// 현재 주소가 page 배열 내부라면
		if (pivot_addr < array_end_addr) {
			pivot_addr += PAGE_SIZE;	// 패스
		} // 현재 주소가 page 배열을 벗어났다면
		else {
			page[pivot_addr >> 12].status = FREE;
			page[pivot_addr >> 12].order = 0;

			pivot_addr += PAGE_SIZE;
		}
	}

	// page 배열 이후의 usable_region들 정보 초기화
	for (uint32_t i = page_region + 1; i < usable_region_count; i++) {
		page_num = (usable_regions[i].end - usable_regions[i].start) >> 12;
		pivot_addr = usable_regions[i].start;

		for (uint32_t j = 0; j < page_num; j++) {
			page[pivot_addr >> 12].status = FREE;
			page[pivot_addr >> 12].order = 0;

			pivot_addr += PAGE_SIZE;
		}
	}
}
