/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: shell.c
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

#include "window.h"
#include "programs/shell.h"

// internal from window.c
extern void window_set_on_key(Window* win, void (*cb)(char));

static Window* g_shell = NULL;

// Cursor marker entendido pelo renderer da janela (window.c)
#define SHELL_CURSOR_MARKER ((char)0x1F)

static void shell_banner(void){
    window_write(g_shell, "Cinser Shell (test) ✅\n");
    window_write(g_shell, "TAB: troca foco | Ctrl+W: fecha janela\n");
    window_write(g_shell, "--------------------------------------\n");
    window_write(g_shell, "> ");
    char m[2] = { SHELL_CURSOR_MARKER, 0 };
    window_write(g_shell, m);
}

void shell_open(void){
    if(g_shell) return;
    g_shell = window_make("Shell", 40, 40, 560, 280);
    if(!g_shell) return;
    window_set_on_key(g_shell, shell_key);
    shell_banner();
}

void shell_key(char c){
    if(!g_shell) return;

    if(c=='\r' || c=='\n'){
        window_write(g_shell, "\n> ");
        char m[2] = { SHELL_CURSOR_MARKER, 0 };
        window_write(g_shell, m);
        return;
    }
    if(c==8){
        // backspace real: o renderer interpreta '\b'
        char bs[2] = { '\b', 0 };
        window_write(g_shell, bs);
        char m[2] = { SHELL_CURSOR_MARKER, 0 };
        window_write(g_shell, m);
        return;
    }
    if(c >= 32 && c <= 126){
        char s[2] = { c, 0 };
        window_write(g_shell, s);
        char m[2] = { SHELL_CURSOR_MARKER, 0 };
        window_write(g_shell, m);
    }
}
