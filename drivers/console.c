#include "video.h"
#include "font.h"
#include <stdint.h>

// ---------------------------------------------------------------------------
// Runtime font copy
//
// Em alguns caminhos de boot/driver (principalmente com framebuffer/VESA),
// qualquer escrita fora do framebuffer pode acabar corrompendo regioes do kernel
// onde a fonte fica armazenada (tipicamente em .rodata). O sintoma observado foi
// "apenas letras minusculas" aparecerem deslocadas (ex: 'x' virando 'y'),
// enquanto numeros/maiusculas permanecem corretos.
//
// Para deixar o console resiliente (e manter a arquitetura de video modular),
// fazemos uma copia da fonte na inicializacao e renderizamos sempre a partir
// dessa copia.
// ---------------------------------------------------------------------------

static uint8_t g_font_runtime[128][8];
static int g_font_ready = 0;

static void font_runtime_init(void) {
    if (g_font_ready) return;
    for (int i = 0; i < 128; i++) {
        for (int y = 0; y < 8; y++) {
            g_font_runtime[i][y] = font8x8_basic[i][y];
        }
    }
    g_font_ready = 1;
}

// Cores atuais (padrão: Branco no Preto)
static uint32_t fg_color = 0xFFFFFFFF;
static uint32_t bg_color = 0xFF000000;

// Posição do cursor simulado
static int cursor_x = 0;
static int cursor_y = 0;

// Paleta VGA básica mapeada para RGB (para compatibilidade com vga_set_color)
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

// Limpa a tela com a cor de fundo atual
void console_clear() {
    if (!g_video_driver) return;
    g_video_driver->clear_screen(bg_color);
    cursor_x = 0;
    cursor_y = 0;
}

// Configura cores (compatível com o jeito VGA)
void console_set_color(uint8_t fg, uint8_t bg) {
    fg_color = vga_palette[fg & 0xF];
    bg_color = vga_palette[bg & 0xF];
}

// Desenha um caractere na posição atual
void console_putc(char c) {
    if (!g_video_driver) return;

    // Garante que a fonte em runtime esteja pronta (copia da .rodata).
    font_runtime_init();

    // Tratamento de Newline
    if (c == '\n') {
        cursor_x = 0;
        cursor_y += 10; // 8 pixels da fonte + 2 de espaçamento
        return;
    }

    if (c == '\r') {
        cursor_x = 0;
        return;
    }

    // Desenha o caractere pixel por pixel
    // Se o char for fora da tabela, desenha espaço
    int glyph_index = (unsigned char)c;
    if (glyph_index > 127) glyph_index = 32;

    // Garante que a fonte runtime esteja carregada
    if (!g_font_ready) font_runtime_init();
    const uint8_t* glyph = g_font_runtime[glyph_index];

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // O bit mais à esquerda é o bit 7
            // Se bit ativo, desenha Foreground, senão Background
            int px = cursor_x + x;
            int py = cursor_y + y;

            // Evita escrita fora da tela (protege contra corrupção de memória)
            if (px < 0 || py < 0 || px >= g_video_driver->width || py >= g_video_driver->height) {
                continue;
            }

            if ((glyph[y] >> (7 - x)) & 1) {
                put_pixel(px, py, fg_color);
            } else {
                put_pixel(px, py, bg_color);
            }
        }
    }

    // Avança cursor
    cursor_x += 8;
    
    // Wrap-around (quebra de linha automática)
    if (cursor_x >= g_video_driver->width) {
        cursor_x = 0;
        cursor_y += 10;
    }
}

void console_write(const char* str) {
    while (*str) {
        console_putc(*str++);
    }
}

void console_init() {
    font_runtime_init();
    console_clear();
}
