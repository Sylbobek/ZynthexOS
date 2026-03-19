BUILD_DIR = build
SRC_DIR   = src

CFLAGS = -ffreestanding -fno-builtin -nostdlib -m32 \
         -Wall -Wextra -std=gnu99 \
         -I$(SRC_DIR)/kernel

LDFLAGS = -m elf_i386 -T linker.ld

C_SOURCES := $(shell find $(SRC_DIR) -name "*.c")
ASM_SOURCES := $(shell find $(SRC_DIR) -name "*.s")

C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

OBJS := $(C_OBJS) $(ASM_OBJS)

# =========================
# Main Targets
# =========================

all: $(BUILD_DIR)/zynthexos.iso

.PHONY: all clean

# =========================
# Build directories dynamically
# =========================

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	nasm -f elf32 $< -o $@

# =========================
# Linking
# =========================

$(BUILD_DIR)/kernel.bin: $(OBJS)
	ld $(LDFLAGS) $^ -o $@

# =========================
# ISO
# =========================

$(BUILD_DIR)/zynthexos.iso: $(BUILD_DIR)/kernel.bin
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/iso/boot/
	cp $(SRC_DIR)/grub/grub.cfg $(BUILD_DIR)/iso/boot/grub/
	grub-mkrescue -o $@ $(BUILD_DIR)/iso
	rm -rf $(BUILD_DIR)/iso

# =========================
# Clean
# =========================

clean:
	rm -rf $(BUILD_DIR)
