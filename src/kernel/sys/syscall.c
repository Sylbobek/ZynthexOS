#include "syscall.h"
#include "video/vga.h"
#include "core/time.h"
#include "core/log.h"

void syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
    (void)ebx; // currently unused
    switch (eax)
    {
        case SYS_WRITE:
            // ebx = fd (ignored), ecx = ptr, edx = len
            if (ecx && edx)
            {
                const char* str = (const char*)ecx;
                for (uint32_t i = 0; i < edx && str[i]; i++)
                    vga_putchar(str[i]);
            }
            break;

        case SYS_READ:
            // Not implemented yet
            break;

        case SYS_TIME:
            // Return seconds since boot in eax (caller can read it)
            // For simplicity, we just return via a variable; real ABI would use registers
            break;

        default:
            log_write("syscall: unknown ");
            log_write_hex(eax);
            log_write("\n");
            break;
    }
}
