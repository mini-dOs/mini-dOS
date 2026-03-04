# Cross-compiler for x86_64 bare metal
CC  := x86_64-elf-gcc
AS  := x86_64-elf-as

CFLAGS  := -ffreestanding -m64 -fno-stack-protector -fno-pie -mno-red-zone -O2 -Wall -Wextra
LDFLAGS := -ffreestanding -nostdlib -no-pie -Wl,--build-id=none

BUILD := build
SRC   := src

OBJS :=	$(BUILD)/boot.o \
	$(BUILD)/kernel.o \
	$(BUILD)/gdt.o \
	$(BUILD)/lgdt_asm.o

.PHONY: all clean iso run

all: $(BUILD)/kernel.elf

$(BUILD)/kernel.elf: $(OBJS)
	$(CC) -T linker.ld -o $@ $(LDFLAGS) $(OBJS) -lgcc

$(BUILD)/boot.o: $(SRC)/boot/boot.s | $(BUILD)
	$(AS) $(SRC)/boot/boot.s -o $@

$(BUILD)/kernel.o: $(SRC)/kernel/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $(SRC)/kernel/kernel.c -o $@

$(BUILD)/gdt.o: $(SRC)/kernel/gdt.c | $(BUILD)
	$(CC) $(CFLAGS) -c $(SRC)/kernel/gdt.c -o $@

$(BUILD)/lgdt_asm.o: $(SRC)/kernel/lgdt_asm.S | $(BUILD)
	$(AS) --64 $(SRC)/kernel/lgdt_asm.S -o $@

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
