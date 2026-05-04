#ifndef VMM_H
#define VMM_H

#include <stdint.h>

/*
 * VMM 정책 (단일 코어 가정 — TLB shootdown 없음)
 *
 * 매핑 종류는 ownership에 따라 두 갈래로 분리한다:
 *   1) phys 매핑  : 호출자가 PA를 소유 (MMIO, 커널 이미지, 부트 영역 등)
 *                  → unmap 시 pmm_free 하지 않음
 *   2) alloc 매핑 : VMM이 PMM에서 프레임을 할당 (커널 힙, 익명 페이지 등)
 *                  → free 시 pmm_free 까지 수행
 *
 * 페이지 크기:
 *   - flags에 PAGE_PS가 있고 va/pa/size가 2MB 정렬이면 map_page_2mb 사용
 *   - 그 외에는 4KB(map_page) 사용
 *   - 두 조건이 어긋나면 호출 실패(-1) 반환
 */

void vmm_init(void);

// PA 명시 매핑 (호출자 소유). 성공 0, 실패 -1.
int  vmm_map_phys(uint64_t va, uint64_t pa, uint64_t size, uint64_t flags);

// VMM 소유 매핑 — PMM에서 프레임 할당해 va에 매핑. 성공 0, 실패 -1.
int  vmm_alloc(uint64_t va, uint64_t size, uint64_t flags);

// vmm_map_phys로 만든 매핑 해제 (pmm_free 하지 않음).
void vmm_unmap(uint64_t va, uint64_t size);

// vmm_alloc으로 만든 매핑 해제 + 물리 프레임 반환.
void vmm_free(uint64_t va, uint64_t size);

#endif // VMM_H
