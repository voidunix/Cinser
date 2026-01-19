/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: vga.c
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

#include "vga.h"


#define VGA_WIDTH  80
#define VGA_HEIGHT 25

static volatile uint16_t *const VGA_MEM = (uint16_t *)0xB8000;

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t vga_color = (VGA_COLOR_LIGHT_GREY | (VGA_COLOR_BLACK << 4));

static inline uint16_t vga_entry(char ch, uint8_t color) {
    return (uint16_t)ch | ((uint16_t)color << 8);
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_color = (uint8_t)fg | ((uint8_t)bg << 4);
}

void vga_clear(void) {
    for (uint16_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEM[y * VGA_WIDTH + x] = vga_entry(' ', vga_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void vga_newline(void) {
    cursor_x = 0;
    if (cursor_y < VGA_HEIGHT - 1) {
        cursor_y++;
    } else {
        /* Sem scroll ainda (deixa travado na ultima linha por enquanto) */
        cursor_y = VGA_HEIGHT - 1;
    }
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_newline();
        return;
    }

    VGA_MEM[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, vga_color);

    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        vga_newline();
    }
}

void vga_write(const char *s) {
    if (!s) return;
    while (*s) vga_putc(*s++);
}

void vga_init(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}
