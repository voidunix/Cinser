/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: desktop.c
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
#include <stddef.h>

#include "video.h"
#include "window.h"
#include "programs/shell.h"

static Window* g_focused = NULL;

// Variável de controle (estática = mantém valor entre chamadas)
static int g_bg_painted = 0;

void __desktop_set_focused(Window* w) { g_focused = w; }

static void fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (g_video_driver && g_video_driver->fill_rect) {
        g_video_driver->fill_rect(x, y, w, h, color);
    } else {
        draw_rect(x, y, w, h, color);
    }
}

void desktop_init(void) {
    shell_open();
    g_bg_painted = 0; // Reseta no boot
}

void desktop_draw(void) {
    // 1. Pinta fundo AZUL (Só na primeira vez!)
    if (!g_bg_painted) {
        int W = 1024;
        int H = 768;
        if (g_video_driver && g_video_driver->width > 0)  W = g_video_driver->width;
        if (g_video_driver && g_video_driver->height > 0) H = g_video_driver->height;

        fill_rect(0, 0, W, H, 0x00204A87);
        g_bg_painted = 1; // Trava para não pintar mais
    }

    // 2. Desenha janelas (Shell)
    window_draw_all();
}

void desktop_key(char c) {
    if (c == '\t') { window_focus(NULL); return; }
    if (g_focused) window_key(g_focused, c);
    else window_focus(NULL);
}