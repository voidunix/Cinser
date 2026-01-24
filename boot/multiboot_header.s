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

SECTION .multiboot
align 4
    dd 0x1BADB002          ; Multiboot1 magic
    dd 0x00010003          ; flags: align + meminfo
    dd -(0x1BADB002 + 0x00010003) ; checksum
