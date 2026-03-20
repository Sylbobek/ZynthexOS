#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

void vga_clear();
void vga_set_color(uint8_t color);
void vga_putchar(char c);
void vga_print(const char* str);
void vga_print_hex(uint32_t value); 
void vga_print_dec(uint32_t num);
void vga_print_color(const char* str, unsigned char color); 
size_t vga_get_column(void);
void vga_set_cursor(size_t row, size_t col);
void vga_enable_cursor(uint8_t start, uint8_t end);
void vga_disable_cursor();

// Direct cell access (character + attribute). Useful for overlays (e.g., mouse cursor in text mode).
unsigned short vga_read_cell(size_t row, size_t col);
void vga_write_cell(size_t row, size_t col, unsigned short value);

#endif
