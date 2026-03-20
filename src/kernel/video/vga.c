#include "vga.h"
#include "drivers/io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static size_t terminal_row = 0;
static size_t terminal_column = 0;
static unsigned const char default_terminal_color = 0x07;
static unsigned char terminal_color = default_terminal_color;
static unsigned short* terminal_buffer = (unsigned short*) 0xB8000;
static unsigned short back_buffer[VGA_WIDTH * VGA_HEIGHT];

static unsigned short vga_entry(unsigned char uc, unsigned char color) {
    return (unsigned short) uc | (unsigned short) color << 8;
}

static void vga_flush(void)
{
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        terminal_buffer[i] = back_buffer[i];
}

void vga_disable_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

unsigned short vga_read_cell(size_t row, size_t col)
{
    if (row >= VGA_HEIGHT || col >= VGA_WIDTH)
        return 0;
    return back_buffer[row * VGA_WIDTH + col];
}

void vga_write_cell(size_t row, size_t col, unsigned short value)
{
    if (row >= VGA_HEIGHT || col >= VGA_WIDTH)
        return;
    back_buffer[row * VGA_WIDTH + col] = value;
    terminal_buffer[row * VGA_WIDTH + col] = value;
}


void vga_enable_cursor(uint8_t start, uint8_t end)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

static void vga_update_cursor()
{
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_set_color(uint8_t color)
{
    terminal_color = color;
}

void vga_set_cursor(size_t row, size_t col)
{
    if (row < VGA_HEIGHT)
        terminal_row = row;

    if (col < VGA_WIDTH)
        terminal_column = col;
}


void vga_scroll()
{
    if (terminal_row >= VGA_HEIGHT)
    {
        // Move everything one line up
        for (size_t y = 1; y < VGA_HEIGHT; y++)
        {
            for (size_t x = 0; x < VGA_WIDTH; x++)
            {
                back_buffer[(y - 1) * VGA_WIDTH + x] =
                    back_buffer[y * VGA_WIDTH + x];
            }
        }

        // Clear last line
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            back_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
                vga_entry(' ', terminal_color);
        }

        terminal_row = VGA_HEIGHT - 1;
        vga_flush();
    }
}


void vga_print_dec(uint32_t num) {
    char buffer[11];
    int i = 0;

    if (num == 0) {
        vga_print("0");
        return;
    }

    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    for (int j = i - 1; j >= 0; j--) {
        char c[2] = { buffer[j], '\0' };
        vga_print(c);
    }
}

void vga_clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            back_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
    vga_flush();
}

void vga_putchar(char c) {
    if (c == '\n') 
    {
        terminal_column = 0;
        terminal_row++;
        vga_scroll();
        return;
    }
    else if (c == '\b') 
    {
        if (terminal_column > 0) 
        {
            terminal_column--;
        }
        else if (terminal_row > 0)
        {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }

        size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
        vga_update_cursor();

        return;
    }



    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    back_buffer[index] = vga_entry(c, terminal_color);

    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
    vga_scroll();
    vga_flush();
    vga_update_cursor();
}

void vga_print(const char* str) 
{
    for (size_t i = 0; str[i] != '\0'; i++) 
    {
        vga_putchar(str[i]);
    }
}


void vga_print_hex(uint32_t value)
{
    char hex_chars[] = "0123456789ABCDEF";

    char buffer[11];   // 0x + 8 hex digits + null
    buffer[0] = '0';
    buffer[1] = 'x';

    // We print from most significant nibble to least
    for (int i = 0; i < 8; i++)
    {
        // Shift right and isolate 4 bits
        uint32_t shift = 28 - (i * 4);
        uint32_t nibble = (value >> shift) & 0xF;

        buffer[2 + i] = hex_chars[nibble];
    }

    buffer[10] = '\0';

    vga_print(buffer);
}

size_t vga_get_column(void)
{
    return terminal_column;
}


void vga_print_color(const char* str, unsigned char color) 
{
    terminal_color = color;
    for (size_t i = 0; str[i] != '\0'; i++) 
    {
        vga_putchar(str[i]);
    }
    terminal_color = default_terminal_color;
}
