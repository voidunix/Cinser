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

; Multiboot v1 header
; Flags:
;  bit0 = align modules on page boundaries
;  bit1 = request meminfo
; (Do NOT set bit16 unless you also provide the 5 address fields.)

dd 0x1BADB002          ; magic
dd 0x00000003          ; flags: align + meminfo

dd -(0x1BADB002 + 0x00000003) ; checksum
