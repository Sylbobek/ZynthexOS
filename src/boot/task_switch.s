global task_switch_asm

; void task_switch_asm(task_t* old, task_t* new)
; Switches from old task to new task.
; For now we only support switching from NULL (kernel idle) to a new task.

task_switch_asm:
    ; Arguments: old = ebx, new = edx (cdecl)
    mov ebx, [esp+4]   ; old
    mov edx, [esp+8]   ; new

    ; If old != NULL, save context (EIP, ESP, etc.) — not used yet
    ; For now we assume old == NULL (first switch from idle)

    ; Load new task's stack pointer and jump to its entry point
    mov eax, [edx+8]   ; new->esp
    mov ecx, [edx+12]  ; new->eip

    mov esp, eax
    jmp ecx
