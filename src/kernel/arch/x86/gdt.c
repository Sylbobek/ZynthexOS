#include "gdt.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define GDT_ENTRIES 6

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gp;

extern void gdt_flush(uint32_t);

void gdt_set_entry(int num,
                        uint32_t base,
                        uint32_t limit,
                        uint8_t access,
                        uint8_t gran)
{
    gdt[num].base_low    = base & 0xFFFF;
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = limit & 0xFFFF;
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

void gdt_init(void)
{
    gp.limit = sizeof(struct gdt_entry) * GDT_ENTRIES - 1;
    gp.base  = (uint32_t)&gdt;

    // 0x00 — Null descriptor
    gdt_set_entry(0, 0, 0, 0, 0);

    // 0x08 — Kernel Code Segment
    gdt_set_entry(
        1,
        0,
        0xFFFFFFFF,
        0x9A,   // Present, Ring 0, Code, Executable, Readable
        0xCF    // 4KB granularity, 32-bit
    );

    // 0x10 — Kernel Data Segment
    gdt_set_entry(
        2,
        0,
        0xFFFFFFFF,
        0x92,   // Present, Ring 0, Data, Writable
        0xCF
    );

    // 0x18 — User Code Segment
    gdt_set_entry(
        3,
        0,
        0xFFFFFFFF,
        0xFA,   // Present, Ring 3, Code, Executable, Readable
        0xCF
    );

    // 0x20 — User Data Segment
    gdt_set_entry(
        4,
        0,
        0xFFFFFFFF,
        0xF2,   // Present, Ring 3, Data, Writable
        0xCF
    );


    gdt_flush((uint32_t)&gp);
}
