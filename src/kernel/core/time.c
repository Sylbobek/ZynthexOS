#include "time.h"
#include "debug.h"
#include "video/vga.h"
#include "video/cursor.h"
#include "video/text_cursor.h"
#include "video/framebuffer.h"
#include "core/mouse_mode.h"


static volatile uint32_t ticks = 0;
static volatile uint32_t seconds = 0;

void timer_tick() 
{
    ticks++;

    // Update cursor from mouse events (FB cursor if available, otherwise text-mode cursor)
    if (mouse_is_enabled())
    {
        if (framebuffer.available)
            cursor_tick();
        else
            text_cursor_tick();
    }

    if (ticks % 100 == 0) 
    {
        seconds++;
    }
}

uint32_t timer_get_ticks() 
{
    return ticks;
}

uint32_t timer_get_seconds()
{
    return seconds;
}

void print_seconds()
{   
    static uint32_t previousSeconds = 0;
    uint32_t current = timer_get_seconds();

    if (previousSeconds != current)
    {
        vga_print_dec(current);
        vga_print(" seconds passed since boot\n");

        previousSeconds = current;
    }
}

void sleep(uint32_t ms) 
{
    uint32_t start = ticks;
    uint32_t wait_ticks = ms / 10;   // 100Hz = 10ms per tick

    while ((ticks - start) < wait_ticks) {
        asm volatile("hlt");
    }
}
