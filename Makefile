# Cross-compiler for x86_64 bare metal
CC  := x86_64-elf-gcc
AS  := x86_64-elf-as

CFLAGS  := -ffreestanding -m64 -fno-stack-protector -fno-pie -O2 -Wall -Wextra
LDFLAGS := -ffreestanding -nostdlib -Wl,--build-id=none

BUILD := build
SRC   := src

.PHONY: all clean iso run

all: $(BUILD)/kernel.elf

$(BUILD)/kernel.elf: $(BUILD)/boot.o $(BUILD)/kernel.o
	$(CC) -T $(SRC)/linker.ld -o $@ $(LDFLAGS) $(BUILD)/boot.o $(BUILD)/kernel.o -lgcc

$(BUILD)/boot.o: $(SRC)/boot.s | $(BUILD)
	$(AS) $(SRC)/boot.s -o $@

$(BUILD)/kernel.o: $(SRC)/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $(SRC)/kernel.c -o $@

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/myos.iso: $(BUILD)/kernel.elf
	cp $(BUILD)/kernel.elf isodir/boot/kernel.elf
	grub-mkrescue -o $(BUILD)/myos.iso isodir

iso: $(BUILD)/myos.iso

run: $(BUILD)/myos.iso
	qemu-system-x86_64 \
	  -m 256M \
	  -cdrom $(BUILD)/myos.iso \
	  -bios /usr/share/OVMF/OVMF_CODE.fd \
	  -serial stdio \
	  -no-reboot

clean:
	rm -rf $(BUILD)
