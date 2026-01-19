SECTION .multiboot
align 4
    dd 0x1BADB002          ; Multiboot1 magic
    dd 0x00010003          ; flags: align + meminfo
    dd -(0x1BADB002 + 0x00010003) ; checksum
