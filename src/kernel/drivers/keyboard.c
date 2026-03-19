#include "keyboard.h"
#include "io.h"
#include "pic.h"
#include <stdint.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 256

static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint32_t buffer_head = 0;
static uint32_t buffer_tail = 0;

static const char keymap[128] = {
    0,   27,  '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*', 0,   ' '
};


void keyboard_init(void)
{
    // Nothing required here yet.
}


uint8_t keyboard_read_scancode(void)
{
    return inb(KEYBOARD_DATA_PORT);
}

// TODO: add support for Shift, Caps lock, and all the ASCII charachters
char keyboard_scancode_to_ascii(uint8_t scancode)
{
    if (scancode < 128) 
    {
        return keymap[scancode];
    }

    return 0;
}

void keyboard_irq_handler(void)
{
    uint8_t scancode = keyboard_read_scancode();

    if (scancode & 0x80) {
        pic_send_eoi(1);
        return;
    }

    char c = keyboard_scancode_to_ascii(scancode);

    if (c != 0) {
        keyboard_buffer[buffer_head] = c;
        buffer_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    }

    pic_send_eoi(1);
}




// buffer check
int keyboard_has_input(void)
{
    // head == tail means buffer is empty
    return (buffer_head != buffer_tail);
}


char keyboard_getchar(void)
{
    if (!keyboard_has_input()) 
    {
        return 0;
    }

    char c = keyboard_buffer[buffer_tail];

    // Move tail forward
    buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;

    return c;
}
