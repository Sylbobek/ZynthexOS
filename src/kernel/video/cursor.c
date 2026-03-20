#include "cursor.h"
#include "video/framebuffer.h"
#include "drivers/mouse.h"

static uint32_t cur_x = 0;
static uint32_t cur_y = 0;
static uint32_t cur_color = 0x00FFFFFF;
static uint32_t cur_size = 5; // square size in pixels

static inline uint32_t clamp_u32(uint32_t val, uint32_t min, uint32_t max)
{
    return (val < min) ? min : (val > max ? max : val);
}

void cursor_init(uint32_t x, uint32_t y, uint32_t color)
{
    cur_x = x;
    cur_y = y;
    cur_color = color;
    cur_size = 5;
    cursor_draw();
}

void cursor_handle_event(const mouse_event_t* ev)
{
    if (!ev)
        return;

    int nx = (int)cur_x + ev->dx;
    int ny = (int)cur_y + ev->dy;

    if (framebuffer.available && framebuffer.width > 0 && framebuffer.height > 0)
    {
        cur_x = clamp_u32(nx < 0 ? 0 : (uint32_t)nx, 0, framebuffer.width - 1);
        cur_y = clamp_u32(ny < 0 ? 0 : (uint32_t)ny, 0, framebuffer.height - 1);
    }

    cursor_draw();
}

void cursor_tick(void)
{
    mouse_event_t ev;
    while (mouse_read_event(&ev))
        cursor_handle_event(&ev);
}

void cursor_draw(void)
{
    if (!framebuffer.available)
        return;

    uint32_t size = cur_size;
    uint32_t x = cur_x;
    uint32_t y = cur_y;

    // Center the square on (x, y)
    uint32_t half = size / 2;
    int start_x = (int)x - (int)half;
    int start_y = (int)y - (int)half;

    // Clamp to bounds
    uint32_t clamped_x = (start_x < 0) ? 0 : (uint32_t)start_x;
    uint32_t clamped_y = (start_y < 0) ? 0 : (uint32_t)start_y;
    uint32_t w = size;
    uint32_t h = size;
    if (clamped_x + w > framebuffer.width)  w = framebuffer.width  - clamped_x;
    if (clamped_y + h > framebuffer.height) h = framebuffer.height - clamped_y;

    framebuffer_fill_rect(clamped_x, clamped_y, w, h, cur_color);
}
