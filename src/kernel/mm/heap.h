#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

void heap_init();
void* kmalloc(size_t size);
void kfree(void* ptr);
void heap_dump(void);
void heap_validate(void);

#endif
