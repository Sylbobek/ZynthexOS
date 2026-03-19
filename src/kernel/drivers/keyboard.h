#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);


void keyboard_irq_handler(void);


uint8_t keyboard_read_scancode(void);

char keyboard_scancode_to_ascii(uint8_t scancode);


int keyboard_has_input(void);

char keyboard_getchar(void);

#endif
