/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: window.h
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
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Window Window;

Window* window_make(const char* title, int x, int y, int w, int h);
void    window_close(Window* win);
void    window_focus(Window* win);          // NULL => cycle focus
void    window_draw_all(void);              // draw all in z-order
void    window_key(Window* win, char c);    // sends to window (focused by desktop)
void    window_write(Window* win, const char* text);

#ifdef __cplusplus
}
#endif
