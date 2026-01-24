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
#include <stdint.h>
#include <stddef.h>

// =============================
// VESA OTIMIZADO & SEGURO (GCC Fix)
// =============================

static volatile uint32_t* g_vram = NULL;
static uint32_t g_stride = 0; 
static uint32_t* g_back = NULL;

extern video_driver_t vesa_driver;
extern void* kmalloc(uint32_t size);

// Dirty Rect Control
static int g_dirty = 0;
static int g_minx = 0, g_miny = 0, g_maxx = 0, g_maxy = 0;

static inline int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// === ASSEMBLY CORRIGIDO PARA -O2 ===
// O uso de "+D", "+S", "+c" avisa o GCC que esses valores mudam.
// Isso impede que o loop se perca na memória.

static inline void fast_memcpy32(void* dst, const void* src, uint32_t count) {
    int d0, d1, d2;
    __asm__ volatile (
        "cld; rep movsl"
        : "=&D"(d0), "=&S"(d1), "=&c"(d2) // Outputs (clobbered vars)
        : "0"(dst), "1"(src), "2"(count)  // Inputs (ligados aos outputs)
        : "memory", "cc"
    );
}

static inline void fast_memset32(void* dst, uint32_t val, uint32_t count) {
    int d0, d1;
    __asm__ volatile (
        "cld; rep stosl"
        : "=&D"(d0), "=&c"(d1)            // Outputs
        : "0"(dst), "a"(val), "1"(count)  // Inputs (EAX é 'val')
        : "memory", "cc"
    );
}
// ===================================

static void dirty_mark_rect(int x, int y, int w, int h) {
    if (vesa_driver.width <= 0 || vesa_driver.height <= 0) return;
    int x0 = clampi(x, 0, vesa_driver.width - 1);
    int y0 = clampi(y, 0, vesa_driver.height - 1);
    int x1 = clampi(x + w - 1, 0, vesa_driver.width - 1);
    int y1 = clampi(y + h - 1, 0, vesa_driver.height - 1);

    if (!g_dirty) {
        g_dirty = 1;
        g_minx = x0; g_miny = y0; g_maxx = x1; g_maxy = y1;
    } else {
        if (x0 < g_minx) g_minx = x0;
        if (y0 < g_miny) g_miny = y0;
        if (x1 > g_maxx) g_maxx = x1;
        if (y1 > g_maxy) g_maxy = y1;
    }
}

static void vesa_init_impl(void* info);
static void vesa_put_pixel(int x, int y, uint32_t color);
static void vesa_fill_rect(int x, int y, int w, int h, uint32_t color);
static void vesa_clear(uint32_t color);
static void vesa_update(void);

video_driver_t vesa_driver = {
    .driver_name   = "VESA Fixed",
    .width         = 0,
    .height        = 0,
    .bpp           = 32,
    .init          = vesa_init_impl,
    .put_pixel     = vesa_put_pixel,
    .clear_screen  = vesa_clear,
    .update        = vesa_update,
    .fill_rect     = vesa_fill_rect,
};

static void vesa_init_impl(void* info) {
    multiboot_info_t* mbi = (multiboot_info_t*)info;
    if (!mbi || mbi->framebuffer_type != 1 || mbi->framebuffer_bpp != 32) return;

    g_vram = (volatile uint32_t*)(uintptr_t)mbi->framebuffer_addr;
    g_stride = mbi->framebuffer_pitch / 4; // Pitch em bytes -> Stride em pixels

    vesa_driver.width  = (int)mbi->framebuffer_width;
    vesa_driver.height = (int)mbi->framebuffer_height;
    vesa_driver.bpp    = (int)mbi->framebuffer_bpp;

    // Aloca Backbuffer
    uint32_t total_pixels = g_stride * vesa_driver.height;
    g_back = (uint32_t*)kmalloc(total_pixels * 4);
    
    if (g_back) {
        // Limpa backbuffer com AZUL ESCURO para testar visualmente
        fast_memset32(g_back, 0x000000FF, total_pixels); 
    }

    g_dirty = 1;
    g_minx = 0; g_miny = 0;
    g_maxx = vesa_driver.width - 1;
    g_maxy = vesa_driver.height - 1;
    
    // O primeiro update limpa o "lixo" da tela
    vesa_update();
	
    if (!g_back) {
        // SE APARECER UM QUADRADO VERMELHO NO CANTO, É FALTA DE RAM!
        // Força desenho direto na VRAM para avisar erro
        int stride = mbi->framebuffer_pitch / 4;
        volatile uint32_t* vram = (volatile uint32_t*)(uintptr_t)mbi->framebuffer_addr;
        for(int y=0; y<50; y++) {
            for(int x=0; x<50; x++) {
                vram[y*stride + x] = 0xFF0000; // Vermelho
            }
        }
        return;
    }
}

static void vesa_put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= vesa_driver.width || y >= vesa_driver.height) return;
    uint32_t off = (uint32_t)y * g_stride + (uint32_t)x;

    if (g_back) {
        g_back[off] = color;
        dirty_mark_rect(x, y, 1, 1);
    } else if (g_vram) {
        g_vram[off] = color;
    }
}

static void vesa_fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (w <= 0 || h <= 0) return;
    int x0 = clampi(x, 0, vesa_driver.width - 1);
    int y0 = clampi(y, 0, vesa_driver.height - 1);
    int x1 = clampi(x + w - 1, 0, vesa_driver.width - 1);
    int y1 = clampi(y + h - 1, 0, vesa_driver.height - 1);
    int rw = (x1 - x0) + 1;

    // Se temos backbuffer, usamos ele (mais rápido e seguro)
    if (g_back) {
        for (int yy = y0; yy <= y1; yy++) {
            uint32_t* row = g_back + (uint32_t)yy * g_stride + (uint32_t)x0;
            fast_memset32(row, color, rw);
        }
        dirty_mark_rect(x0, y0, rw, (y1 - y0) + 1);
    } 
    // Fallback: desenha direto na VRAM
    else if (g_vram) {
        for (int yy = y0; yy <= y1; yy++) {
            // Cast removendo volatile para passar ao assembly (seguro aqui)
            void* row = (void*)(g_vram + (uint32_t)yy * g_stride + (uint32_t)x0);
            fast_memset32(row, color, rw);
        }
    }
}

static void vesa_clear(uint32_t color) {
    vesa_fill_rect(0, 0, vesa_driver.width, vesa_driver.height, color);
}

// ... (mantenha o resto do arquivo igual) ...

static void vesa_update(void) {
    if (!g_vram || !g_back || !g_dirty) return;

    int x0 = clampi(g_minx, 0, vesa_driver.width - 1);
    int y0 = clampi(g_miny, 0, vesa_driver.height - 1);
    int x1 = clampi(g_maxx, 0, vesa_driver.width - 1);
    int y1 = clampi(g_maxy, 0, vesa_driver.height - 1);
    int rw = (x1 - x0) + 1;

    // --- ZONA CRÍTICA: BLINDAGEM CONTRA TEARING ---
    // Desativa interrupções para que o Timer ou Teclado não 
    // pausem a cópia no meio do desenho.
    __asm__ volatile("cli");

    for (int yy = y0; yy <= y1; yy++) {
        uint32_t off = (uint32_t)yy * g_stride + (uint32_t)x0;
        
        void* dst = (void*)(g_vram + off);
        const void* src = (const void*)(g_back + off);
        
        // Cópia ultra-rápida sem pausas
        fast_memcpy32(dst, src, rw);
    }

    // Reativa interrupções
    __asm__ volatile("sti");
    // ----------------------------------------------

    g_dirty = 0;
}