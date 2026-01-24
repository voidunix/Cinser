/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: console.h
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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

// Definições de cores VGA (para não quebrar seu código antigo)
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

// Protótipos
void console_init(void);
void console_clear(void);
void console_set_color(uint8_t fg, uint8_t bg);
void console_putc(char c);
void console_write(const char* str);

// Extras (usado por splash/shell): dimensões e cursor em células de texto
int console_get_cols(void);
int console_get_rows(void);
void console_set_cursor(int col, int row);
void console_get_cursor(int* col, int* row);

// MACROS DE COMPATIBILIDADE
// Isso faz seu código antigo que chama 'vga_write' chamar 'console_write'
#define vga_init()        console_init()
#define vga_write(s)      console_write(s)
#define vga_putc(c)       console_putc(c)
#define vga_set_color(f,b) console_set_color(f,b)

#endif
