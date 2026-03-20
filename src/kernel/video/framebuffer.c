#include "framebuffer.h"
#include "core/multiboot.h"
#include "core/log.h"
#include "lib/string.h"
#include "font8x8.h"

fb_info_t framebuffer = {0};

static inline uint8_t* fb_target_base(void)
{
    return framebuffer.backbuffer ? framebuffer.backbuffer : framebuffer.addr;
}

static inline uint32_t fb_target_pitch(void)
{
    return framebuffer.backbuffer ? framebuffer.backbuffer_pitch : framebuffer.pitch;
}

static inline uint32_t fb_pack_color(uint32_t argb)
{
    uint8_t r = (argb >> 16) & 0xFF;
    uint8_t g = (argb >> 8)  & 0xFF;
    uint8_t b = argb & 0xFF;

    uint32_t color = 0;

    if (framebuffer.red_mask)
        color |= ((uint32_t)(r >> (8 - framebuffer.red_mask))) << framebuffer.red_pos;
    if (framebuffer.green_mask)
        color |= ((uint32_t)(g >> (8 - framebuffer.green_mask))) << framebuffer.green_pos;
    if (framebuffer.blue_mask)
        color |= ((uint32_t)(b >> (8 - framebuffer.blue_mask))) << framebuffer.blue_pos;

    return color;
}

void framebuffer_set_backbuffer(uint8_t* buf, uint32_t pitch)
{
    framebuffer.backbuffer = buf;
    framebuffer.backbuffer_pitch = pitch;
}

void framebuffer_flush(void)
{
    if (!framebuffer.available || framebuffer.backbuffer == NULL)
        return;

    uint32_t bpp_bytes = framebuffer.bpp / 8;
    if (bpp_bytes == 0)
        return;

    uint8_t* src = framebuffer.backbuffer;
    uint8_t* dst = framebuffer.addr;
    uint32_t src_pitch = framebuffer.backbuffer_pitch;
    uint32_t dst_pitch = framebuffer.pitch;

    uint32_t copy_bytes = framebuffer.width * bpp_bytes;
    for (uint32_t y = 0; y < framebuffer.height; y++)
    {
        memcpy(dst + y * dst_pitch, src + y * src_pitch, copy_bytes);
    }
}

void framebuffer_present(const uint8_t* src, uint32_t src_pitch)
{
    if (!framebuffer.available || src == NULL)
        return;

    uint32_t bpp_bytes = framebuffer.bpp / 8;
    if (bpp_bytes == 0)
        return;

    uint8_t* dst = framebuffer.addr;
    uint32_t dst_pitch = framebuffer.pitch;
    uint32_t copy_bytes = framebuffer.width * bpp_bytes;

    for (uint32_t y = 0; y < framebuffer.height; y++)
    {
        memcpy(dst + y * dst_pitch, src + y * src_pitch, copy_bytes);
    }
}

static inline void fb_write_pixel(uint8_t* dst, uint32_t packed, uint8_t bytes_per_pixel)
{
    switch (bytes_per_pixel)
    {
    case 4:
        *(uint32_t*)dst = packed;
        break;
    case 3:
        dst[0] = (uint8_t)(packed & 0xFF);
        dst[1] = (uint8_t)((packed >> 8) & 0xFF);
        dst[2] = (uint8_t)((packed >> 16) & 0xFF);
        break;
    case 2:
        *(uint16_t*)dst = (uint16_t)packed;
        break;
    default:
        break;
    }
}

