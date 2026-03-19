#ifndef LOG_H
#define LOG_H

#include <stddef.h>
#include <stdint.h>

// Initializes kernel ring buffer log. Safe to call once during boot.
void log_init(void);

// Append raw string to the log (no formatting). Accepts NULL-terminated strings.
void log_write(const char* message);

// Convenience helpers to append numeric values.
void log_write_hex(uint32_t value);
void log_write_dec(uint32_t value);

// Dump full log to VGA in chronological order.
void log_dump(void);

#endif
