#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

/*
    Initializes console system.
    Clears buffer.
*/
void console_init(void);

/*
    Processes a single character from input.
    Handles:
        - normal chars
        - backspace
        - newline
*/
void console_handle_char(char c);

/*
    Returns pointer to current input line buffer.
*/
const char* console_get_line(void);

/*
    Clears the current input line.
*/
void console_clear_line(void);

#endif
