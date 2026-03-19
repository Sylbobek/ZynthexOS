#include "multiboot.h"
#include "video/vga.h"

void multiboot_print_memory(uint32_t multiboot_addr)
{
    multiboot_info_t* mbi = (multiboot_info_t*)multiboot_addr;

    if (!(mbi->flags & (1 << 6)))
    {
        vga_print("No memory map found\n");
        return;
    }

    vga_print("Memory map:\n");

    multiboot_mmap_entry_t* mmap =
        (multiboot_mmap_entry_t*)mbi->mmap_addr;

    while ((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length)
    {
        vga_print("Region: ");
        vga_print_hex((uint32_t)mmap->addr);
        vga_print(" - ");
        vga_print_hex((uint32_t)(mmap->addr + mmap->len));
        vga_print("\n");

        mmap = (multiboot_mmap_entry_t*)
            ((uint32_t)mmap + mmap->size + sizeof(uint32_t));
    }
}
