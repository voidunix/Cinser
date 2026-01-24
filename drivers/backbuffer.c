/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: backbuffer.c
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

#include "backbuffer.h"
#include <stddef.h>

// kernel heap
extern void* kmalloc_aligned(uint32_t size, uint32_t align);
extern void  kfree(void* ptr);

static int clampi(int v, int lo, int hi){
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void bb_setup(backbuffer_t* bb, int width, int height, uint32_t stride, uint32_t idle_force_full_ticks){
    if (!bb) return;
    bb->buf = NULL;
    bb->width = width;
    bb->height = height;
    bb->stride = stride;

    bb->dirty = 1;
    bb->minx = 0;
    bb->miny = 0;
    bb->maxx = (width  > 0) ? (width  - 1) : 0;
    bb->maxy = (height > 0) ? (height - 1) : 0;

    bb->last_present_ticks = 0;
    bb->idle_force_full_ticks = idle_force_full_ticks;
    bb->force_full_next = 1;
}

int bb_try_alloc(backbuffer_t* bb){
    if (!bb) return 0;
    if (bb->buf) return 1;
    if (bb->width <= 0 || bb->height <= 0 || bb->stride == 0) return 0;

    uint32_t bytes = (uint32_t)bb->stride * (uint32_t)bb->height * 4u;
    // 16-byte aligned is enough for 32-bit copies; keep it simple and safe.
    uint32_t* p = (uint32_t*)kmalloc_aligned(bytes, 16);
    if (!p) return 0;
    bb->buf = p;

    // Initialize to black to avoid any "garbage frame" on first present.
    for (uint32_t i = 0; i < (uint32_t)bb->stride * (uint32_t)bb->height; i++) {
        bb->buf[i] = 0x00000000u;
    }
    return 1;
}

void bb_init(backbuffer_t* bb, int width, int height, uint32_t stride, uint32_t idle_force_full_ticks){
    bb_setup(bb, width, height, stride, idle_force_full_ticks);
    (void)bb_try_alloc(bb);
}

void bb_shutdown(backbuffer_t* bb){
    if (!bb) return;
    if (bb->buf) kfree(bb->buf);
    bb->buf = NULL;
    bb->dirty = 0;
}

void bb_mark_dirty(backbuffer_t* bb, int x, int y, int w, int h){
    if (!bb) return;
    if (w <= 0 || h <= 0) return;

    int x0 = x;
    int y0 = y;
    int x1 = x + w - 1;
    int y1 = y + h - 1;

    // clip to bounds
    x0 = clampi(x0, 0, bb->width  - 1);
    y0 = clampi(y0, 0, bb->height - 1);
    x1 = clampi(x1, 0, bb->width  - 1);
    y1 = clampi(y1, 0, bb->height - 1);

    if (!bb->dirty) {
        bb->dirty = 1;
        bb->minx = x0; bb->miny = y0;
        bb->maxx = x1; bb->maxy = y1;
        return;
    }

    if (x0 < bb->minx) bb->minx = x0;
    if (y0 < bb->miny) bb->miny = y0;
    if (x1 > bb->maxx) bb->maxx = x1;
    if (y1 > bb->maxy) bb->maxy = y1;
}

void bb_force_full(backbuffer_t* bb){
    if (!bb) return;
    bb->force_full_next = 1;
    bb_mark_dirty(bb, 0, 0, bb->width, bb->height);
}

void bb_present(backbuffer_t* bb, volatile uint32_t* vram, uint32_t vram_stride, uint32_t now_ticks){
    if (!bb || !vram) return;

    // Video can be initialized before the heap; lazily allocate on first present.
    if (!bb->buf) {
        if (!bb_try_alloc(bb)) return;
        // after a late allocation, force a full refresh once
        bb->force_full_next = 1;
        bb->dirty = 1;
        bb->minx = 0; bb->miny = 0;
        bb->maxx = (bb->width  > 0) ? (bb->width  - 1) : 0;
        bb->maxy = (bb->height > 0) ? (bb->height - 1) : 0;
    }

    // Decide whether we must refresh the entire screen.
    int do_full = 0;
    if (bb->force_full_next) do_full = 1;

    if (!do_full && bb->idle_force_full_ticks != 0) {
        uint32_t dt = now_ticks - bb->last_present_ticks;
        if (dt > bb->idle_force_full_ticks) do_full = 1;
    }

    if (do_full) {
        // Full screen copy
        int w = bb->width;
        int h = bb->height;
        for (int y = 0; y < h; y++) {
            uint32_t off = (uint32_t)y * bb->stride;
            volatile uint32_t* dst = vram + (uint32_t)y * vram_stride;
            const uint32_t* src = bb->buf + off;
            for (int x = 0; x < w; x++) dst[x] = src[x];
        }
        bb->dirty = 0;
        bb->force_full_next = 0;
        bb->last_present_ticks = now_ticks;
        return;
    }

    if (!bb->dirty) return;

    int x0 = bb->minx;
    int y0 = bb->miny;
    int x1 = bb->maxx;
    int y1 = bb->maxy;

    int rw = (x1 - x0) + 1;

    for (int yy = y0; yy <= y1; yy++) {
        uint32_t off = (uint32_t)yy * bb->stride + (uint32_t)x0;
        volatile uint32_t* dst = vram + (uint32_t)yy * vram_stride + (uint32_t)x0;
        const uint32_t* src = bb->buf + off;
        for (int i = 0; i < rw; i++) dst[i] = src[i];
    }

    bb->dirty = 0;
    bb->last_present_ticks = now_ticks;
}
