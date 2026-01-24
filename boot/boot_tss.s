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
GLOBAL tss_load

EXTERN tss
EXTERN gdt_write_tss_descriptor

SECTION .text

tss_load:
    ; Fill TSS descriptor (selector 0x18) before loading TR
    mov eax, tss
    mov ecx, 104 - 1
    call gdt_write_tss_descriptor

    mov ax, 0x18
    ltr ax
    ret
