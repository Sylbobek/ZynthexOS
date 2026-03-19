// Simple linear framebuffer support (Multiboot v1 video mode request)
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t* addr;      // mapped physical address (identity-mapped early)
    uint32_t pitch;     // bytes per row
    uint32_t width;
    uint32_t height;
    uint8_t  bpp;       // bits per pixel
    uint8_t  type;      // 1 = RGB
    uint8_t  red_pos, red_mask;
    uint8_t  green_pos, green_mask;
    uint8_t  blue_pos, blue_mask;
    int      available;
} fb_info_t;

extern fb_info_t framebuffer;

// Initialize from multiboot_info pointer (physical address passed by boot.s)
void framebuffer_init(uint32_t multiboot_addr);

// Fill screen with a solid ARGB color (8:8:8, ignores alpha). No-op if unavailable.
void framebuffer_clear(uint32_t argb);

#endif
