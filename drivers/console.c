/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: console.c
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

#include "video.h"
#include "font.h"
#include <stdint.h>
#include <stddef.h>

static void console_recompute_dims(void);

// ---------------------------------------------------------------------------
// Console baseado em framebuffer (VESA) com fonte 8x8 + espaçamento vertical.
// Agora com:
//  - buffer de caracteres (para scroll consistente)
//  - scroll quando a tela enche
//  - backspace que apaga na tela (além de mover o cursor)
//  - compatível com qualquer driver de vídeo (usa apenas put_pixel/clear_screen/fill_rect)
// ---------------------------------------------------------------------------

// Runtime font copy (protege contra corrupção de .rodata em certos caminhos)
#define FONT_GLYPHS 129
static uint8_t g_font_runtime[FONT_GLYPHS][8];
static int g_font_ready = 0;

static void font_runtime_init(void) {
    if (g_font_ready) return;
    for (int i = 0; i < FONT_GLYPHS; i++) {
        for (int y = 0; y < 8; y++) {
            g_font_runtime[i][y] = font8x8_basic[i][y];
        }
    }
    g_font_ready = 1;
}

// Cores atuais (padrão: Branco no Preto)
static uint32_t fg_color = 0xFFFFFFFF;
static uint32_t bg_color = 0xFF000000;

// Métricas do "terminal"
#define GLYPH_W 8
#define GLYPH_H 8
#define LINE_H  10   // 8px de glyph + 2px de espaçamento

// Limites estáticos seguros (resoluções comuns: 320..1920)
#define CONSOLE_MAX_COLS 200
#define CONSOLE_MAX_ROWS 120

static int g_cols = 0;
static int g_rows = 0;

// Cursor em células (não em pixels)
static int g_cur_col = 0;
static int g_cur_row = 0;

// Buffer de células (char + cores) para permitir scroll/redraw
typedef struct {
    char ch;
    uint32_t fg;
    uint32_t bg;
} console_cell_t;

static console_cell_t g_cells[CONSOLE_MAX_ROWS][CONSOLE_MAX_COLS];

static inline int min_i(int a, int b) { return (a < b) ? a : b; }

static void console_recalc_geometry(void) {
    if (!g_video_driver) { g_cols = g_rows = 0; return; }
    g_cols = g_video_driver->width / GLYPH_W;
    g_rows = g_video_driver->height / LINE_H;
    g_cols = min_i(g_cols, CONSOLE_MAX_COLS);
    g_rows = min_i(g_rows, CONSOLE_MAX_ROWS);
    if (g_cols < 1) g_cols = 1;
    if (g_rows < 1) g_rows = 1;
    if (g_cur_col >= g_cols) g_cur_col = g_cols - 1;
    if (g_cur_row >= g_rows) g_cur_row = g_rows - 1;
}

static void console_recompute_dims(void) {
    console_recalc_geometry();
}

static void console_clear_cells(void) {
    for (int y = 0; y < CONSOLE_MAX_ROWS; y++) {
        for (int x = 0; x < CONSOLE_MAX_COLS; x++) {
            g_cells[y][x].ch = ' ';
            g_cells[y][x].fg = fg_color;
            g_cells[y][x].bg = bg_color;
        }
    }
}

static void console_fill_bg(void) {
    if (!g_video_driver) return;
    if (g_video_driver->clear_screen) {
        g_video_driver->clear_screen(bg_color);
        return;
    }
    // fallback
    if (g_video_driver->fill_rect) {
        g_video_driver->fill_rect(0, 0, g_video_driver->width, g_video_driver->height, bg_color);
        return;
    }
    draw_rect(0, 0, g_video_driver->width, g_video_driver->height, bg_color);
}

static void draw_glyph_at(int col, int row, char c, uint32_t fg, uint32_t bg) {
    if (!g_video_driver) return;
    font_runtime_init();

    int x0 = col * GLYPH_W;
    int y0 = row * LINE_H;

    int glyph_index = (unsigned char)c;
    if (glyph_index >= FONT_GLYPHS) glyph_index = 32;

    const uint8_t* glyph = g_font_runtime[glyph_index];

    // Desenha o caractere pixel por pixel (8x8), e limpa o fundo na célula
    // Primeiro o fundo do retângulo da célula
    if (g_video_driver->fill_rect) {
        g_video_driver->fill_rect(x0, y0, GLYPH_W, LINE_H, bg_color);
    } else {
        for (int y = 0; y < LINE_H; y++) {
            for (int x = 0; x < GLYPH_W; x++) {
                int px = x0 + x;
                int py = y0 + y;
                if (px < 0 || py < 0 || px >= g_video_driver->width || py >= g_video_driver->height) continue;
                put_pixel(px, py, bg);
            }
        }
    }

    for (int y = 0; y < GLYPH_H; y++) {
        uint8_t rowbits = glyph[y];
        for (int x = 0; x < GLYPH_W; x++) {
            int px = x0 + x;
            int py = y0 + y;
            if (px < 0 || py < 0 || px >= g_video_driver->width || py >= g_video_driver->height) continue;
            if ((rowbits >> (7 - x)) & 1) {
                put_pixel(px, py, fg);
            }
        }
    }
}

static void console_redraw_all(void) {
    if (!g_video_driver) return;
    console_fill_bg();
    for (int y = 0; y < g_rows; y++) {
        for (int x = 0; x < g_cols; x++) {
            draw_glyph_at(x, y, g_cells[y][x].ch, g_cells[y][x].fg, g_cells[y][x].bg);
        }
    }
    if (g_video_driver->update) g_video_driver->update();
}

