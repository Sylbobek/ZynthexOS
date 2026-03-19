global tss_flush

tss_flush:
    mov ax, 0x28      ; TSS selector (index 5 << 3)
    ltr ax            ; Load Task Register
    ret