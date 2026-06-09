// doomgeneric <-> mini-dOS 플랫폼 glue
//
// doomgeneric.h가 "Implement below functions for your platform"로 남겨둔
// 6개 DG_* 훅을 mini-dOS 커널 서비스에 연결한다.
//   - 타이머 : PIT(1000Hz) tick  -> DG_GetTicksMs / DG_SleepMs
//   - 입력   : 키 이벤트 큐       -> DG_GetKey
//   - 화면   : 프레임버퍼         -> DG_Init / DG_DrawFrame
//
// DOOM 소스(doom/)는 -I libc/include -I doom 로 커널과 격리돼 있으므로
// 이 파일은 *커널 쪽*(glue/)에 두어 커널 헤더와 doomgeneric.h를 동시에 본다.

#include <stdint.h>
#include <stdlib.h>

#include <drivers/framebuffer.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <kernel/pit.h>

#include "doomgeneric.h"
#include "doomkeys.h"

#include <doomgeneric_minidos.h>

// ---------------------------------------------------------------------------
// A. 타이머  (PIT @ 1000Hz 라서 1 tick == 1ms)
// ---------------------------------------------------------------------------
uint32_t DG_GetTicksMs(void) {
	return (uint32_t)pit_get_ticks();
}

void DG_SleepMs(uint32_t ms) {
	uint64_t start = pit_get_ticks();
	// 인터럽트가 켜진 상태(쉘 컨텍스트)에서 tick이 진행되므로 hlt로 대기
	while ((pit_get_ticks() - start) < (uint64_t)ms)
		__asm__ __volatile__("hlt");
}

// ---------------------------------------------------------------------------
// C. 화면 출력
//   DG_ScreenBuffer: 640x400, 32bpp 0xAARRGGBB (i_video.c rgba8888 모드)
//   fb_paint_pixel의 color 인자와 바이트 레이아웃이 동일해 그대로 복사 가능.
// ---------------------------------------------------------------------------
void DG_Init(void) {
	fb_clear(0x00000000);
}

// ===== AI-GENERATED (Claude) BEGIN =====
// DOOM 640x400 프레임을 화면에 들어가는 최대 정수배로 업스케일해 중앙에 그린다.
// (기존엔 1:1 복사라 큰 GOP 해상도에서 화면 가운데에만 작게 떠 보였음.)
// 최근접(nearest) 확대 — 각 src 픽셀을 s×s 블록으로 복제. 예) 1280x800 → s=2.
void DG_DrawFrame(void) {
	const uint32_t fbw = fb_get_width();
	const uint32_t fbh = fb_get_height();

	// 화면에 들어가는 최대 정수 배율 (화면이 DOOM보다 작으면 1 = 클리핑)
	uint32_t sx = fbw / DOOMGENERIC_RESX;
	uint32_t sy = fbh / DOOMGENERIC_RESY;
	uint32_t s  = sx < sy ? sx : sy;
	if (s == 0) s = 1;

	const uint32_t dw = DOOMGENERIC_RESX * s;	// 그려질 폭/높이
	const uint32_t dh = DOOMGENERIC_RESY * s;
	const uint32_t ox = fbw > dw ? (fbw - dw) / 2 : 0;	// 중앙 정렬
	const uint32_t oy = fbh > dh ? (fbh - dh) / 2 : 0;
	// 화면 밖으로 안 나가도록 그릴 src 픽셀 수 클리핑
	const uint32_t cw = (fbw - ox) / s < DOOMGENERIC_RESX ? (fbw - ox) / s : DOOMGENERIC_RESX;
	const uint32_t ch = (fbh - oy) / s < DOOMGENERIC_RESY ? (fbh - oy) / s : DOOMGENERIC_RESY;

	if (fb_get_bpp() == 32) {
		// 빠른 경로: 32비트 직접 복사 (행/픽셀을 s배 복제)
		uint8_t *base  = fb_get_base();
		uint32_t pitch = fb_get_pitch();
		for (uint32_t y = 0; y < ch; y++) {
			const pixel_t *src = DG_ScreenBuffer + (uint64_t)y * DOOMGENERIC_RESX;
			for (uint32_t ry = 0; ry < s; ry++) {
				uint32_t *dst = (uint32_t *)(base + (uint64_t)(oy + y * s + ry) * pitch + (uint64_t)ox * 4);
				for (uint32_t x = 0; x < cw; x++) {
					uint32_t c = (uint32_t)src[x];
					uint32_t *d = dst + (uint64_t)x * s;
					for (uint32_t rx = 0; rx < s; rx++)
						d[rx] = c;
				}
			}
		}
	} else {
		// 일반 경로: bpp 변환은 fb_paint_pixel에 위임
		for (uint32_t y = 0; y < ch; y++) {
			const pixel_t *src = DG_ScreenBuffer + (uint64_t)y * DOOMGENERIC_RESX;
			for (uint32_t x = 0; x < cw; x++) {
				uint32_t c = (uint32_t)src[x];
				for (uint32_t ry = 0; ry < s; ry++)
					for (uint32_t rx = 0; rx < s; rx++)
						fb_paint_pixel(ox + x * s + rx, oy + y * s + ry, c);
			}
		}
	}
}
// ===== AI-GENERATED (Claude) END ======

