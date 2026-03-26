# ---------------------------------
# toolchain
# ---------------------------------

CC := x86_64-elf-gcc
AS := x86_64-elf-as

SRC := src
BUILD := build

CFLAGS := -ffreestanding -m64 -fno-stack-protector -fno-pie -mno-red-zone -O2 -Wall -Wextra -I $(SRC)/include
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

.PHONY: all clean iso run

all: $(BUILD)/kernel.elf


# ---------------------------------
# link kernel
# ---------------------------------

$(BUILD)/kernel.elf: $(OBJS)
	$(CC) -T linker.ld -o $@ $(LDFLAGS) $(OBJS) -lgcc


# ---------------------------------
# compile C
# ---------------------------------

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# ---------------------------------
# compile preprocessed asm (.S)
# ---------------------------------

$(BUILD)/%.o: $(SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# ---------------------------------
# compile raw asm (.s)
# ---------------------------------

$(BUILD)/%.o: $(SRC)/%.s
	@mkdir -p $(dir $@)
	$(AS) $< -o $@


# ---------------------------------
# ISO build
# ---------------------------------

$(BUILD)/myos.iso: $(BUILD)/kernel.elf
	cp $(BUILD)/kernel.elf isodir/boot/kernel.elf
	grub-mkrescue -o $(BUILD)/myos.iso isodir

iso: $(BUILD)/myos.iso


# ---------------------------------
# run
# ---------------------------------

run: $(BUILD)/myos.iso
	qemu-system-x86_64 \
	  -m 8G \
	  -cdrom $(BUILD)/myos.iso \
	  -bios /usr/share/OVMF/OVMF_CODE.fd \
	  -serial stdio \
	  -no-shutdown \
	  -no-reboot


# ---------------------------------
# clean
# ---------------------------------

clean:
	rm -rf $(BUILD)