static int framebuffer_parse(multiboot_info_t* mbi)
{
    if (!(mbi->flags & (1 << 12))) // framebuffer info valid
        return 0;

    framebuffer.addr   = (uint8_t*)(uintptr_t)mbi->framebuffer_addr;
    framebuffer.pitch  = mbi->framebuffer_pitch;
    framebuffer.width  = mbi->framebuffer_width;
    framebuffer.height = mbi->framebuffer_height;
    framebuffer.bpp    = mbi->framebuffer_bpp;
    framebuffer.type   = mbi->framebuffer_type;

    if (framebuffer.type != 1) // not RGB
        return 0;

    framebuffer.red_pos   = mbi->color_info[0];
    framebuffer.red_mask  = mbi->color_info[1];
    framebuffer.green_pos = mbi->color_info[2];
    framebuffer.green_mask= mbi->color_info[3];
    framebuffer.blue_pos  = mbi->color_info[4];
    framebuffer.blue_mask = mbi->color_info[5];

    framebuffer.available = 1;
    return 1;
}

void framebuffer_init(uint32_t multiboot_addr)
{
    multiboot_info_t* mbi = (multiboot_info_t*)multiboot_addr;
    if (framebuffer_parse(mbi))
    {
        log_write("fb: available ");
        log_write_dec(framebuffer.width);
        log_write("x");
        log_write_dec(framebuffer.height);
        log_write("x");
        log_write_dec(framebuffer.bpp);
        log_write(" pitch=");
        log_write_dec(framebuffer.pitch);
        log_write("\n");
    }
    else
    {
        log_write("fb: unavailable\n");
    }
}

void framebuffer_clear(uint32_t argb)
{
    if (!framebuffer.available)
        return;

    uint32_t packed = fb_pack_color(argb);
    uint8_t* base = fb_target_base();
    uint32_t pitch = fb_target_pitch();
    uint8_t bpp_bytes = framebuffer.bpp / 8;

    for (uint32_t y = 0; y < framebuffer.height; y++)
    {
        uint8_t* row = base + y * pitch;
        for (uint32_t x = 0; x < framebuffer.width; x++)
            fb_write_pixel(row + x * bpp_bytes, packed, bpp_bytes);
    }
}

// Draw single 8x8 character (transparent background)
void framebuffer_draw_char(uint32_t x, uint32_t y, char c, uint32_t argb)
{
    if (!framebuffer.available)
        return;

    if ((uint32_t)c < 32 || (uint32_t)c > 127)
        c = '?';

    uint8_t bpp_bytes = framebuffer.bpp / 8;
    if (bpp_bytes == 0)
        return;

    uint32_t packed = fb_pack_color(argb);
    const uint8_t* glyph = font8x8_basic[(uint32_t)c - 32];

    uint8_t* base = fb_target_base();
    uint32_t pitch = fb_target_pitch();

    for (uint32_t row = 0; row < 8; row++)
    {
        uint8_t bits = glyph[row];
        uint8_t* dst = base + (y + row) * pitch + x * bpp_bytes;
        for (uint32_t col = 0; col < 8; col++)
        {
            if (bits & (1u << (7 - col)))
                fb_write_pixel(dst + col * bpp_bytes, packed, bpp_bytes);
        }
    }
}

void framebuffer_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t argb)
{
    if (!framebuffer.available || str == NULL)
        return;
    uint32_t cursor_x = x;
    while (*str)
    {
        if (cursor_x + 8 > framebuffer.width)
            break;
        framebuffer_draw_char(cursor_x, y, *str, argb);
        cursor_x += 8;
        str++;
    }
}

// progress ring using fixed-point sin/cos table (16 steps basis)
static const int8_t ring_cos[16] = {127,115,90,49,0,-49,-90,-115,-127,-115,-90,-49,0,49,90,115};
static const int8_t ring_sin[16] = {0,49,90,115,127,115,90,49,0,-49,-90,-115,-127,-115,-90,-49};

