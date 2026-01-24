/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: shice_date.c
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

#include <stdint.h>
#include "console.h"
#include "cmos.h"

static void nl(void) { console_putc('\n'); }

static void print_2d(uint8_t v) {
    if (v < 10) console_putc('0');
    print_int((int)v);
}

void print_rtc_date(void) {
    rtc_time_t t;
    cmos_read_rtc(&t);

    print_2d(t.day);
    console_putc('/');
    print_2d(t.mon);
    console_putc('/');
    print_int((int)t.year);
    nl();
}