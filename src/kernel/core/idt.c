#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/pic.h"
#include "mm/vm.h"
#include "mm/pmm.h"
#include "mm/paging.h"
#include "sched/sched.h"
#include "debug.h"
#include "time.h"

#include "video/vga.h"
#define IDT_ENTRIES 256

static const char* exception_messages[32] = {
    "Divide By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating Point",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point",
    "Virtualization",
    "Control Protection",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection",
    "VMM Communication",
    "Security Exception",
    "Reserved"
};


void isr_handler(uint32_t interrupt_number, uint32_t error_code) 
{
    // CPU EXCEPTIONS (0–31)
    if (interrupt_number < 32)
    {
        if (interrupt_number == 14) // Page Fault
        {
            uint32_t fault_addr;
            asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

            uint32_t aligned_addr = fault_addr & 0xFFFFF000;

            int not_present = !(error_code & 0x1);
            int write       =  (error_code & 0x2);
            int user        =  (error_code & 0x4);
            int reserved    =  (error_code & 0x8);
            int instr_fetch =  (error_code & 0x10);

            if (not_present && !user)
            {
                if (aligned_addr >= KERNEL_HEAP_START &&
                    aligned_addr <  KERNEL_HEAP_END)
                {
                    void* frame = pmm_alloc_frame();

                    if (!frame)
                        debug_panic("Out of physical memory during demand paging");

                    map_page(aligned_addr, (uint32_t)frame);
                    asm volatile("invlpg (%0)" :: "r"(aligned_addr) : "memory");
                    return;
                }
            }

            vga_print("\nPAGE FAULT\n");

            vga_print("Fault Address: ");
            vga_print_hex(fault_addr);
            vga_print("\n");

            vga_print("Error Code: ");
            vga_print_hex(error_code);
            vga_print("\n\n");

            if (not_present)
                vga_print(" - Page not present\n");
            else
                vga_print(" - Protection violation\n");

            if (write)
                vga_print(" - Caused by write\n");
            else
                vga_print(" - Caused by read\n");

            if (user)
                vga_print(" - From user mode\n");
            else
                vga_print(" - From kernel mode\n");

            if (reserved)
                vga_print(" - Reserved bit violation\n");

            if (instr_fetch)
                vga_print(" - Instruction fetch\n");

            vga_print("\n");

            // If not handled, panic
            debug_panic("Unhandled Page Fault");
        }



        vga_print("EXCEPTION: ");
        vga_print(exception_messages[interrupt_number]);
        vga_print("\n");

        debug_panic("CPU Exception");
    }

    // HARDWARE IRQs (32–47)
    if (interrupt_number >= 32 && interrupt_number <= 47)
    {
        if (interrupt_number == 32)
            timer_tick();
            scheduler_tick();

        if (interrupt_number == 33)
        {
            keyboard_irq_handler();
            pic_send_eoi(1);
            return;
        }

        if (interrupt_number == 44) // IRQ12 mouse
        {
            mouse_irq_handler();
            pic_send_eoi(12);
            return;
        }

        if (interrupt_number == 0x80)  // syscall
        {
            // Syscall entry will handle registers and EOI
            extern void syscall_entry(void);
            syscall_entry();
            return;
        }

        pic_send_eoi(interrupt_number - 32);
        return;
    }
}


void isr0_handler() {
    debug_panic("Divide by 0\n");

    while (1) {
        asm volatile("hlt");
    }
}



struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

extern void idt_load(uint32_t);

void idt_set_gate(int n, uint32_t handler) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = 0x08; // code segment
    idt[n].zero = 0;
    idt[n].type_attr = 0x8E; // interrupt gate
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_init() {
    idtp.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idtp.base = (uint32_t)&idt;

    // Clear entire IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }

    #define DECLARE_ISR(n) extern void isr##n();
    DECLARE_ISR(0) DECLARE_ISR(1) DECLARE_ISR(2) DECLARE_ISR(3)
    DECLARE_ISR(4) DECLARE_ISR(5) DECLARE_ISR(6) DECLARE_ISR(7)
    DECLARE_ISR(8) DECLARE_ISR(9) DECLARE_ISR(10) DECLARE_ISR(11)
    DECLARE_ISR(12) DECLARE_ISR(13) DECLARE_ISR(14) DECLARE_ISR(15)
    DECLARE_ISR(16) DECLARE_ISR(17) DECLARE_ISR(18) DECLARE_ISR(19)
    DECLARE_ISR(20) DECLARE_ISR(21) DECLARE_ISR(22) DECLARE_ISR(23)
    DECLARE_ISR(24) DECLARE_ISR(25) DECLARE_ISR(26) DECLARE_ISR(27)
    DECLARE_ISR(28) DECLARE_ISR(29) DECLARE_ISR(30) DECLARE_ISR(31)
    #undef DECLARE_ISR


    #define DECLARE_IRQ(n) extern void irq##n();
    DECLARE_IRQ(0) DECLARE_IRQ(1) DECLARE_IRQ(2) DECLARE_IRQ(3)
    DECLARE_IRQ(4) DECLARE_IRQ(5) DECLARE_IRQ(6) DECLARE_IRQ(7)
    DECLARE_IRQ(8) DECLARE_IRQ(9) DECLARE_IRQ(10) DECLARE_IRQ(11)
    DECLARE_IRQ(12) DECLARE_IRQ(13) DECLARE_IRQ(14) DECLARE_IRQ(15)
    #undef DECLARE_IRQ

    #define IRQ_ENTRY(n) irq##n,
    void (*irq_handlers[16])() = { 
        IRQ_ENTRY(0) IRQ_ENTRY(1) IRQ_ENTRY(2) IRQ_ENTRY(3)
        IRQ_ENTRY(4) IRQ_ENTRY(5) IRQ_ENTRY(6) IRQ_ENTRY(7)
        IRQ_ENTRY(8) IRQ_ENTRY(9) IRQ_ENTRY(10) IRQ_ENTRY(11)
        IRQ_ENTRY(12) IRQ_ENTRY(13) IRQ_ENTRY(14) IRQ_ENTRY(15)
    };
    #undef IRQ_ENTRY

    for (int i = 0; i < 16; i++) {
        idt_set_gate(32 + i, (uint32_t)irq_handlers[i]);
    }


    void (*isr_handlers[32])() = {
        isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
        isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };

    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)isr_handlers[i]);
    }

    idt_load((uint32_t)&idtp);
}

