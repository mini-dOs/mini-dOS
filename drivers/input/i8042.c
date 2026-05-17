#include <asm/cpu.h>
#include <drivers/i8042.h>
#include <drivers/serial.h>

#define I8042_DATA_PORT		0x60
#define I8042_STATUS_REG	0x64
#define I8042_COMMAND_REG	0x64

#define I8042_STATUS_OBF	0X01
#define I8042_STATUS_IBF	0X02

static void i8042_wait_write() {
	// IBF가 찼으면 1 => 처리해서 0이 될 때까지 무한 반복
	while(inb(I8042_STATUS_REG) & I8042_STATUS_IBF);
}

static void i8042_wait_read() {
	// OBF가 비어있으면 0 => 1이 될 때까지 무한 반복
	while(!(inb(I8042_STATUS_REG) & I8042_STATUS_OBF));
}

uint8_t i8042_get_status() {
	return inb(I8042_STATUS_REG);
}

void i8042_send_command(uint8_t command) {
	i8042_wait_write();
	outb(I8042_COMMAND_REG, command);
}

// 키보드가 먹통이 되거나, LED를 켜야할 때 명령(데이터)을 보내야 함
void i8042_send_data(uint8_t data) {
	i8042_wait_write();
	outb(I8042_DATA_PORT, data);
}

uint8_t i8042_read_data() {
	i8042_wait_read();
	return inb(I8042_DATA_PORT);
}

void i8042_flush_buffer() {
	while (inb(0x64) & 0x01) {
		inb(0x60);
	}
}

void i8042_init() {
	serial_write("Step 1: Flush\r\n");
	i8042_flush_buffer();

	// Test: i8042
	serial_write("Step 2: Send Command 0xAA\r\n");
	i8042_send_command(0xAA);

	uint8_t res = i8042_read_data();
	
	serial_write("Step 3: Waiting for 0x55\r\n");
	if (res == 0x55) {
		serial_write("Step 4: Controller OK\r\n");
	}
	else if (res == 0xFC) {
		serial_write("Step 4: Controller Error\r\n");
	}

	// Activate Keyboard Interface
	i8042_send_command(0xAE);

	// Test: Keyboard
	i8042_send_data(0xF4);

	// Check the answer is ACK(0xFA; 악 명령이 너무 좋아 해병님)
	if (i8042_read_data() == 0xFA) {
		serial_write("i8042: Keyboard Connected!\r\n");
	}
	else {
		serial_write("i8042: Keyboard Not Found!\r\n");
	}
}
