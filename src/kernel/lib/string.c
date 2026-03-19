#include "lib/string.h"
#include "mm/heap.h"

size_t strlen(const char* str)
{
    size_t len = 0;

    while (str[len] != '\0')
        len++;

    return len;
}

int strcmp(const char* a, const char* b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return *a - *b;

        a++;
        b++;
    }

    return *a - *b;
}

int strncmp(const char* a, const char* b, size_t n)
{
    while (n && *a && *b)
    {
        if (*a != *b)
            return *a - *b;

        a++;
        b++;
        n--;
    }

    if (n == 0)
        return 0;

    return *a - *b;
}

char* strcpy(char* dest, const char* src)
{
    char* original = dest;

    while ((*dest++ = *src++) != '\0')
        ;

    return original;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    char* original = dest;

    while (n && *src)
    {
        *dest++ = *src++;
        n--;
    }

    while (n)
    {
        *dest++ = '\0';
        n--;
    }

    return original;
}

void* memcpy(void* dest, const void* src, size_t n)
{
    unsigned char* d = dest;
    const unsigned char* s = src;

    while (n--)
        *d++ = *s++;

    return dest;
}

void* memset(void* dest, int value, size_t n)
{
    unsigned char* d = dest;

    while (n--)
        *d++ = (unsigned char)value;

    return dest;
}

char* strcat(char* dest, const char* src)
{
    char* original = dest;

    while (*dest != '\0')
        dest++;

    while ((*dest++ = *src++) != '\0')
        ;

    return original;
}

int atoi(const char* str)
{
    int result = 0;

    while (*str)
    {
        if (*str < '0' || *str > '9')
            break;

        result = result * 10 + (*str - '0');
        str++;
    }

    return result;
}

int atoi_hex(const char* str)
{
    int result = 0;

    // Skip optional 0x
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        str += 2;

    while (*str)
    {
        char c = *str;
        int value;

        if (c >= '0' && c <= '9')
            value = c - '0';
        else if (c >= 'a' && c <= 'f')
            value = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            value = c - 'A' + 10;
        else
            break;

        result = result * 16 + value;
        str++;
    }

    return result;
}


char* utoa(uint32_t value, char* buffer, int base)
{
    char* ptr = buffer;
    char* ptr1 = buffer;
    char tmp_char;
    uint32_t tmp_value;

    if (value == 0)
    {
        *ptr++ = '0';
        *ptr = '\0';
        return buffer;
    }

    while (value != 0)
    {
        tmp_value = value % base;

        if (tmp_value < 10)
            *ptr++ = '0' + tmp_value;
        else
            *ptr++ = 'A' + (tmp_value - 10);

        value /= base;
    }

    *ptr-- = '\0';

    // Reverse the string
    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return buffer;
}
