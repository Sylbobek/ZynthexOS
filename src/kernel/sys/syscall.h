#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// Simple syscall numbers (int 0x80)
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_TIME    3

// Entry point for syscall from assembly
void syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

#endif
