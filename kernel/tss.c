/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: tss.c
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
