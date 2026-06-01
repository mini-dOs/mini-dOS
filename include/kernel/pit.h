#ifndef PIT_H
#define PIT_H

#include <stdint.h>

void pit_init(uint32_t freq);
void pit_set_frequency(uint32_t freq);

// DOOM 포팅을 위한 틱 발생기
void pit_tick(void);
uint64_t pit_get_ticks(void);

#endif
