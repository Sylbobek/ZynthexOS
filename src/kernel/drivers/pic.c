#include "pic.h"
#include "io.h"

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2+1)

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

void pic_remap() {
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    // starts the initialization sequence
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // Remap offsets
    outb(PIC1_DATA, 0x20);  // Master PIC → interrupt 32
    outb(PIC2_DATA, 0x28);  // Slave PIC → interrupt 40

    // Tell Master PIC that there is a slave at IRQ2
    outb(PIC1_DATA, 4);

    // Tell Slave PIC its cascade identity
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Restore saved masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8)
        outb(0xA0, 0x20);

    outb(0x20, 0x20);
}
