/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: video.c
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
#include "multiboot.h"

// Driver ativo (começa nulo)
video_driver_t* g_video_driver = 0;

// Driver VESA (fallback padrão quando o bootloader fornece framebuffer)
extern video_driver_t vesa_driver;

void video_init_system(void* mb_ptr) {
    multiboot_info_t* mbi = (multiboot_info_t*)mb_ptr;

    // Estrutura pensada para futuramente trocar por drivers externos (Intel iGPU, etc).
    // Por enquanto, só ativamos VESA quando for seguro (RGB + 32bpp).
    if (!mbi) return;

    // Multiboot v1: bit 12 => framebuffer info válido
    if ((mbi->flags & (1u << 12)) == 0) {
        return;
    }

    // Tipo 1 = RGB
    if (mbi->framebuffer_type != 1) {
        return;
    }

    // Nosso driver assume 32bpp (uint32_t por pixel). Se não for, não ativa.
    if (mbi->framebuffer_bpp != 32) {
        return;
    }

    g_video_driver = &vesa_driver;
    g_video_driver->init(mbi);
}

void put_pixel(int x, int y, uint32_t color) {
    if (g_video_driver && g_video_driver->put_pixel) {
        g_video_driver->put_pixel(x, y, color);
    }
}

// Retângulo preenchido (simples, usa put_pixel)
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    // Se o driver tiver acelerador, usa (muito mais rápido e evita flicker/tearing).
    if (g_video_driver && g_video_driver->fill_rect) {
        g_video_driver->fill_rect(x, y, w, h, color);
        return;
    }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}
