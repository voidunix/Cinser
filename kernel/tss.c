#include "tss.h"
#include "gdt.h"

// tss is defined in ASM (boot/boot_gdt.s)
extern tss_entry_t tss;

void tss_set_kernel_stack(uint32_t stack_top) {
    // Minimal setup: only ring0 stack fields are important now
    tss.esp0 = stack_top;
    tss.ss0  = 0x10;
    tss.iomap_base = sizeof(tss_entry_t);
}
