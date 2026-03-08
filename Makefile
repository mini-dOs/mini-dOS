# cross compiler
CC := x86_64-elf-gcc
AS := x86_64-elf-as

SRC := src
BUILD := build

CFLAGS := -ffreestanding -m64 -fno-stack-protector -fno-pie -mno-red-zone -O2 -Wall -Wextra -I src/include
LDFLAGS := -ffreestanding -nostdlib -no-pie -Wl,--build-id=none

# ----------------------------
# source files
# ----------------------------

C_SOURCES := $(shell find $(SRC) -name "*.c")
ASM_SOURCES := $(shell find $(SRC) -name "*.S")
ASM_SOURCES += $(shell find $(SRC) -name "*.s")

# convert src paths -> build paths
C_OBJS := $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(C_SOURCES))
ASM_OBJS := $(patsubst $(SRC)/%.S,$(BUILD)/%.o,$(ASM_SOURCES))
ASM_OBJS += $(patsubst $(SRC)/%.s,$(BUILD)/%.o,$(ASM_SOURCES))

OBJS := $(C_OBJS) $(ASM_OBJS)

.PHONY: all clean iso run

# ----------------------------
# build kernel
# ----------------------------

all: $(BUILD)/kernel.elf

$(BUILD)/kernel.elf: $(OBJS)
	$(CC) -T linker.ld -o $@ $(LDFLAGS) $(OBJS) -lgcc


# ----------------------------
# compile C
# ----------------------------

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# ----------------------------
# compile asm (.S)
# ----------------------------

$(BUILD)/%.o: $(SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# ----------------------------
# compile asm (.s)
# ----------------------------

$(BUILD)/%.o: $(SRC)/%.s
	@mkdir -p $(dir $@)
	$(AS) $< -o $@


# ----------------------------
# ISO build
# ----------------------------

$(BUILD)/myos.iso: $(BUILD)/kernel.elf
	cp $(BUILD)/kernel.elf isodir/boot/kernel.elf
	grub-mkrescue -o $(BUILD)/myos.iso isodir

iso: $(BUILD)/myos.iso


# ----------------------------
# run
# ----------------------------

run: $(BUILD)/myos.iso
	qemu-system-x86_64 \
	  -m 256M \
	  -cdrom $(BUILD)/myos.iso \
	  -bios /usr/share/OVMF/OVMF_CODE.fd \
	  -serial stdio \
	  -no-reboot


# ----------------------------
# clean
# ----------------------------

clean:
	rm -rf $(BUILD)