void DG_SetWindowTitle(const char *title) {
	if (!title) return;
	serial_write("[doom] ");
	serial_write(title);
	serial_write("\r\n");
}

// ---------------------------------------------------------------------------
// B. 입력
//   keyboard_poll_event()가 PS/2 scancode set 1의 make 코드(release 비트 제거),
//   pressed, extended(0xE0 prefix) 를 돌려준다. 이를 DOOM 키코드로 변환.
// ---------------------------------------------------------------------------

// 비확장(non-0xE0) make 코드 -> DOOM 키코드. 0 = 매핑 없음(무시).
static const unsigned char s_base_map[128] = {
	[0x01] = KEY_ESCAPE,
	[0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
	[0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
	[0x0C] = KEY_MINUS, [0x0D] = KEY_EQUALS,
	[0x0E] = KEY_BACKSPACE,
	[0x0F] = KEY_TAB,
	[0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
	[0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
	[0x1A] = '[', [0x1B] = ']',
	[0x1C] = KEY_ENTER,
	[0x1D] = KEY_FIRE,			// Left Ctrl   -> 발사
	[0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
	[0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l',
	[0x27] = ';', [0x28] = '\'', [0x29] = '`',
	[0x2A] = KEY_RSHIFT,			// Left Shift  -> 달리기
	[0x2B] = '\\',
	[0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b',
	[0x31] = 'n', [0x32] = 'm',
	[0x33] = ',', [0x34] = '.', [0x35] = '/',
	[0x36] = KEY_RSHIFT,			// Right Shift -> 달리기
	[0x37] = KEYP_MULTIPLY,
	[0x38] = KEY_LALT,			// Left Alt    -> 옆걸음(strafe)
	[0x39] = KEY_USE,			// Space       -> 사용/문열기
	[0x3A] = KEY_CAPSLOCK,
	[0x3B] = KEY_F1,  [0x3C] = KEY_F2,  [0x3D] = KEY_F3,  [0x3E] = KEY_F4,
	[0x3F] = KEY_F5,  [0x40] = KEY_F6,  [0x41] = KEY_F7,  [0x42] = KEY_F8,
	[0x43] = KEY_F9,  [0x44] = KEY_F10,
	// 키패드 (NumLock off 시 방향키로도 쓰이지만 기본 매핑만 제공)
	[0x47] = KEY_HOME,     [0x48] = KEY_UPARROW,   [0x49] = KEY_PGUP,
	[0x4B] = KEY_LEFTARROW,[0x4D] = KEY_RIGHTARROW,
	[0x4F] = KEY_END,      [0x50] = KEY_DOWNARROW, [0x51] = KEY_PGDN,
	[0x52] = KEY_INS,      [0x53] = KEY_DEL,
};

static unsigned char translate(uint8_t sc, uint8_t ext) {
	if (ext) {
		// 0xE0 확장: 메인 방향키/내비게이션 및 우측 수정자 키
		switch (sc) {
			case 0x48: return KEY_UPARROW;
			case 0x50: return KEY_DOWNARROW;
			case 0x4B: return KEY_LEFTARROW;
			case 0x4D: return KEY_RIGHTARROW;
			case 0x1D: return KEY_FIRE;		// Right Ctrl
			case 0x38: return KEY_RALT;		// Right Alt
			case 0x1C: return KEY_ENTER;		// 키패드 Enter
			case 0x47: return KEY_HOME;
			case 0x4F: return KEY_END;
			case 0x49: return KEY_PGUP;
			case 0x51: return KEY_PGDN;
			case 0x52: return KEY_INS;
			case 0x53: return KEY_DEL;
			default:   return 0;
		}
	}
	if (sc < 128) return s_base_map[sc];
	return 0;
}

int DG_GetKey(int *pressed, unsigned char *key) {
	uint8_t sc, pr, ext;
	while (keyboard_poll_event(&sc, &pr, &ext)) {
		unsigned char k = translate(sc, ext);
		if (!k) continue;		// 매핑 안 된 키는 건너뛰고 다음 이벤트
		*pressed = pr;
		*key     = k;
		return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------
// 진입점
//   doomgeneric_Create()는 DOOM을 초기화하고 첫 프레임 1틱만 돌린 뒤 *복귀*한다
//   (doomgeneric 구조: D_DoomLoop이 doomgeneric_Tick을 1회만 호출). 따라서 호스트가
//   doomgeneric_Tick()을 매 프레임 반복 호출해야 게임이 실제로 진행된다.
//   이 루프가 없으면 첫 프레임만 그리고 셸로 복귀해 버린다.
//   IWAD(doom1.wad)는 부팅 시 GRUB 모듈로 적재되어 libc 모듈 레지스트리에 등록돼
//   있어야 한다 (kmain의 libc_register_module). -iwad로 그 이름을 지정한다.
// ---------------------------------------------------------------------------
// ===== AI-GENERATED (Claude) BEGIN =====
// 호스트 종료 요청 플래그. DOOM 메뉴 Quit → I_Quit이 1로 올리면 아래 루프가 빠져나온다.
volatile int dg_quit_requested = 0;

void doom_run(void) {
	char *argv[] = { "doom", "-iwad", "doom1.wad" };

	dg_quit_requested = 0;			// 재실행 대비 초기화
	doomgeneric_Create(3, argv);

	// 게임 루프: I_Quit이 종료 플래그를 올릴 때까지 매 프레임 틱.
	// (Quit한 그 틱의 나머지는 아직 살아있는 버퍼 위에서 무해하게 끝나고 복귀한다.)
	while (!dg_quit_requested)
		doomgeneric_Tick();

	// ── 종료 정리: 셸로 깨끗이 복귀 + 다음 "doom" 재실행이 가능하도록 ──
	I_DoomShutdown();			// zone/lumpinfo/atexit 해제 (doom 측 상태)
	if (DG_ScreenBuffer) {			// glue가 매 실행 malloc하는 프레임 버퍼
		free(DG_ScreenBuffer);
		DG_ScreenBuffer = NULL;
	}
	while (keyboard_dequeue() != '\0')	// 셸로 새어나갈 잔여 키 입력 비우기
		;
	serial_write("[doom] quit -> shell\r\n");	// 종료/복귀 진단 마커
	// 화면 복원(배경색·커서)은 셸 소관이므로 호출부(mini_shell)가 doom_run 복귀 후 처리한다.
}
// ===== AI-GENERATED (Claude) END ======
