#include "text_cursor.h"
#include "video/vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint32_t tcur_col = 0;
static uint32_t tcur_row = 0;
static uint8_t  tcur_attr = 0x70; // inverse bright bg
static unsigned short saved_cell = 0;
static int has_saved = 0;
static int accum_x = 0;
static int accum_y = 0;

static inline uint32_t clamp_u32(uint32_t v, uint32_t max)
{
    return (v > max) ? max : v;
}

static void restore_cell(void)
{
    if (has_saved)
    {
        vga_write_cell(tcur_row, tcur_col, saved_cell);
        has_saved = 0;
    }
}

void text_cursor_init(uint32_t col, uint32_t row, uint8_t attr)
{
    restore_cell();
    tcur_col = clamp_u32(col, VGA_WIDTH - 1);
    tcur_row = clamp_u32(row, VGA_HEIGHT - 1);
    tcur_attr = attr;
    accum_x = accum_y = 0;
    text_cursor_draw();
}

void text_cursor_handle_event(const mouse_event_t* ev)
{
    if (!ev)
        return;

    accum_x += ev->dx;
    accum_y += ev->dy;

    int moved = 0;
    while (accum_x >= 4) { if (tcur_col < VGA_WIDTH - 1) tcur_col++; accum_x -= 4; moved = 1; }
    while (accum_x <= -4) { if (tcur_col > 0) tcur_col--; accum_x += 4; moved = 1; }
    while (accum_y >= 4) { if (tcur_row < VGA_HEIGHT - 1) tcur_row++; accum_y -= 4; moved = 1; }
    while (accum_y <= -4) { if (tcur_row > 0) tcur_row--; accum_y += 4; moved = 1; }

    if (moved)
        text_cursor_draw();
}

void text_cursor_tick(void)
{
    mouse_event_t ev;
    while (mouse_read_event(&ev))
        text_cursor_handle_event(&ev);
}

void text_cursor_draw(void)
{
    restore_cell();
    saved_cell = vga_read_cell(tcur_row, tcur_col);
    has_saved = 1;

    unsigned char ch = 0xDB; // solid block
    unsigned short cell = (unsigned short)ch | ((unsigned short)tcur_attr << 8);
    vga_write_cell(tcur_row, tcur_col, cell);
}
