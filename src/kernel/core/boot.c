#include "boot.h"

#include "core/idt.h"
#include "core/debug.h"
#include "core/console.h"
#include "core/multiboot.h"
#include "core/status.h"
#include "core/log.h"
#include "time.h"

#include "video/framebuffer.h"
#include "sched/sched.h"
#include "sys/syscall.h"

#include "drivers/pic.h"
#include "drivers/pit.h"
#include "drivers/keyboard.h"

#include "mm/heap.h"
#include "mm/paging.h"
#include "mm/pmm.h"

#include "include/system.h"

#include "video/vga.h"

#include "fs/fd/fd.h"
#include "fs/vfs/vfs.h"
#include "fs/vfs/path.h"
#include "fs/ramfs/ramfs.h"

#include "arch/x86/gdt.h"
#include "arch/x86/tss.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static size_t get_logo_width(const char* logo)
{
    size_t max = 0;
    size_t current = 0;

    for (size_t i = 0; logo[i]; i++)
    {
        if (logo[i] == '\n')
        {
            if (current > max)
                max = current;

            current = 0;
        }
        else
        {
            current++;
        }
    }

    if (current > max)
        max = current;

    return max;
}

static size_t get_logo_height(const char* logo)
{
    size_t lines = 1;

    for (size_t i = 0; logo[i]; i++)
        if (logo[i] == '\n')
            lines++;

    return lines;
}

void boot_screen(void)
{
    vga_clear();
    vga_set_color(0x1F);


    size_t logo_width  = get_logo_width(boot_logo);
    size_t logo_height = get_logo_height(boot_logo);

    size_t start_row = (VGA_HEIGHT - logo_height) / 2;
    size_t start_col = (VGA_WIDTH - logo_width) / 2;

    size_t row = start_row;
    size_t col = start_col;

    vga_set_color(BOOT_LOGO_COLOR);

    vga_set_cursor(row, col);


    for (size_t i = 0; boot_logo[i]; i++)
    {
        if (boot_logo[i] == '\n')
        {
            row++;
            col = start_col;
            vga_set_cursor(row, col);
        }
        else
        {
            vga_putchar(boot_logo[i]);
            col++;
        }
    }

    vga_set_color(0x07);
}

void kernel_boot(uint32_t multiboot_magic, uint32_t multiboot_addr)
{
    vga_clear();
    log_init();
    log_write("boot: kernel_boot start\n");
    multiboot_print_memory(multiboot_addr);                

    framebuffer_init(multiboot_addr);


    status_begin("Initializing PMM");

    int pmm_result = pmm_init(multiboot_addr);

    if (pmm_result == 0)
    {
        status_end_ok();
    }
    else
    {
        status_end_error();

        if (pmm_result == 1) // Missing memory map
            debug_panic("PMM failed: Missing memory map");

        if (pmm_result == 2) // Out of physical Memory
            debug_panic("PMM failed: Out of physical memory");

        debug_panic("PMM failed: Unknown error");
    }


    status_begin("Initializing GDT");
    gdt_init();
    status_end_ok();

    status_begin("Initializing TSS");
    tss_init();
    status_end_ok();

    status_begin("Initializing IDT");
    idt_init();
    // Set syscall vector 0x80
    extern void syscall_entry(void);
    idt_set_gate(0x80, (uint32_t)syscall_entry, 0x08, 0x8E);
    status_end_ok();

    status_begin("Remapping PIC");
    pic_remap();
    status_end_ok();

    status_begin("Setting hardware time frequency to 100Hz");
    pit_init(100);
    status_end_ok();

    
    status_begin("Initializing Scheduler");
    scheduler_init();
    status_end_ok();

    status_begin("Enabling Interrupts");
    asm volatile("sti");
    status_end_ok();

    status_begin("Initializing Paging");
    paging_init();
    status_end_ok();

    status_begin("Initializing Heap");
    heap_init();
    status_end_ok();

    status_begin("Initializing VFS");
    vfs_init();
    status_end_ok();

    status_begin("Initializing FD");
    fd_init();
    status_end_ok();

    status_begin("Initializing Root Filesystem");
    // Create main root filesystem (RAM for now)
    vfs_node_t* main_root = ramfs_create_instance();
    vfs_root = main_root;
    status_end_ok();

    status_begin("Mounting /tmp as RAMFS");
    vfs_root->ops->create(vfs_root, "tmp", VFS_DIR);
    vfs_node_t* tmp_ramfs = ramfs_create_instance();
    vfs_mount("/tmp", tmp_ramfs);
    status_end_ok();


    status_begin("Initializing Console");
    console_init();
    status_end_ok();

    status_begin("Initializing Keyboard");
    keyboard_init();
    status_end_ok();

    status_begin("Booting system");
    status_loading_animation(823);
    status_end_ok();

    sleep(1700);

    vga_clear();
    boot_screen();
    sleep(2200);
    vga_clear();
    vga_enable_cursor(14, 15);   // classic underline
    vga_print_color(boot_logo, BOOT_LOGO_COLOR);

}

