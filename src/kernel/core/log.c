#include "log.h"

#include "video/vga.h"
#include "lib/string.h"

#define LOG_BUFFER_SIZE 8192

static char log_buffer[LOG_BUFFER_SIZE];
static size_t log_head = 0;
static size_t log_used = 0;

static size_t log_start_index(void)
{
    if (log_used < LOG_BUFFER_SIZE)
        return 0;

    return log_head;
}

void log_init(void)
{
    log_head = 0;
    log_used = 0;
    log_write("log: initialized\n");
}

void log_write(const char* message)
{
    if (!message)
        return;

    for (size_t i = 0; message[i] != '\0'; i++)
    {
        log_buffer[log_head] = message[i];
        log_head = (log_head + 1) % LOG_BUFFER_SIZE;

        if (log_used < LOG_BUFFER_SIZE)
            log_used++;
    }
}

void log_write_hex(uint32_t value)
{
    char buf[11]; // 0x + 8 hex + null
    buf[0] = '0';
    buf[1] = 'x';

    for (int i = 0; i < 8; i++)
    {
        uint8_t nibble = (value >> ((7 - i) * 4)) & 0xF;
        buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
    }

    buf[10] = '\0';
    log_write(buf);
}

void log_write_dec(uint32_t value)
{
    char buf[16];
    utoa(value, buf, 10);
    log_write(buf);
}

void log_dump(void)
{
    size_t start = log_start_index();

    vga_print("[dmesg]\n");

    for (size_t i = 0; i < log_used; i++)
    {
        size_t idx = (start + i) % LOG_BUFFER_SIZE;
        vga_putchar(log_buffer[idx]);
    }

    vga_print("\n[end]\n");
}
