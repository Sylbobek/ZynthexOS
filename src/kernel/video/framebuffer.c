#include "framebuffer.h"
#include "core/multiboot.h"
#include "core/log.h"

fb_info_t framebuffer = {0};

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

    uint8_t* base = framebuffer.addr;
    uint32_t pitch = framebuffer.pitch;

    for (uint32_t y = 0; y < framebuffer.height; y++)
    {
        uint32_t* row = (uint32_t*)(base + y * pitch);
        for (uint32_t x = 0; x < framebuffer.width; x++)
            row[x] = argb;
    }
}
