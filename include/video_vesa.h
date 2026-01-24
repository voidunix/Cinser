/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: video_vesa.h
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

#ifndef VIDEO_VESA_H
#define VIDEO_VESA_H

#include <stdint.h>

// Helpers expostos pelo driver VESA para rotinas de desenho rápidas.
// (Somente válidos quando o driver ativo é o VESA framebuffer.)

volatile uint32_t* vesa_framebuffer_ptr(void);
uint32_t vesa_pitch_pixels(void);
void vesa_fill_rect_fast(int x, int y, int w, int h, uint32_t color);

#endif
