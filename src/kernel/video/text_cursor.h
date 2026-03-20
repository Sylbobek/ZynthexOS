#ifndef TEXT_CURSOR_H
#define TEXT_CURSOR_H

#include <stdint.h>
#include "drivers/mouse.h"

void text_cursor_init(uint32_t col, uint32_t row, uint8_t attr);
void text_cursor_tick(void); // consumes mouse events and moves cursor
void text_cursor_handle_event(const mouse_event_t* ev);
void text_cursor_draw(void);

#endif
