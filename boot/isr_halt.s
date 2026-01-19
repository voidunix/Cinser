BITS 32
GLOBAL isr_halt

; Handler universal: desliga interrupcoes e para.
; (Sem iret de proposito: se cair aqui, queremos congelar pra debug.)
isr_halt:
    cli
.hang:
    hlt
    jmp .hang
