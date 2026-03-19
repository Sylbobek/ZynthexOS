#include "tss.h"
#include <stdint.h>

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

static struct tss_entry tss;

extern void gdt_set_entry(int, uint32_t, uint32_t, uint8_t, uint8_t);
extern void tss_flush(void);

void tss_init(void)
{
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(struct tss_entry);

    gdt_set_entry(
        5,
        base,
        limit,
        0x89,   // Present, Ring 0, TSS (not busy)
        0x40
    );

    tss.ss0 = 0x10;   // Kernel data segment
    tss.esp0 = 0;     // Will set later when scheduler exists
    tss.iomap_base = sizeof(struct tss_entry);

    tss_flush();
}

void tss_set_kernel_stack(uint32_t esp0)
{
    tss.esp0 = esp0;
}
