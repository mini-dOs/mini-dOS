#ifndef I8042_H
#define I8042_H

#include <stdint.h>

uint8_t i8042_get_status();
void i8042_send_command(uint8_t command);
void i8042_send_data(uint8_t data);
uint8_t i8042_read_data();
void i8042_init();

#endif
