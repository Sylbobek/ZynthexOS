#include "cpuid.h"
#include <stdint.h>

static void cpuid(uint32_t eax_in,
                  uint32_t* eax,
                  uint32_t* ebx,
                  uint32_t* ecx,
                  uint32_t* edx)
{
    asm volatile("cpuid"
                 : "=a"(*eax), "=b"(*ebx),
                   "=c"(*ecx), "=d"(*edx)
                 : "a"(eax_in));
}

void cpu_get_brand(char* buffer)
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t* ptr = (uint32_t*)buffer;

    for (uint32_t i = 0; i < 3; i++)
    {
        cpuid(0x80000002 + i, &eax, &ebx, &ecx, &edx);

        *ptr++ = eax;
        *ptr++ = ebx;
        *ptr++ = ecx;
        *ptr++ = edx;
    }

    buffer[48] = '\0';
}
