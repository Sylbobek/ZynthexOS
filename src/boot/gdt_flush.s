global gdt_flush

gdt_flush:
    mov eax, [esp + 4]   ; Load pointer to gdt_ptr struct
    lgdt [eax]           ; Load the new GDT

    ; Reload data segment registers
    mov ax, 0x10         ; Kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS
    jmp 0x08:.flush      ; 0x08 = Kernel code segment selector

.flush:
    ret
