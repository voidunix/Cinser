; ---------------------------------------------------------------------------
; Tervia Cinser OS - Sistema Operacional
; Desenvolvido por: Tervia Corporation (2026)
; 
; Licença: GNU GPLv3
; Este arquivo faz parte do projeto Tervia Cinser. 
; Você tem a liberdade de estudar, modificar e distribuir este código
; desde que mantenha esta licença original.
; ---------------------------------------------------------------------------
; Descrição: Rotinas de bootloader e inicialização de registradores.
; ---------------------------------------------------------------------------

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
