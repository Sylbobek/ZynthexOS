#ifndef KERNEL_STRING_H
#define KERNEL_STRING_H

#include <stddef.h>
#include <stdint.h>
// Length
size_t strlen(const char* str);

// Compare
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t n);

// Copy
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);

// Memory
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int value, size_t n);

// other
int atoi(const char* str);
int atoi_hex(const char* str);
char* utoa(uint32_t value, char* buffer, int base);

#endif
