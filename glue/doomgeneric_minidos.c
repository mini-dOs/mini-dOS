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

void DG_DrawFrame(void) {
	const uint32_t fbw = fb_get_width();
	const uint32_t fbh = fb_get_height();

	// HW 프레임버퍼가 DOOM 해상도보다 크면 가운데 정렬, 작으면 클리핑
	const uint32_t cw = fbw < DOOMGENERIC_RESX ? fbw : DOOMGENERIC_RESX;
	const uint32_t ch = fbh < DOOMGENERIC_RESY ? fbh : DOOMGENERIC_RESY;
	const uint32_t ox = (fbw - cw) / 2;
	const uint32_t oy = (fbh - ch) / 2;

	if (fb_get_bpp() == 32) {
		// 빠른 경로: 행 단위 32비트 직접 복사
		uint8_t *base    = fb_get_base();
		uint32_t pitch   = fb_get_pitch();
		for (uint32_t y = 0; y < ch; y++) {
			uint32_t *dst       = (uint32_t *)(base + (uint64_t)(oy + y) * pitch + (uint64_t)ox * 4);
			const pixel_t *src  = DG_ScreenBuffer + (uint64_t)y * DOOMGENERIC_RESX;
			for (uint32_t x = 0; x < cw; x++)
				dst[x] = src[x];
		}
	} else {
		// 일반 경로: bpp 변환은 fb_paint_pixel에 위임
		for (uint32_t y = 0; y < ch; y++) {
			const pixel_t *src = DG_ScreenBuffer + (uint64_t)y * DOOMGENERIC_RESX;
			for (uint32_t x = 0; x < cw; x++)
				fb_paint_pixel(ox + x, oy + y, (uint32_t)src[x]);
		}
	}
}

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
//   doomgeneric_Create() -> D_DoomMain() 내부 게임 루프로 진입하며 보통 복귀하지
//   않는다. (실제 구동에는 IWAD가 libc 모듈로 등록돼 있어야 한다 — 후속 작업.)
// ---------------------------------------------------------------------------
void doom_run(void) {
	char *argv[] = { "doom" };
	doomgeneric_Create(1, argv);
}