void framebuffer_progress_ring(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t steps, uint32_t active, uint32_t color)
{
    if (!framebuffer.available || steps == 0)
        return;

    uint32_t bpp_bytes = framebuffer.bpp / 8;
    if (bpp_bytes == 0)
        return;

    uint32_t packed_active = fb_pack_color(color);
    uint32_t packed_inactive = fb_pack_color(0x00404040);

    uint32_t use_steps = steps > 16 ? 16 : steps;
    for (uint32_t i = 0; i < use_steps; i++)
    {
        int idx = (int)((i * 16) / use_steps) & 15;
        int dx = (radius * ring_cos[idx]) / 127;
        int dy = (radius * ring_sin[idx]) / 127;
        uint32_t px = cx + dx;
        uint32_t py = cy + dy;
        uint32_t packed = (i < active) ? packed_active : packed_inactive;

        if (px < framebuffer.width && py < framebuffer.height)
        {
            uint8_t* row = fb_target_base() + py * fb_target_pitch();
            fb_write_pixel(row + px * bpp_bytes, packed, bpp_bytes);
            // small 3x3 dot for visibility
            if (px + 1 < framebuffer.width)
                fb_write_pixel(row + (px + 1) * bpp_bytes, packed, bpp_bytes);
            if (py + 1 < framebuffer.height)
            {
                uint8_t* row2 = fb_target_base() + (py + 1) * fb_target_pitch();
                fb_write_pixel(row2 + px * bpp_bytes, packed, bpp_bytes);
            }
        }
    }
}

void framebuffer_test_pattern(void)
{
    if (!framebuffer.available)
        return;

    uint32_t colors[] = {
        0x00FF0000, // Red
        0x0000FF00, // Green
        0x000000FF, // Blue
        0x00FFFF00, // Yellow
        0x0000FFFF, // Cyan
        0x00FF00FF, // Magenta
        0x00FFFFFF, // White
        0x00000000  // Black
    };
    uint32_t bars = sizeof(colors) / sizeof(colors[0]);
    uint32_t bar_w = framebuffer.width / bars;

    for (uint32_t i = 0; i < bars; i++)
    {
        uint32_t x = i * bar_w;
        uint32_t w = (i == bars - 1) ? (framebuffer.width - x) : bar_w;
        framebuffer_fill_rect(x, 0, w, framebuffer.height, colors[i]);
    }
}

void framebuffer_putpixel(uint32_t x, uint32_t y, uint32_t argb)
{
    if (!framebuffer.available)
        return;

    if (x >= framebuffer.width || y >= framebuffer.height)
        return;

    uint8_t bpp_bytes = framebuffer.bpp / 8;
    if (bpp_bytes == 0)
        return;

    uint32_t packed = fb_pack_color(argb);
    uint8_t* base = fb_target_base();
    uint32_t pitch = fb_target_pitch();
    uint8_t* dst = base + y * pitch + x * bpp_bytes;
    fb_write_pixel(dst, packed, bpp_bytes);
}

void framebuffer_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb)
{
    if (!framebuffer.available || w == 0 || h == 0)
        return;

    if (x >= framebuffer.width || y >= framebuffer.height)
        return;

    uint32_t max_w = framebuffer.width - x;
    uint32_t max_h = framebuffer.height - y;
    if (w > max_w) w = max_w;
    if (h > max_h) h = max_h;

    uint8_t bpp_bytes = framebuffer.bpp / 8;
    if (bpp_bytes == 0)
        return;

    uint32_t packed = fb_pack_color(argb);
    uint8_t* base = fb_target_base() + y * fb_target_pitch() + x * bpp_bytes;
    uint32_t pitch = fb_target_pitch();

    for (uint32_t yy = 0; yy < h; yy++)
    {
        uint8_t* row = base + yy * pitch;
        for (uint32_t xx = 0; xx < w; xx++)
            fb_write_pixel(row + xx * bpp_bytes, packed, bpp_bytes);
    }
}

void framebuffer_fill_gradient_vert(uint32_t top_argb, uint32_t bottom_argb)
{
    if (!framebuffer.available)
        return;

    uint8_t top_r = (top_argb >> 16) & 0xFF;
    uint8_t top_g = (top_argb >> 8)  & 0xFF;
    uint8_t top_b = top_argb & 0xFF;

    uint8_t bot_r = (bottom_argb >> 16) & 0xFF;
    uint8_t bot_g = (bottom_argb >> 8)  & 0xFF;
    uint8_t bot_b = bottom_argb & 0xFF;

    uint32_t height = framebuffer.height ? framebuffer.height : 1;
    uint8_t bpp_bytes = framebuffer.bpp / 8;
    if (bpp_bytes == 0)
        return;

    if (height == 1)
    {
        uint32_t packed = fb_pack_color((top_r << 16) | (top_g << 8) | top_b);
        uint8_t* row = framebuffer.addr;
        for (uint32_t x = 0; x < framebuffer.width; x++)
            fb_write_pixel(row + x * bpp_bytes, packed, bpp_bytes);
        return;
    }

    for (uint32_t y = 0; y < framebuffer.height; y++)
    {
        uint8_t r = top_r + (uint8_t)(((int32_t)(bot_r - top_r) * (int32_t)y) / (int32_t)(height - 1));
        uint8_t g = top_g + (uint8_t)(((int32_t)(bot_g - top_g) * (int32_t)y) / (int32_t)(height - 1));
        uint8_t b = top_b + (uint8_t)(((int32_t)(bot_b - top_b) * (int32_t)y) / (int32_t)(height - 1));

        uint32_t packed = fb_pack_color((r << 16) | (g << 8) | b);
        uint8_t* row = fb_target_base() + y * fb_target_pitch();
        for (uint32_t x = 0; x < framebuffer.width; x++)
            fb_write_pixel(row + x * bpp_bytes, packed, bpp_bytes);
    }
}

