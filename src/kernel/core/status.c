#include "status.h"
#include "video/vga.h"
#include "core/time.h"


#define COLOR_DEFAULT 0x07
#define COLOR_GREEN   0x0A
#define COLOR_RED     0x0C
// #define COLOR_YELLOW  0x0E

#define STATUS_COLUMN 60

void status_loading_animation(uint32_t duration_ms)
{
    size_t current = vga_get_column();

    if (current >= STATUS_COLUMN)
        return;

    size_t dots = STATUS_COLUMN - current;

    if (dots == 0)
        return;

    uint32_t delay = duration_ms / dots;

    if (delay == 0)
        delay = 1;

    for (size_t i = 0; i < dots; i++)
    {
        vga_print(".");
        sleep(delay);
    }
}



static void status_align(void)
{
    size_t current = vga_get_column();

    if (current >= STATUS_COLUMN)
        return;

    size_t dots = STATUS_COLUMN - current;

    for (size_t i = 0; i < dots; i++)
        vga_print(".");
}


// void status_ok(const char* message)
// {
//     vga_set_color(COLOR_GREEN);
//     vga_print("[ OK ] ");   
//     vga_set_color(COLOR_DEFAULT);
//     vga_print(message);
//     vga_print("\n");
// }

// void status_error(const char* message)
// {
//     vga_set_color(COLOR_RED);
//     vga_print("[ ERR ] ");   
//     vga_set_color(COLOR_DEFAULT);
//     vga_print(message);
//     vga_print("\n");
// }

// void status_info(const char* message)
// {
//     vga_set_color(COLOR_YELLOW);
//     vga_print("[ I ] "); 
//     vga_set_color(COLOR_DEFAULT);
//     vga_print(message);
//     vga_print("\n");
// }

void status_begin(const char* message)
{
    vga_set_color(COLOR_DEFAULT);
    vga_print(message);
}

void status_end_ok(void)
{
    status_align();
    vga_set_color(COLOR_GREEN);
    vga_print(" [ OK ]\n");
    vga_set_color(COLOR_DEFAULT);
}

void status_end_error(void)
{
    status_align();
    vga_set_color(COLOR_RED);
    vga_print(" [ ERROR ]\n");
    vga_set_color(COLOR_DEFAULT);
}
