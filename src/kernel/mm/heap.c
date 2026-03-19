#include "heap.h"
#include "mm/vm.h"
#include "video/vga.h"

extern uint32_t kernel_end;   // defined in linker

static uint32_t heap_start;
static uint32_t heap_end;

#define HEAP_SIZE 1024 * 1024  // 1MB heap for now
#define MIN_BLOCK_SIZE 8
#define MAX_HEAP_ALLOWED (KERNEL_HEAP_END - KERNEL_HEAP_START)
#define HEAP_MAGIC 0xDEADBEEF

typedef struct block_header
{
    uint32_t magic;
    size_t size;
    int free;
    struct block_header* next;
    struct block_header* prev;
} block_header_t;


static block_header_t* heap_head = NULL;

void heap_init()
{
    heap_start   = KERNEL_HEAP_START;
    heap_end     = KERNEL_HEAP_START;
}


void* kmalloc(size_t size)
{
    if (size == 0)
        return NULL;

    size_t aligned = (size + 7) & ~7;

    if (aligned < size)  // overflow check
        return NULL;

    size = aligned;

    if (size > (KERNEL_HEAP_END - heap_end - sizeof(block_header_t) - sizeof(uint32_t)))
        return NULL;



    block_header_t* current = heap_head;

    // First allocation
    if (!heap_head)
    {
        size_t new_end = heap_end + sizeof(block_header_t) + size;

        if (new_end >= KERNEL_HEAP_END)
            return NULL;

        heap_head = (block_header_t*)heap_end;
        heap_head->magic = HEAP_MAGIC;

        heap_head->size = size;
        heap_head->free = 0;
        heap_head->next = NULL;
        heap_head->prev = NULL;

        uint32_t* tail = (uint32_t*)((char*)(heap_head + 1) + size);
        *tail = HEAP_MAGIC;


        heap_end = new_end;

        return (void*)(heap_head + 1);
    }


    // Search free block
    while (current)
    {
        if (current->free && current->size >= size)
        {
            // Split if large enough
            if (current->size >= size + sizeof(block_header_t) + sizeof(uint32_t) + MIN_BLOCK_SIZE)
            {
                block_header_t* new_block =
                    (block_header_t*)((char*)(current + 1) + size);

                new_block->magic = HEAP_MAGIC;

                new_block->size = current->size - size - sizeof(block_header_t) - sizeof(uint32_t);
                new_block->free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                uint32_t* new_tail = (uint32_t*)((char*)(new_block + 1) + new_block->size);
                *new_tail = HEAP_MAGIC;



                if (new_block->next)
                    new_block->next->prev = new_block;

                current->size = size;
                current->next = new_block;
                uint32_t* tail = (uint32_t*)((char*)(current + 1) + size);
                *tail = HEAP_MAGIC;

            }

            current->free = 0;
            return (void*)(current + 1);
        }

        if (!current->next)
            break;

        current = current->next;
    }

    // Append new block
    size_t new_end = heap_end + sizeof(block_header_t) + size + sizeof(uint32_t);

    if (new_end < heap_end)
        return NULL;

    if (new_end >= KERNEL_HEAP_END)
        return NULL;

    block_header_t* new_block = (block_header_t*)heap_end;
    new_block->magic = HEAP_MAGIC;

    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL;
    new_block->prev = current;
    uint32_t* new_tail = (uint32_t*)((char*)(new_block + 1) + new_block->size);
    *new_tail = HEAP_MAGIC;

    current->next = new_block;

    heap_end = new_end;

    

    return (void*)(new_block + 1);

}


void kfree(void* ptr)
{
    
    if (!ptr)
        return;


    block_header_t* block = (block_header_t*)ptr - 1;

    if ((uint32_t)block < heap_start ||
        (uint32_t)block >= heap_end)
    {
        vga_print("Invalid free header!\n");
        return;
    }

    if (block->magic != HEAP_MAGIC)
    {
        vga_print("Heap header corrupted!\n");
        return;
    }

    uint32_t* tail = (uint32_t*)((char*)(block + 1) + block->size);

    if (*tail != HEAP_MAGIC)
    {
        vga_print("Heap tail corruption detected!\n");
        return; 
    }


    if (block->size == 0 ||
    (uint32_t)(block + 1) + block->size > heap_end)
    {
        vga_print("Heap corruption detected!\n");
        return;
    }



    if (block->free)
    {
        vga_print("Double free detected!\n");
        return;
    }

    block->free = 1;
    //heap_validate();

    // Merge with next
    if (block->next &&
        (uint32_t)block->next >= heap_start &&
        (uint32_t)block->next < heap_end &&
        block->next->free)
    {
        
        block->size += sizeof(block_header_t) + block->next->size + sizeof(uint32_t);
        block->next = block->next->next;

        uint32_t* tail = (uint32_t*)((char*)(block + 1) + block->size);
        *tail = HEAP_MAGIC;


        if (block->next)
            block->next->prev = block;
    }

    // Merge with previous
    if (block->prev &&
    (uint32_t)block->prev >= heap_start &&
    (uint32_t)block->prev < heap_end &&
    block->prev->free)
    {
        block->prev->size += sizeof(block_header_t) + block->size + sizeof(uint32_t);
        uint32_t* tail = (uint32_t*)((char*)(block->prev + 1) + block->prev->size);
        *tail = HEAP_MAGIC;
        block->prev->next = block->next;

        if (block->next)
            block->next->prev = block->prev;

    }

    if (heap_head && heap_head->free && heap_head->next == NULL)
    {
        heap_end = heap_start;
        heap_head = NULL;
    }
    else
    {
        block_header_t* last = heap_head;

        while (last && last->next)
            last = last->next;

        if (last && last->free)
            heap_end = (uint32_t)last;
    }


}




void heap_dump(void)
{
    block_header_t* current = heap_head;
    uint32_t index = 0;

    if (!current)
    {
        vga_print("Heap empty.\n");
        return;
    }

    vga_print("Heap blocks:\n");

    while (current)
    {
        vga_print("  Block ");
        vga_print_dec(index);
        vga_print(": addr=");
        vga_print_hex((uint32_t)current);

        vga_print(" size=");
        vga_print_dec(current->size);

        vga_print(" header=");
        vga_print_hex((uint32_t)current);
        vga_print(" payload=");
        vga_print_hex((uint32_t)(current + 1));


        vga_print(" ");
        if (current->free)
            vga_print("FREE");
        else
            vga_print("USED");

        vga_print("\n");

        current = current->next;
        index++;
    }
}

void heap_validate(void)
{

    block_header_t* slow = heap_head;
    block_header_t* fast = heap_head;

    while (fast && fast->next)
    {
        slow = slow->next;
        fast = fast->next->next;

        if (slow == fast)
        {
            vga_print("Heap cycle detected!\n");
            return;
        }
    }
    block_header_t* current = heap_head;

    while (current)
    {
        if (current->magic != HEAP_MAGIC)
        {
            vga_print("Heap magic corrupted!\n");
            return;
        }
        if (current->next && current->next->prev != current)
        {
            vga_print("Heap corruption detected!\n");
            return;
        }
        if ((uint32_t)current < heap_start ||
            (uint32_t)current >= heap_end)
        {
            vga_print("Block outside heap range!\n");
            return;
        }

        if ((uint32_t)(current + 1) + current->size > heap_end)
        {
            vga_print("Block exceeds heap boundary!\n");
            return;
        }


        current = current->next;
    }

    vga_print("Heap structure valid.\n");
}
