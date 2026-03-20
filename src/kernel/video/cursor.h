#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>
#include "drivers/mouse.h"

void cursor_init(uint32_t x, uint32_t y, uint32_t color);
void cursor_tick(void); // read mouse events and update position
void cursor_draw(void); // draw cursor at current position on framebuffer

// Handle a single mouse event (dx, dy, buttons) and redraw cursor; safe from IRQ.
void cursor_handle_event(const mouse_event_t* ev);

#endif
