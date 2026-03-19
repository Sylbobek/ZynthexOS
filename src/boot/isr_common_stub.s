global isr_common_stub
extern isr_handler

isr_common_stub:
    pusha

    mov eax, [esp + 32]    ; interrupt number
    mov ebx, [esp + 36]    ; error code

    push ebx               ; error code
    push eax               ; interrupt number

    call isr_handler

    add esp, 8

    popa
    add esp, 8

    iretd

