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
EXTERN kernel_main



_start:
    cli

    ; Salva os registradores do Multiboot antes de mexer em AX
    mov edi, eax            ; edi = magic
    mov esi, ebx            ; esi = multiboot_info ptr

    ; Carrega nossa GDT (code=0x08, data=0x10)
    lgdt [gdt_desc]

    ; Faz far jump pra recarregar CS com o seletor correto
    jmp 0x08:.flush_cs

.flush_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Stack alinhada
    mov esp, stack_top
    and esp, 0xFFFFFFF0
    sub esp, 4              ; faz o alinhamento ficar perfeito dentro do C

    ; Multiboot1 (restaurado dos backups):
    ;   edi = magic
    ;   esi = ponteiro multiboot_info
    push esi                ; arg2: mb_info
    push edi                ; arg1: magic
    call kernel_main

.hang:
    hlt
    jmp .hang

; ---------------- GDT ----------------
align 8
gdt:
    dq 0x0000000000000000       ; null
    dq 0x00CF9A000000FFFF       ; code: base=0, limit=4GB, RX
    dq 0x00CF92000000FFFF       ; data: base=0, limit=4GB, RW

gdt_desc:
    dw (gdt_desc - gdt - 1)
    dd gdt

; ---------------- Stack --------------
SECTION .bss
align 16
stack_bottom:
    resb 16384
stack_top:
