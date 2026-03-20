#include "mouse.h"
#include "io.h"
#include "pic.h"
#include <stddef.h>

#define MOUSE_DATA_PORT   0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_CMD_PORT    0x64

#define MOUSE_BUF_SIZE 32

static mouse_event_t mouse_buffer[MOUSE_BUF_SIZE];
static uint32_t buf_head = 0;
static uint32_t buf_tail = 0;

static uint8_t packet[3];
static uint8_t packet_index = 0;

static inline int buffer_empty(void) { return buf_head == buf_tail; }
static inline int buffer_full(void) { return ((buf_head + 1) % MOUSE_BUF_SIZE) == buf_tail; }

static void buffer_push(const mouse_event_t* ev)
{
    if (buffer_full())
        return;
    mouse_buffer[buf_head] = *ev;
    buf_head = (buf_head + 1) % MOUSE_BUF_SIZE;
}

int mouse_has_event(void)
{
    return !buffer_empty();
}

int mouse_read_event(mouse_event_t* out)
{
    if (buffer_empty() || out == NULL)
        return 0;
    *out = mouse_buffer[buf_tail];
    buf_tail = (buf_tail + 1) % MOUSE_BUF_SIZE;
    return 1;
}

static void mouse_wait_input(void)
{
    // wait for bit1 to clear
    for (int i = 0; i < 100000; i++)
        if ((inb(MOUSE_STATUS_PORT) & 2) == 0)
            return;
}

static void mouse_wait_output(void)
{
    // wait for bit0 to set
    for (int i = 0; i < 100000; i++)
        if (inb(MOUSE_STATUS_PORT) & 1)
            return;
}

static void mouse_write(uint8_t val)
{
    mouse_wait_input();
    outb(MOUSE_CMD_PORT, 0xD4); // tell controller we send to mouse
    mouse_wait_input();
    outb(MOUSE_DATA_PORT, val);
}

static uint8_t mouse_read(void)
{
    mouse_wait_output();
    return inb(MOUSE_DATA_PORT);
}

static void mouse_set_defaults(void)
{
    mouse_write(0xF6); // set defaults
    mouse_read();
    mouse_write(0xF4); // enable data reporting
    mouse_read();
}

void mouse_init(void)
{
    // Enable auxiliary device
    mouse_wait_input();
    outb(MOUSE_CMD_PORT, 0xA8);

    // Enable interrupts on PS/2 controller
    mouse_wait_input();
    outb(MOUSE_CMD_PORT, 0x20); // read Command Byte
    mouse_wait_output();
    uint8_t status = inb(MOUSE_DATA_PORT);
    status |= 2;   // enable IRQ12
    status &= ~0x20; // disable mouse clock if set
    mouse_wait_input();
    outb(MOUSE_CMD_PORT, 0x60);
    mouse_wait_input();
    outb(MOUSE_DATA_PORT, status);

    // Reset buffer
    buf_head = buf_tail = 0;
    packet_index = 0;

    mouse_set_defaults();

    // Unmask IRQ12
    pic_send_eoi(12);
}

void mouse_irq_handler(void)
{
    uint8_t status = inb(MOUSE_STATUS_PORT);
    if ((status & 0x20) == 0)
    {
        pic_send_eoi(12);
        return; // not mouse data
    }

    uint8_t data = inb(MOUSE_DATA_PORT);
    packet[packet_index++] = data;

    if (packet_index < 3)
    {
        pic_send_eoi(12);
        return;
    }

    packet_index = 0;

    // First byte bits: [7]Yovf [6]Xovf [5]Ysign [4]Xsign [3]Always1 [2]M [1]R [0]L
    int dx = (int8_t)packet[1];
    int dy = (int8_t)packet[2];

    mouse_event_t ev = {0};
    ev.dx = dx;
    ev.dy = -dy; // PS/2 y is opposite screen coords
    ev.buttons = packet[0] & 0x07;

    buffer_push(&ev);

    pic_send_eoi(12);
}
