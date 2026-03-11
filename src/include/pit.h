#ifndef PIT_H
#define PIT_H

#include <stdint.h>

void pit_init(uint32_t freq);
void pit_set_frequency(uint32_t freq);

#endif