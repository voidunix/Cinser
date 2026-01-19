BITS 32
GLOBAL gdt_load
GLOBAL gdt_write_tss_descriptor
GLOBAL tss
GLOBAL tss_end

SECTION .text

gdt_load:
    lgdt [gdt_descriptor]
    jmp 0x08:.flush
.flush:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

; Build the TSS descriptor at runtime (NASM can't bitshift relocations).
; Inputs:
;   EAX = base address of TSS
;   ECX = limit (size-1)
gdt_write_tss_descriptor:
    mov word [gdt_tss_desc + 0], cx      ; limit 0:15
    mov word [gdt_tss_desc + 2], ax      ; base 0:15
    shr eax, 16
    mov byte [gdt_tss_desc + 4], al      ; base 16:23
    mov byte [gdt_tss_desc + 5], 0x89    ; present=1, DPL=0, type=9 (avail 32-bit TSS)
    mov byte [gdt_tss_desc + 6], 0x00    ; gran=0, limit 16:19=0
    mov byte [gdt_tss_desc + 7], ah      ; base 24:31
    ret

SECTION .data
align 8

gdt_start:
    dq 0x0000000000000000            ; null
    dq 0x00CF9A000000FFFF            ; 0x08: code (base=0, limit=4GB)
    dq 0x00CF92000000FFFF            ; 0x10: data (base=0, limit=4GB)

    ; 0x18: TSS descriptor (filled at runtime by gdt_write_tss_descriptor)
gdt_tss_desc:
    dq 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

SECTION .bss
align 16
; 32-bit TSS is 104 bytes
GLOBAL tss

tss:
    resb 104

tss_end:
