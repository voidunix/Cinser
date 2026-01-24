/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: video.h
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

#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

// A Estrutura Universal de Driver
typedef struct {
    const char* driver_name;
    int width;
    int height;
    int bpp;
    
    // Funcoes que o driver deve ter
    void (*init)(void* info);
    void (*put_pixel)(int x, int y, uint32_t color);
    void (*clear_screen)(uint32_t color);
    void (*update)(void);

    // (Opcional) Aceleradores: quando presentes, evitam desenhar pixel-a-pixel.
    // Drivers antigos podem deixar NULL sem quebrar nada.
    void (*fill_rect)(int x, int y, int w, int h, uint32_t color);
} video_driver_t;

// Variavel global do driver ativo
extern video_driver_t* g_video_driver;

// Funcoes para o Kernel chamar
void video_init_system(void* multiboot_info);
void put_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);

#endif