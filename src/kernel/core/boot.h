#ifndef BOOT_H
#define BOOT_H

#include <stdint.h>

void kernel_boot(uint32_t multiboot_magic, uint32_t multiboot_addr);

#endif
