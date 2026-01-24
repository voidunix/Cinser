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
GLOBAL isr_halt

; Handler universal: desliga interrupcoes e para.
; (Sem iret de proposito: se cair aqui, queremos congelar pra debug.)
isr_halt:
    cli
.hang:
    hlt
    jmp .hang
