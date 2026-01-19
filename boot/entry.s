BITS 32
GLOBAL _start
EXTERN gdt_load
EXTERN kmain

; ------------------------------------------------------------
; Multiboot entrypoint (GRUB already in protected mode)
; ------------------------------------------------------------
_start:
    cli

    ; Load our GDT + reload segments
    call gdt_load

    ; Setup stack (use our own clean stack ASAP)
    mov esp, stack_top
    and esp, 0xFFFFFFF0

    ; Multiboot1: eax=magic, ebx=mb_info
    push ebx
    push eax
    call kmain

.hang:
    hlt
    jmp .hang

SECTION .bss
align 16
GLOBAL kernel_stack_top
stack_bottom:
    resb 16384
stack_top:
kernel_stack_top:
