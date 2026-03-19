#include "pit.h"
#include "io.h"

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40

void pit_init(uint32_t frequency) {

    uint32_t divisor = 1193182 / frequency;

    // Command byte:
    // 00 = channel 0
    // 11 = access mode lobyte/hibyte
    // 010 = mode 2 (rate generator)
    // 0 = binary mode

    outb(PIT_COMMAND, 0x36);

    // Send low byte
    outb(PIT_CHANNEL0, divisor & 0xFF);

    // Send high byte
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}
