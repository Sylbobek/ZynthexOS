#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

void debug_print_segments();
uint16_t debug_get_cs();
uint16_t debug_get_ds();
uint16_t debug_get_es();
uint16_t debug_get_fs();
uint16_t debug_get_gs();
uint16_t debug_get_ss();

void debug_panic(const char* message);

#endif

// CS: 0x10
// DS: 0x18
// ES: 0x18
// FS: 0x18
// GS: 0x18
// SS: 0x18