BITS 32
GLOBAL idt_load

SECTION .text

; void idt_load(uint32_t idt_ptr_addr)
idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret
