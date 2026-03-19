#ifndef TIME_H
#define TIME_H

#include <stdint.h>

void timer_tick();           // called from IRQ0
uint32_t timer_get_ticks();  // read-only
uint32_t timer_get_seconds(); // read-only
void sleep(uint32_t ms);

#endif
