#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

void paging_init();
void map_page(uint32_t virtual_addr, uint32_t physical_addr);

#endif