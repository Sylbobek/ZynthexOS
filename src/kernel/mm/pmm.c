#include "mm/pmm.h"
#include "core/multiboot.h"

#include <stddef.h>
#include "lib/string.h"

#define FRAME_SIZE 4096

#define MISSING_MEMORY_MAP 1
#define OUT_OF_PM          2

static uint32_t *bitmap;
static uint32_t total_frames;

extern uint32_t kernel_start;
extern uint32_t kernel_end;

// Bitmap Helpers
static void set_frame(uint32_t frame)
{
    bitmap[frame / 32] |= (1 << (frame % 32));
}

static void clear_frame(uint32_t frame)
{
    bitmap[frame / 32] &= ~(1 << (frame % 32));
}

static int first_free_frame(void)
{
    for (uint32_t i = 0; i < total_frames / 32; i++)
    {
        if (bitmap[i] != 0xFFFFFFFF)
        {
            for (int j = 0; j < 32; j++)
            {
                if (!(bitmap[i] & (1 << j)))
                    return i * 32 + j;
            }
        }
    }

    return -1;
}

int pmm_init(uint32_t multiboot_addr)
{
    multiboot_info_t* mbi = (multiboot_info_t*)multiboot_addr;

    if (!(mbi->flags & (1 << 6)))
        return MISSING_MEMORY_MAP;

    multiboot_mmap_entry_t* mmap =
        (multiboot_mmap_entry_t*)mbi->mmap_addr;

    // Find highest address
    uint32_t highest_addr = 0;

    while ((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length)
    {
        uint32_t region_end = mmap->addr + mmap->len;

        if (region_end > highest_addr)
            highest_addr = region_end;

        mmap = (multiboot_mmap_entry_t*)
            ((uint32_t)mmap + mmap->size + sizeof(uint32_t));
    }

    total_frames = highest_addr / FRAME_SIZE;

    // Allocate bitmap after kernel
    uint32_t kernel_end_addr = (uint32_t)&kernel_end;

    bitmap = (uint32_t*)kernel_end_addr;

    uint32_t bitmap_entries = (total_frames + 31) / 32;
    uint32_t bitmap_size = bitmap_entries * sizeof(uint32_t);


    // Round up bitmap end to 4K 
    uint32_t bitmap_end =
        (kernel_end_addr + bitmap_size + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);

    // Mark all frames used initially
    memset(bitmap, 0xFF, bitmap_size);


    // Free usable memory
    mmap = (multiboot_mmap_entry_t*)mbi->mmap_addr;

    while ((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length)
    {
        if (mmap->type == 1) // usable RAM
        {
            uint32_t start_frame = mmap->addr / FRAME_SIZE;
            uint32_t end_frame   = (mmap->addr + mmap->len) / FRAME_SIZE;

            for (uint32_t f = start_frame; f < end_frame; f++)
            {
                if (f < total_frames)
                    clear_frame(f);
            }
        }

        mmap = (multiboot_mmap_entry_t*)
            ((uint32_t)mmap + mmap->size + sizeof(uint32_t));
    }


    // Reserve kernel frames 
    uint32_t kernel_start_addr = (uint32_t)&kernel_start;

    uint32_t kernel_start_frame = kernel_start_addr / FRAME_SIZE;
    uint32_t kernel_end_frame   =
        (kernel_end_addr + FRAME_SIZE - 1) / FRAME_SIZE;

    for (uint32_t f = kernel_start_frame; f < kernel_end_frame; f++)
        set_frame(f);


    // Reserve bitmap frames  
    uint32_t bitmap_start_frame = kernel_end_addr / FRAME_SIZE;
    uint32_t bitmap_end_frame   = bitmap_end / FRAME_SIZE;

    for (uint32_t f = bitmap_start_frame; f < bitmap_end_frame; f++)
        set_frame(f);

    return 0;
}


// Allocation
void* pmm_alloc_frame(void)
{
    int frame = first_free_frame();

    if (frame == -1)
        return NULL;

    set_frame(frame);
    return (void*)(frame * FRAME_SIZE);
}

void pmm_free_frame(void* frame)
{
    uint32_t frame_number = (uint32_t)frame / FRAME_SIZE;
    clear_frame(frame_number);
}

uint32_t pmm_get_total_frames(void)
{
    return total_frames;
}

uint32_t pmm_get_free_frames(void)
{
    uint32_t free = 0;

    for (uint32_t i = 0; i < total_frames; i++)
    {
        uint32_t index = i / 32;
        uint32_t bit   = i % 32;

        if (!(bitmap[index] & (1 << bit)))
            free++;
    }

    return free;
}
