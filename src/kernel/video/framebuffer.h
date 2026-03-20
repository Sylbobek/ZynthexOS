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
    // Optional software backbuffer (caller-owned). If set, draw calls target it.
    uint8_t* backbuffer;
    uint32_t backbuffer_pitch;
    int      available;
} fb_info_t;

extern fb_info_t framebuffer;

// Initialize from multiboot_info pointer (physical address passed by boot.s)
void framebuffer_init(uint32_t multiboot_addr);

// Fill screen with a solid ARGB color (8:8:8, ignores alpha). No-op if unavailable.
void framebuffer_clear(uint32_t argb);

// Draw a single pixel if within bounds.
void framebuffer_putpixel(uint32_t x, uint32_t y, uint32_t argb);

// Fill a rectangle region with a solid color if within bounds.
void framebuffer_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb);

// Fill screen with a vertical gradient (top->bottom).
void framebuffer_fill_gradient_vert(uint32_t top_argb, uint32_t bottom_argb);

// Draw a line using integer Bresenham.
void framebuffer_draw_line(int x0, int y0, int x1, int y1, uint32_t argb);

// Simple blit from a linear buffer into the framebuffer.
void framebuffer_blit(uint32_t dst_x, uint32_t dst_y, uint32_t w, uint32_t h,
                      const uint8_t* src, uint32_t src_pitch, uint8_t src_bpp);

// Set a caller-provided backbuffer and pitch; drawing uses it until unset (NULL).
void framebuffer_set_backbuffer(uint8_t* buf, uint32_t pitch);

// Copy current backbuffer to hardware framebuffer; no-op if backbuffer unset.
void framebuffer_flush(void);

// Copy arbitrary source buffer (e.g., off-screen) to hardware framebuffer directly.
void framebuffer_present(const uint8_t* src, uint32_t src_pitch);

// Text rendering with 8x8 bitmap font (transparent background).
void framebuffer_draw_char(uint32_t x, uint32_t y, char c, uint32_t argb);
void framebuffer_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t argb);

// Small progress ring (Win-style) centered at (cx, cy) with given radius and step count; filled with color.
void framebuffer_progress_ring(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t steps, uint32_t active, uint32_t color);

// Diagnostic test pattern (color bars) to validate framebuffer output.
void framebuffer_test_pattern(void);

// Optional: simple demo screen reminiscent of Windows boot background.
void framebuffer_demo_bootscreen(void);

#endif
