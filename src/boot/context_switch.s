global context_switch

; void context_switch(cpu_context_t* old, cpu_context_t* new)

context_switch:
    pusha

    mov eax, [esp + 36]    ; old context pointer
    mov [eax], edi
    mov [eax + 4], esi
    mov [eax + 8], ebp
    mov [eax + 12], esp
    mov [eax + 16], ebx
    mov [eax + 20], edx
    mov [eax + 24], ecx
    mov [eax + 28], eax

    mov eax, [esp + 40]    ; new context pointer

    mov edi, [eax]
    mov esi, [eax + 4]
    mov ebp, [eax + 8]
    mov ebx, [eax + 16]
    mov edx, [eax + 20]
    mov ecx, [eax + 24]
    mov eax, [eax + 28]

    popa
    ret
