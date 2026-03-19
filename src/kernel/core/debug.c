#include "debug.h"
#include "video/vga.h"
#include "core/log.h"

static void print_hex16(uint16_t val) {
    char hex[] = "0123456789ABCDEF";
    char buf[7];

    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = hex[(val >> 12) & 0xF];
    buf[3] = hex[(val >> 8) & 0xF];
    buf[4] = hex[(val >> 4) & 0xF];
    buf[5] = hex[val & 0xF];
    buf[6] = '\0';

    vga_print(buf);
}

uint16_t debug_get_cs() {
    uint16_t val;
    asm volatile("mov %%cs, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_ds() {
    uint16_t val;
    asm volatile("mov %%ds, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_es() {
    uint16_t val;
    asm volatile("mov %%es, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_fs() {
    uint16_t val;
    asm volatile("mov %%fs, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_gs() {
    uint16_t val;
    asm volatile("mov %%gs, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_ss() {
    uint16_t val;
    asm volatile("mov %%ss, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_esp()
{
    uint32_t val;
    asm volatile("mov %%esp, %0" : "=r"(val));

    return val;
}


uint32_t debug_get_cr2()
{
    uint32_t val;
    asm volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_cr3()
{
    uint32_t val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_tr()
{
    uint16_t val;
    asm volatile("str %0" : "=r"(val));
    return val;
}

void debug_get_gdtr(uint32_t* base, uint16_t* limit)
{
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) gdtr;

    asm volatile("sgdt %0" : "=m"(gdtr));

    *base = gdtr.base;
    *limit = gdtr.limit;
}

void debug_get_idtr(uint32_t* base, uint16_t* limit)
{
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) idtr;

    asm volatile("sidt %0" : "=m"(idtr));

    *base = idtr.base;
    *limit = idtr.limit;
}

uint32_t debug_get_cr0()
{
    uint32_t val;
    asm volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_cr4()
{
    uint32_t val;
    asm volatile("mov %%cr4, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_eflags()
{
    uint32_t val;

    asm volatile(
        "pushf\n\t"
        "pop %0"
        : "=r"(val)
    );

    return val;
}



void debug_print_segments()
{
    vga_print("==== CPU STATE ====\n");

    uint16_t cs = debug_get_cs();
    uint16_t ds = debug_get_ds();
    uint16_t es = debug_get_es();
    uint16_t fs = debug_get_fs();
    uint16_t gs = debug_get_gs();
    uint16_t ss = debug_get_ss();
    uint16_t tr = debug_get_tr();

    uint32_t cr0 = debug_get_cr0();
    uint32_t cr2 = debug_get_cr2();
    uint32_t cr3 = debug_get_cr3();
    uint32_t cr4 = debug_get_cr4();
    uint32_t eflags = debug_get_eflags();
    uint32_t esp = debug_get_esp();

    uint32_t gdtr_base, idtr_base;
    uint16_t gdtr_limit, idtr_limit;

    debug_get_gdtr(&gdtr_base, &gdtr_limit);
    debug_get_idtr(&idtr_base, &idtr_limit);

    // Segments
    vga_print("CS: "); print_hex16(cs);
    vga_print(" DS: "); print_hex16(ds);
    vga_print(" ES: "); print_hex16(es);
    vga_print(" FS: "); print_hex16(fs);
    vga_print(" GS: "); print_hex16(gs);
    vga_print(" SS: "); print_hex16(ss);
    vga_print(" TR: "); print_hex16(tr);
    vga_print("\n");

    // Stack + Flags
    vga_print("ESP: "); vga_print_hex(esp);
    vga_print(" EFLAGS: "); vga_print_hex(eflags);
    vga_print(" CPL: "); vga_print_dec(cs & 0x3);
    vga_print("\n");

    // Control Registers
    vga_print("CR0: "); vga_print_hex(cr0);
    vga_print(" CR2: "); vga_print_hex(cr2);
    vga_print(" CR3: "); vga_print_hex(cr3);
    vga_print(" CR4: "); vga_print_hex(cr4);
    vga_print("\n");

    // Descriptor Tables
    vga_print("GDTR: "); vga_print_hex(gdtr_base);
    vga_print("/"); vga_print_hex(gdtr_limit);

    vga_print(" IDTR: "); vga_print_hex(idtr_base);
    vga_print("/"); vga_print_hex(idtr_limit);
    vga_print("\n");

    // Mode flags (super compact)
    vga_print("PM:");
    vga_print((cr0 & 0x1) ? "1 " : "0 ");

    vga_print("PG:");
    vga_print((cr0 & 0x80000000) ? "1" : "0");

    vga_print("\n====================\n");
}





void debug_panic(const char* message)
{
    asm volatile("cli");   // Disable interrupts

    log_write("panic: ");
    if (message)
        log_write(message);
    log_write("\n");

    vga_set_color(0x4F);   // White on red 
    vga_clear();

    vga_print("================================================================================\n");
    vga_set_color(0xCF); // white on red + blinking (or bright bg on QEMU)
    vga_print("                                 KERNEL PANIC                                   \n");
    vga_set_color(0x4F);
    vga_print("================================================================================\n\n");

    vga_set_color(0x4F); 

    vga_print("Reason: ");
    vga_print(message);
    vga_print("\n\n");

    vga_print("CPU Segment Registers:\n");
    debug_print_segments();
    vga_print("\n");

    vga_print("System Halted.\n");

    while (1)
        asm volatile("hlt");
}