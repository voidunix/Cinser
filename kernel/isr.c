/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: isr.c
 * Descrição: Núcleo do sistema operacional / Gerenciamento de processos.
 * * Copyright (C) 2026 Tervia Corporation.
 *
 * Este programa é um software livre: você pode redistribuí-lo e/ou 
 * modificá-lo sob os termos da Licença Pública Geral GNU como publicada 
 * pela Free Software Foundation, bem como a versão 3 da Licença.
 *
 * Este programa é distribuído na esperança de que possa ser útil, 
 * mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO 
 * a qualquer MERCADO ou APLICAÇÃO EM PARTICULAR. Veja a 
 * Licença Pública Geral GNU para mais detalhes.
 ****************************************************************************/

#include "isr.h"
#include "console.h"

static const char *exc_names[32] = {
    "Divide-by-zero", "Debug", "NMI", "Breakpoint",
    "Overflow", "Bound Range", "Invalid Opcode", "Device Not Available",
    "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present",
    "Stack-Segment Fault", "General Protection", "Page Fault", "Reserved",
    "x87 Floating-Point", "Alignment Check", "Machine Check", "SIMD Floating-Point",
    "Virtualization", "Control Protection", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Hypervisor Injection", "VMM Communication", "Security", "Reserved"
};

void isr_install(void) {
    // nothing yet; idt_init already wired exceptions
}

void isr_handler(regs_t *r) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_write("\n\n!!! EXCEPTION !!!\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    vga_write("int_no=");
    // print int number in hex (tiny)
    const char *hex = "0123456789ABCDEF";
    vga_write("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t n = (r->int_no >> (i * 4)) & 0xF;
        vga_putc(hex[n]);
    }

    if (r->int_no < 32) {
        vga_write(" (");
        vga_write(exc_names[r->int_no]);
        vga_write(")");
    }

    vga_write("\nerr=");
    vga_write("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t n = (r->err_code >> (i * 4)) & 0xF;
        vga_putc(hex[n]);
    }

    vga_write("\nSystem halted.\n");

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
