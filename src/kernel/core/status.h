#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>

// void status_ok(const char* message);
// void status_error(const char* message);
// void status_info(const char* message);
void status_end_ok(void);
void status_end_error(void);
void status_begin(const char* message);
void status_loading_animation(uint32_t duration_ms);

#endif