void framebuffer_draw_line(int x0, int y0, int x1, int y1, uint32_t argb)
{
    if (!framebuffer.available)
        return;

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0); // negative for Bresenham variant
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy; // error term

    while (1)
    {
        framebuffer_putpixel((uint32_t)x0, (uint32_t)y0, argb);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void framebuffer_demo_bootscreen(void)
{
    if (!framebuffer.available)
        return;

    // Windows 98 style: teal background and gray taskbar
    framebuffer_clear(0x00008080); // solid teal

    uint32_t taskbar_h = framebuffer.height / 12;
    uint32_t taskbar_y = framebuffer.height - taskbar_h;
    uint32_t tb_light = 0x00C0C0C0; // light gray
    uint32_t tb_dark  = 0x00808080; // dark edge
    framebuffer_fill_rect(0, taskbar_y, framebuffer.width, taskbar_h, tb_light);
    framebuffer_draw_line(0, (int)taskbar_y - 1, (int)framebuffer.width - 1, (int)taskbar_y - 1, tb_dark);

    // Start button-ish block
    uint32_t sb_w = framebuffer.width / 10;
    uint32_t sb_h = taskbar_h - 6;
    uint32_t sb_x = 4;
    uint32_t sb_y = taskbar_y + 3;
    framebuffer_fill_rect(sb_x, sb_y, sb_w, sb_h, 0x00C0DCC0); // classic 98 button tint
    framebuffer_draw_line((int)sb_x, (int)sb_y, (int)(sb_x + sb_w - 1), (int)sb_y, 0x00FFFFFF);
    framebuffer_draw_line((int)sb_x, (int)sb_y, (int)sb_x, (int)(sb_y + sb_h - 1), 0x00FFFFFF);
    framebuffer_draw_line((int)sb_x + 1, (int)(sb_y + sb_h - 1), (int)(sb_x + sb_w - 1), (int)(sb_y + sb_h - 1), 0x00808080);
    framebuffer_draw_line((int)(sb_x + sb_w - 1), (int)sb_y + 1, (int)(sb_x + sb_w - 1), (int)(sb_y + sb_h - 1), 0x00808080);

    // Center window-like panel
    uint32_t win_w = framebuffer.width / 3;
    uint32_t win_h = framebuffer.height / 3;
    uint32_t win_x = (framebuffer.width - win_w) / 2;
    uint32_t win_y = (framebuffer.height - win_h) / 2 - taskbar_h / 2;

    // Window frame
    framebuffer_fill_rect(win_x, win_y, win_w, win_h, 0x00C0C0C0);
    framebuffer_draw_line((int)win_x, (int)win_y, (int)(win_x + win_w - 1), (int)win_y, 0x00FFFFFF);
    framebuffer_draw_line((int)win_x, (int)win_y, (int)win_x, (int)(win_y + win_h - 1), 0x00FFFFFF);
    framebuffer_draw_line((int)win_x + 1, (int)(win_y + win_h - 1), (int)(win_x + win_w - 1), (int)(win_y + win_h - 1), 0x00808080);
    framebuffer_draw_line((int)(win_x + win_w - 1), (int)win_y + 1, (int)(win_x + win_w - 1), (int)(win_y + win_h - 1), 0x00808080);

    // Title bar
    uint32_t bar_h = win_h / 6;
    framebuffer_fill_rect(win_x + 1, win_y + 1, win_w - 2, bar_h, 0x00008080);
    framebuffer_draw_line((int)win_x + 1, (int)win_y + 1, (int)(win_x + win_w - 2), (int)win_y + 1, 0x00FFFFFF);
    framebuffer_draw_line((int)win_x + 1, (int)win_y + 1, (int)win_x + 1, (int)(win_y + bar_h), 0x00FFFFFF);
    framebuffer_draw_line((int)win_x + 1, (int)(win_y + bar_h), (int)(win_x + win_w - 2), (int)(win_y + bar_h), 0x00000000);
    framebuffer_draw_line((int)(win_x + win_w - 2), (int)win_y + 1, (int)(win_x + win_w - 2), (int)(win_y + bar_h), 0x00000000);

    // Inner client area
    uint32_t client_y = win_y + bar_h + 2;
    uint32_t client_h = win_h - bar_h - 3;
    framebuffer_fill_rect(win_x + 2, client_y, win_w - 4, client_h - 2, 0x00FFFFFF);
}

// Simple blit: copy a linear buffer (src_pitch) into framebuffer region (with bounds clamp)
void framebuffer_blit(uint32_t dst_x, uint32_t dst_y, uint32_t w, uint32_t h,
                      const uint8_t* src, uint32_t src_pitch, uint8_t src_bpp)
{
    if (!framebuffer.available || src == NULL || w == 0 || h == 0)
        return;

    if (dst_x >= framebuffer.width || dst_y >= framebuffer.height)
        return;

    uint8_t dst_bpp_bytes = framebuffer.bpp / 8;
    if (dst_bpp_bytes == 0 || src_bpp == 0)
        return;

    uint32_t max_w = framebuffer.width - dst_x;
    uint32_t max_h = framebuffer.height - dst_y;
    if (w > max_w) w = max_w;
    if (h > max_h) h = max_h;

    uint32_t copy_bytes_per_pixel = (src_bpp < dst_bpp_bytes * 8) ? src_bpp / 8 : dst_bpp_bytes;
    if (copy_bytes_per_pixel == 0)
        copy_bytes_per_pixel = dst_bpp_bytes;

    for (uint32_t yy = 0; yy < h; yy++)
    {
        const uint8_t* srow = src + yy * src_pitch;
        uint8_t* base = fb_target_base() + (dst_y + yy) * fb_target_pitch() + dst_x * dst_bpp_bytes;

        for (uint32_t xx = 0; xx < w; xx++)
        {
            // For simplicity assume src pixels are already in framebuffer native format or byte-identical
            for (uint32_t b = 0; b < copy_bytes_per_pixel; b++)
                base[xx * dst_bpp_bytes + b] = srow[xx * copy_bytes_per_pixel + b];
        }
    }
}
