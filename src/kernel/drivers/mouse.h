#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

typedef struct {
    int dx;
    int dy;
    int scroll;      // wheel delta (positive = up)
    uint8_t buttons; // bit0 L, bit1 R, bit2 M
} mouse_event_t;

void mouse_init(void);
void mouse_irq_handler(void);

int mouse_has_event(void);
int mouse_read_event(mouse_event_t* out);

#endif