static void console_scroll_up(void) {
    // Move linhas 1..rows-1 para 0..rows-2
    for (int y = 1; y < g_rows; y++) {
        for (int x = 0; x < g_cols; x++) {
            g_cells[y - 1][x] = g_cells[y][x];
        }
    }
    // Limpa a última linha
    for (int x = 0; x < g_cols; x++) {
        g_cells[g_rows - 1][x].ch = ' ';
        g_cells[g_rows - 1][x].fg = fg_color;
        g_cells[g_rows - 1][x].bg = bg_color;
    }

    // Redesenha (portável, sem precisar de memcpy de framebuffer)
    console_redraw_all();
}

void console_set_color(uint8_t fg, uint8_t bg) {
    // Paleta VGA básica mapeada para RGB
    static uint32_t vga_palette[16] = {
        0xFF000000, // 0 Black
        0xFF0000AA, // 1 Blue
        0xFF00AA00, // 2 Green
        0xFF00AAAA, // 3 Cyan
        0xFFAA0000, // 4 Red
        0xFFAA00AA, // 5 Magenta
        0xFFAA5500, // 6 Brown
        0xFFAAAAAA, // 7 Light Gray
        0xFF555555, // 8 Dark Gray
        0xFF5555FF, // 9 Light Blue
        0xFF55FF55, // 10 Light Green
        0xFF55FFFF, // 11 Light Cyan
        0xFFFF5555, // 12 Light Red
        0xFFFF55FF, // 13 Light Magenta
        0xFFFFFF55, // 14 Yellow
        0xFFFFFFFF  // 15 White
    };

    fg_color = vga_palette[fg & 0x0F];
    bg_color = vga_palette[bg & 0x0F];
}

void console_clear(void) {
    if (!g_video_driver) return;
    font_runtime_init();
    console_recalc_geometry();
    console_clear_cells();
    g_cur_col = 0;
    g_cur_row = 0;
    console_fill_bg();
    if (g_video_driver->update) g_video_driver->update();
}

void console_putc(char c) {
    if (!g_video_driver) return;
    font_runtime_init();
    console_recalc_geometry();

    // Newline
    if (c == '\n') {
        g_cur_col = 0;
        g_cur_row += 1;
        if (g_cur_row >= g_rows) {
            g_cur_row = g_rows - 1;
            console_scroll_up();
        }
        return;
    }

    // Carriage return
    if (c == '\r') {
        g_cur_col = 0;
        return;
    }

    // Backspace: move cursor one character left (não apaga; apagar é responsabilidade do chamador)
    if (c == '\b') {
        if (g_cur_col > 0) {
            g_cur_col--;
        } else if (g_cur_row > 0) {
            g_cur_row--;
            g_cur_col = g_cols - 1;
        }
        return;
    }

    // Bell: ignore
    if (c == '\a') return;

    // Sanitiza char
    unsigned char uc = (unsigned char)c;
    if (uc < 32 || uc > 128) c = ' ';

    // Escreve no buffer + desenha
    g_cells[g_cur_row][g_cur_col].ch = c;
    g_cells[g_cur_row][g_cur_col].fg = fg_color;
    g_cells[g_cur_row][g_cur_col].bg = bg_color;
    draw_glyph_at(g_cur_col, g_cur_row, c, fg_color, bg_color);

    // Avança
    g_cur_col++;
    if (g_cur_col >= g_cols) {
        g_cur_col = 0;
        g_cur_row++;
        if (g_cur_row >= g_rows) {
            g_cur_row = g_rows - 1;
            console_scroll_up();
        }
    }

    if (g_video_driver->update) g_video_driver->update();
}

void console_write(const char* str) {
    while (*str) console_putc(*str++);
}

void console_init(void) {
    font_runtime_init();
    console_recalc_geometry();
    console_clear_cells();
    console_clear();
}

// ---------------------------------------------------------------------------
// Extras: dimensões e cursor (em células de texto)
// ---------------------------------------------------------------------------
int console_get_cols(void) { return g_cols; }
int console_get_rows(void) { return g_rows; }

void console_set_cursor(int col, int row) {
    console_recompute_dims();
    if (col < 0) col = 0;
    if (row < 0) row = 0;
    if (col >= g_cols) col = g_cols - 1;
    if (row >= g_rows) row = g_rows - 1;
    g_cur_col = col;
    g_cur_row = row;
}

void console_get_cursor(int* col, int* row) {
    if (col) *col = g_cur_col;
    if (row) *row = g_cur_row;
}

void print_u64(uint64_t x) {
    char buf[21]; // até 20 dígitos + '\0'
    int i = 0;

    /* Caso especial: zero */
    if (x == 0) {
        console_putc('0');
        return;
    }

    /* Converte número em string (ao contrário) */
    while (x > 0) {
        buf[i++] = '0' + (x % 10);
        x /= 10;
    }

    /* Imprime na ordem correta */
    while (i--)
        console_putc(buf[i]);
}

void print_int(int v) {
    char buf[12];   // -2147483648 até 2147483647 (11 chars + \0)
    int i = 0;

    if (v == 0) {
        console_putc('0');
        return;
    }

    if (v < 0) {
        console_putc('-');
        /* cuidado com INT_MIN: -INT_MIN overflow
           truque: converte pra unsigned */
        uint32_t x = (uint32_t)(-(int64_t)v);
        while (x > 0) {
            buf[i++] = '0' + (x % 10);
            x /= 10;
        }
    } else {
        uint32_t x = (uint32_t)v;
        while (x > 0) {
            buf[i++] = '0' + (x % 10);
            x /= 10;
        }
    }

    while (i--)
        console_putc(buf[i]);
}