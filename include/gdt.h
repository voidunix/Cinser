/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: gdt.h
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

#pragma once

/*
 * GDT + TSS
 * - GDT is defined/loaded in ASM (boot/boot_gdt.s)
 * - TR (TSS selector) is loaded in ASM (boot/boot_tss.s)
 * - We only expose a helper to set esp0/ss0 from C.
 */

#include <stdint.h>

void tss_set_kernel_stack(uint32_t stack_top);
