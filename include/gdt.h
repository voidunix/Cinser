#pragma once

/*
 * GDT + TSS
 * - GDT is defined/loaded in ASM (boot/boot_gdt.s)
 * - TR (TSS selector) is loaded in ASM (boot/boot_tss.s)
 * - We only expose a helper to set esp0/ss0 from C.
 */

#include <stdint.h>

void tss_set_kernel_stack(uint32_t stack_top);
