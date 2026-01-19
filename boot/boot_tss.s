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
