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
GLOBAL idt_load

SECTION .text

; void idt_load(uint32_t idt_ptr_addr)
idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret
