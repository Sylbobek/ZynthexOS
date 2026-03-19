global syscall_entry

extern syscall_handler

; int 0x80 entry point
; Saves registers and calls C handler with args in eax, ebx, ecx, edx
syscall_entry:
    pusha               ; Save all general registers

    ; Pass args to C function
    push edx            ; arg4
    push ecx            ; arg3
    push ebx            ; arg2
    push eax            ; arg1

    call syscall_handler
    add esp, 16         ; Clean up stack

    popa                ; Restore registers
    iret                ; Return from interrupt
