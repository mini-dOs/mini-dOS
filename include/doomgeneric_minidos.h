#ifndef DOOMGENERIC_MINIDOS_H
#define DOOMGENERIC_MINIDOS_H

// doomgeneric 게임 루프 진입. DOOM을 종료(메뉴 Quit)하면 정리 후 셸로 복귀한다.
void doom_run(void);

// DOOM 종료 요청 플래그. DOOM의 I_Quit이 1로 올리면 doom_run의 게임 루프가 빠져나온다.
extern volatile int dg_quit_requested;

// DOOM 종료 후 1회 정리(zone/lumpinfo/atexit 해제). doom/i_system.c에 정의.
void I_DoomShutdown(void);

#endif
