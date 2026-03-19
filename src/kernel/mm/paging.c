#include "paging.h"
#include "pmm.h"
#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

#define PRESENT   0x1
#define WRITABLE  0x2

static uint32_t page_directory[PAGE_ENTRIES] __attribute__((aligned(4096)));
static uint32_t* page_tables[PAGE_ENTRIES]; // Track tables

static void load_page_directory(uint32_t* pd)
{
    asm volatile("mov %0, %%cr3" :: "r"(pd));
}

static void enable_paging()
{
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void paging_init(void)
{
    // Clear directory
    for (int i = 0; i < PAGE_ENTRIES; i++)
    {
        page_directory[i] = 0;
        page_tables[i] = 0;
    }

    // Identity map first 4MB
    uint32_t* first_table = (uint32_t*)pmm_alloc_frame();

    for (int i = 0; i < PAGE_ENTRIES; i++)
        first_table[i] = 0;

    for (int i = 1; i < PAGE_ENTRIES; i++)
        first_table[i] = (i * PAGE_SIZE) | PRESENT | WRITABLE;
    

    page_directory[0] = ((uint32_t)first_table) | PRESENT | WRITABLE;
    page_tables[0] = first_table;

    load_page_directory(page_directory);
    enable_paging();
}

void map_page(uint32_t virtual_addr, uint32_t physical_addr)
{
    uint32_t dir_index = virtual_addr >> 22;
    uint32_t table_index = (virtual_addr >> 12) & 0x03FF;

    uint32_t* table = page_tables[dir_index];

    // If page table does not exist → allocate
    if (!table)
    {
        table = (uint32_t*)pmm_alloc_frame();

        // Clear table
        for (int i = 0; i < PAGE_ENTRIES; i++)
            table[i] = 0;

        page_directory[dir_index] =
            ((uint32_t)table) | PRESENT | WRITABLE;

        page_tables[dir_index] = table;
    }

    table[table_index] =
        (physical_addr & 0xFFFFF000) | PRESENT | WRITABLE;

    // Flush TLB for that page
    asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
}

