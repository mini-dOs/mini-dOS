# ---------------------------------
# toolchain (fixed)
# ---------------------------------

PREFIX := /opt/cross
CC := $(PREFIX)/bin/x86_64-elf-gcc
AS := $(PREFIX)/bin/x86_64-elf-as

SRC := src
BUILD := build
ISODIR := isodir

CFLAGS_BASE := -ffreestanding -m64 -mcmodel=kernel -fno-stack-protector -fno-pie -mno-red-zone -Wall -Wextra -I $(SRC)/include
CFLAGS_DEBUG := $(CFLAGS_BASE) -O0 -g3 -fno-omit-frame-pointer
CFLAGS_RELEASE := $(CFLAGS_BASE) -O2 -g
CFLAGS ?= $(CFLAGS_RELEASE)
LDFLAGS := -ffreestanding -nostdlib -no-pie -Wl,--build-id=none -Wl,-z,noexecstack

# ---------------------------------
# source discovery
# ---------------------------------

C_SOURCES := $(shell find $(SRC) -name "*.c")
ASM_SOURCES_S := $(shell find $(SRC) -name "*.S")
ASM_SOURCES_s := $(shell find $(SRC) -name "*.s")

# ---------------------------------
# object files
# ---------------------------------

C_OBJS := $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(C_SOURCES))
ASM_OBJS := $(patsubst $(SRC)/%.S,$(BUILD)/%.o,$(ASM_SOURCES_S))
ASM_OBJS += $(patsubst $(SRC)/%.s,$(BUILD)/%.o,$(ASM_SOURCES_s))

OBJS := $(C_OBJS) $(ASM_OBJS)

# ---------------------------------
# targets
# ---------------------------------

.PHONY: all clean iso run run-debug

all: $(BUILD)/kernel.elf

# ---------------------------------
# link kernel
# ---------------------------------

$(BUILD)/kernel.elf: $(OBJS)
	$(CC) -T linker.ld -o $@ $(LDFLAGS) $(OBJS) -lgcc

# ---------------------------------
# compile rules
# ---------------------------------

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRC)/%.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------
# ISO build (FINAL, stable)
# ---------------------------------

$(BUILD)/myos.iso: $(BUILD)/kernel.elf
	@echo "[*] Preparing ISO directory"
	mkdir -p $(ISODIR)/boot/grub
	rm -f $(ISODIR)/boot/kernel.elf

	cp $(BUILD)/kernel.elf $(ISODIR)/boot/kernel.elf
	@if [ ! -f $(ISODIR)/boot/grub/grub.cfg ]; then \
		cp grub.cfg $(ISODIR)/boot/grub/grub.cfg; \
	fi

	@echo "[*] Building hybrid ISO (BIOS + UEFI)"
	grub-mkrescue \
	  -o $(BUILD)/myos.iso \
	  $(ISODIR)

iso: $(BUILD)/myos.iso

# ---------------------------------
# run (UEFI - OVMF)
# ---------------------------------

run:
	$(MAKE) CFLAGS="$(CFLAGS_RELEASE)" $(BUILD)/myos.iso
	qemu-system-x86_64 \
		-m 8G \
		-cdrom $(BUILD)/myos.iso \
		-bios /usr/share/OVMF/OVMF_CODE.fd \
		-vga std \
		-serial stdio \
		-no-reboot \
		-no-shutdown

run-debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS_DEBUG)" $(BUILD)/myos.iso
	qemu-system-x86_64 \
	  -m 2G \
	  -cdrom $(BUILD)/myos.iso \
	  -bios /usr/share/OVMF/OVMF_CODE.fd \
	  -display none \
	  -monitor none \
	  -serial stdio \
	  -no-reboot \
	  -no-shutdown \
	  -s -S \
	  -d int,guest_errors \
	  -D $(BUILD)/qemu-debug.log

# ---------------------------------
# clean
# ---------------------------------

clean:
	rm -rf $(BUILD)
	rm -f $(ISODIR)/boot/kernel.elf
