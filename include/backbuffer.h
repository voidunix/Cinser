/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: backbuffer.h
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
#include <stdint.h>

// Generic 32bpp backbuffer + dirty-rect tracking for Cinser VESA framebuffer.
//
// Design goals:
//  - All drawing happens in RAM (backbuffer).
//  - Present() copies only the dirty rectangle to VRAM.
//  - After a long idle period, the host/VM can "invalidate" the window; in that case,
//    present() can force a full refresh once to avoid white flicker/tearing artifacts.

typedef struct {
    uint32_t* buf;          // backbuffer pixels (size = stride*height)
    int width;
    int height;
    uint32_t stride;        // pixels per scanline

    // dirty rectangle (inclusive)
    int dirty;
    int minx, miny, maxx, maxy;

    // present policy
    uint32_t last_present_ticks;
    uint32_t idle_force_full_ticks; // if now-last_present > this => do full present once
    int force_full_next;
} backbuffer_t;

// Configure the backbuffer metadata, without allocating memory yet.
// Useful when video is initialized before the kernel heap.
void bb_setup(backbuffer_t* bb, int width, int height, uint32_t stride, uint32_t idle_force_full_ticks);

// Try to allocate the backing store if it hasn't been allocated yet.
// Safe to call repeatedly; returns 1 if the buffer exists after the call.
int bb_try_alloc(backbuffer_t* bb);

void bb_init(backbuffer_t* bb, int width, int height, uint32_t stride, uint32_t idle_force_full_ticks);
void bb_shutdown(backbuffer_t* bb);

static inline uint32_t* bb_pixels(backbuffer_t* bb) { return bb ? bb->buf : (uint32_t*)0; }

void bb_mark_dirty(backbuffer_t* bb, int x, int y, int w, int h);
void bb_force_full(backbuffer_t* bb);

void bb_present(backbuffer_t* bb, volatile uint32_t* vram, uint32_t vram_stride, uint32_t now_ticks);
