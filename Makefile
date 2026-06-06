# ---------------------------------
# toolchain
# ---------------------------------

PREFIX := /opt/cross
CC := $(PREFIX)/bin/x86_64-elf-gcc
AS := $(PREFIX)/bin/x86_64-elf-as

BUILD  := build
ISODIR := isodir

# ---------------------------------
# source layout
# ---------------------------------
# Kernel side (everything except libc impls and doom)
# glue = doomgeneric <-> 커널 플랫폼 훅 (커널 헤더 + doomgeneric.h 둘 다 본다)
KSRC_DIRS := arch drivers init kernel mm glue
# libc impl
LSRC_DIR  := libc/src
# DOOM
DSRC_DIR  := doom

# ---------------------------------
# include paths
# ---------------------------------
# Kernel: own headers + libc shims (for memcpy/memset etc.)
KERNEL_INC := -I include -I libc/include
# libc impl: libc headers + kernel headers (so it can call kmalloc, serial_*, etc.)
LIBC_INC   := -I libc/include -I include
# DOOM: ONLY libc — kernel internals are walled off
DOOM_INC   := -I libc/include -I doom

# ---------------------------------
# CFLAGS
# ---------------------------------
CFLAGS_BASE    := -ffreestanding -m64 -mcmodel=kernel -fno-stack-protector \
                  -fno-pie -mno-red-zone -Wall -Wextra
CFLAGS_DEBUG   := $(CFLAGS_BASE) -O0 -g3 -fno-omit-frame-pointer
CFLAGS_RELEASE := $(CFLAGS_BASE) -O2 -g
CFLAGS         ?= $(CFLAGS_RELEASE)

LDFLAGS := -ffreestanding -nostdlib -no-pie -Wl,--build-id=none -Wl,-z,noexecstack

# DOOM (1993 K&R-ish) needs warning relaxation so it can compile against our
# minimal libc shims. Goal: compile cleanly so the linker reports the *exact*
# libc symbols we still need to implement.
DOOM_CFLAGS := -Wno-implicit-function-declaration \
               -Wno-int-conversion \
               -Wno-builtin-declaration-mismatch \
               -Wno-incompatible-pointer-types \
               -Wno-pointer-sign \
               -Wno-unused-parameter \
               -Wno-unused-variable \
               -Wno-unused-but-set-variable \
               -Wno-unused-function \
               -Wno-sign-compare \
               -Wno-format \
               -Wno-implicit-int \
               -Wno-discarded-qualifiers \
               -fno-strict-aliasing \
               -fno-builtin

# ---------------------------------
# source discovery
# ---------------------------------
KERNEL_C_SRCS := $(shell find $(KSRC_DIRS) -name "*.c")
KERNEL_S_SRCS := $(shell find $(KSRC_DIRS) -name "*.S")
KERNEL_s_SRCS := $(shell find $(KSRC_DIRS) -name "*.s")

LIBC_C_SRCS   := $(shell find $(LSRC_DIR) -name "*.c" 2>/dev/null)

DOOM_C_SRCS   := $(shell find $(DSRC_DIR) -name "*.c" 2>/dev/null)

# ---------------------------------
# object files
# ---------------------------------
KERNEL_OBJS := $(patsubst %.c,$(BUILD)/%.o,$(KERNEL_C_SRCS)) \
               $(patsubst %.S,$(BUILD)/%.o,$(KERNEL_S_SRCS)) \
               $(patsubst %.s,$(BUILD)/%.o,$(KERNEL_s_SRCS))
LIBC_OBJS   := $(patsubst %.c,$(BUILD)/%.o,$(LIBC_C_SRCS))
DOOM_OBJS   := $(patsubst %.c,$(BUILD)/%.o,$(DOOM_C_SRCS))

OBJS := $(KERNEL_OBJS) $(LIBC_OBJS) $(DOOM_OBJS)

# ---------------------------------
# targets
# ---------------------------------
.PHONY: all clean iso run run-debug

all: $(BUILD)/kernel.elf

$(BUILD)/kernel.elf: $(OBJS)
	$(CC) -T linker.ld -o $@ $(LDFLAGS) $(OBJS) -lgcc

# ---------------------------------
# compile rules
# Order matters: more specific patterns must precede the generic ones
# so GNU make picks the shorter-stem match.
# ---------------------------------

# DOOM .c
$(BUILD)/doom/%.o: doom/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DOOM_INC) $(DOOM_CFLAGS) -c $< -o $@

# glue (doomgeneric 플랫폼 훅): 커널 헤더 + doomgeneric.h 둘 다 필요해 -I doom 추가
$(BUILD)/glue/%.o: glue/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(KERNEL_INC) -I doom -c $< -o $@

# libc impls
$(BUILD)/libc/%.o: libc/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LIBC_INC) -c $< -o $@

# Kernel — generic rules
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(KERNEL_INC) -c $< -o $@

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(KERNEL_INC) -c $< -o $@

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(KERNEL_INC) -c $< -o $@

# ---------------------------------
# ISO
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
# run
# ---------------------------------
run:
	$(MAKE) CFLAGS="$(CFLAGS_RELEASE)" $(BUILD)/myos.iso
	qemu-system-x86_64 \
		-m 8G \
		-cdrom $(BUILD)/myos.iso \
		-bios /usr/share/OVMF/OVMF_CODE.fd \
		-vga none \
		-device virtio-vga,xres=1280,yres=800 \
		-serial stdio \
		-no-reboot \
		-no-shutdown

run-debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS_DEBUG)" $(BUILD)/myos.iso
	qemu-system-x86_64 \
	  -m 8G \
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
