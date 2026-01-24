/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: window.c
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
#include "font.h"
#include "window.h"

// Ticks (ms) desde o boot (time.c). Não há header público no projeto.
extern uint32_t time_get_ticks(void);

// Alocadores do Kernel
extern void* kmalloc(uint32_t size);
extern void  kfree(void* ptr);

// Hook do desktop
extern void __desktop_set_focused(struct Window* w);

// Helpers de memória
static size_t kstrlen(const char* s) { size_t n=0; while (s && s[n]) n++; return n; }
static void* kmemset(void* p, int v, size_t n){ uint8_t* b=(uint8_t*)p; for(size_t i=0;i<n;i++) b[i]=(uint8_t)v; return p; }
static void* kmemcpy(void* d, const void* s, size_t n){ uint8_t* dd=(uint8_t*)d; const uint8_t* ss=(const uint8_t*)s; for(size_t i=0;i<n;i++) dd[i]=ss[i]; return d; }
static void* kmemmove(void* d, const void* s, size_t n){
    uint8_t* dd=(uint8_t*)d; const uint8_t* ss=(const uint8_t*)s;
    if (dd==ss || n==0) return d;
    if (dd < ss){ for(size_t i=0;i<n;i++) dd[i]=ss[i]; }
    else { for(size_t i=n;i>0;i--) dd[i-1]=ss[i-1]; }
    return d;
}

#define WIN_MAX_TITLE  48
#define WIN_LOG_BYTES  2048

typedef struct Window {
    int x, y, w, h;
    int closed;
    int z;
    int focused;
    char title[WIN_MAX_TITLE];
    char log[WIN_LOG_BYTES];
    size_t log_len;
    void (*on_key)(char c);
    struct Window* next;
} Window;

static Window* g_head = NULL;
static int g_z_counter = 1;
static Window* g_focused = NULL;

// Wrapper direto para evitar overhead
static inline void putpix(int x, int y, uint32_t c){ 
    put_pixel(x,y,c); 
}

static void fill_rect(int x, int y, int w, int h, uint32_t c){
    draw_rect(x,y,w,h,c);
}

// Otimização: Desenha char checando bits mais rápido
static void draw_char8(int x, int y, char ch, uint32_t fg){
    uint8_t c = (uint8_t)ch;
    if (c >= 128) c = '?';
    const uint8_t* glyph = font8x8_basic[c];
    
    for(int row=0; row<8; row++){
        uint8_t bits = glyph[row];
        // Loop desenrolado ou simples, mas evitando contas desnecessárias
        int py = y + row;
        if(bits & 0x01) putpix(x+7, py, fg);
        if(bits & 0x02) putpix(x+6, py, fg);
        if(bits & 0x04) putpix(x+5, py, fg);
        if(bits & 0x08) putpix(x+4, py, fg);
        if(bits & 0x10) putpix(x+3, py, fg);
        if(bits & 0x20) putpix(x+2, py, fg);
        if(bits & 0x40) putpix(x+1, py, fg);
        if(bits & 0x80) putpix(x+0, py, fg);
    }
}

static void draw_text8(int x, int y, const char* s, uint32_t fg){
    int cx = x;
    for(size_t i=0; s && s[i]; i++){
        if (s[i]=='\n'){ y += 10; cx = x; continue; }
        draw_char8(cx, y, s[i], fg);
        cx += 8;
    }
}

static void win_bring_to_front(Window* w){ if(!w) return; w->z = ++g_z_counter; }

static Window* win_topmost(void){
    Window* best=NULL;
    int bestz = (int)0x80000000;
    for(Window* it=g_head; it; it=it->next){
        if(it->closed) continue;
        if(!best || it->z >= bestz){ bestz=it->z; best=it; }
    }
    return best;
}

static Window* win_next_after(Window* cur){
    if(!g_head) return NULL;
    int curz = cur ? cur->z : 0x7FFFFFFF;
    Window* best=NULL;
    int bestz = (int)0x80000000;
    for(Window* it=g_head; it; it=it->next){
        if(it->closed) continue;
        if(it->z < curz && (!best || it->z > bestz)){ bestz=it->z; best=it; }
    }
    if(best) return best;
    return win_topmost();
}

static void win_set_focus(Window* w){
    if(g_focused) g_focused->focused=0;
    g_focused=w;
    if(g_focused){
        g_focused->focused=1;
        win_bring_to_front(g_focused);
    }
    __desktop_set_focused(g_focused);
}

Window* window_make(const char* title, int x, int y, int w, int h){
    if(!title || w < 120 || h < 80) return NULL;
    Window* win=(Window*)kmalloc((uint32_t)sizeof(Window));
    if(!win) return NULL;
    kmemset(win, 0, sizeof(Window));

    win->x=x; win->y=y; win->w=w; win->h=h;
    win->z=++g_z_counter;

    size_t n=0;
    while(title[n] && n < WIN_MAX_TITLE-1){ win->title[n]=title[n]; n++; }
    win->title[n]=0;

    win->next=g_head;
    g_head=win;
    win_set_focus(win);
    return win;
}

void window_close(Window* win){
    if(!win) return;
    win->closed=1;
    Window** pp=&g_head;
    while(*pp){
        if(*pp==win){ *pp=win->next; break; }
        pp=&((*pp)->next);
    }
    if(g_focused==win) win_set_focus(win_topmost());
    kfree(win);
}

void window_focus(Window* win){
    if(!win){
        Window* next=win_next_after(g_focused);
        if(next) win_set_focus(next);
        return;
    }
    if(win->closed) return;
    win_set_focus(win);
}

void window_write(Window* win, const char* text){
    if(!win || !text) return;
    size_t tlen=kstrlen(text);
    if(!tlen) return;

    if(tlen >= WIN_LOG_BYTES){
        text += (tlen - (WIN_LOG_BYTES-1));
        tlen = WIN_LOG_BYTES-1;
        win->log_len = 0;
    } else if (win->log_len + tlen >= WIN_LOG_BYTES){
        size_t overflow = (win->log_len + tlen) - (WIN_LOG_BYTES - 1);
        if (overflow > win->log_len) overflow = win->log_len;
        kmemmove(win->log, win->log + overflow, win->log_len - overflow);
        win->log_len -= overflow;
    }

    kmemcpy(win->log + win->log_len, text, tlen);
    win->log_len += tlen;
    win->log[win->log_len]=0;
}

void window_key(Window* win, char c){
    if(!win || win->closed) return;
    if(c == 23){ window_close(win); return; }
    if(win->on_key) win->on_key(c);
}

static void draw_close_btn(int x, int y, int s, int hot){
    uint32_t bg = hot ? 0x00D04040 : 0x00B03030;
    fill_rect(x,y,s,s,bg);
    draw_text8(x+3,y+3,"X",0x00FFFFFF);
}

static void draw_client_text(Window* w, int cx, int cy, int cw, int ch){
    const int line_h = 10;
    int max_lines = ch / line_h;
    if(max_lines < 1) return;

    // Colunas visíveis (fonte 8px + padding). Mantém o texto DENTRO da janela.
    int cols = (cw - 8) / 8;
    if(cols < 1) cols = 1;
    if(cols > 240) cols = 240; // segurança

    // Renderiza por "layout": aplica \n, wrap por colunas e \b (backspace)
    // sem modificar o log real.
    char lines[64][241];
    int line_count = 0;
    int cur_len = 0;
    int cursor_line = -1;
    int cursor_col  = -1;

    // inicializa primeira linha
    kmemset(lines, 0, sizeof(lines));
    line_count = 1;

    const char* buf = w->log;
    size_t len = w->log_len;
    for(size_t i=0; i<len; i++){
        unsigned char chv = (unsigned char)buf[i];

        if(chv == '\r') continue;

        // Cursor marker (não imprime). Usado pelo shell.
        if(chv == 0x1F){
            cursor_line = line_count - 1;
            cursor_col  = cur_len;
            continue;
        }

        if(chv == '\n'){
            // nova linha
            if(line_count < 64){
                line_count++;
                cur_len = 0;
            }
            continue;
        }

        if(chv == '\b' || chv == 8){
            // backspace: remove um char da linha atual
            if(cur_len > 0){
                cur_len--;
                lines[line_count-1][cur_len] = 0;
                // Se cursor estava após isso, anda junto
                if(cursor_line == line_count-1 && cursor_col > cur_len) cursor_col = cur_len;
            }
            continue;
        }

        // Apenas imprimíveis básicos
        if(chv < 32 || chv > 126) continue;

        // wrap
        if(cur_len >= cols){
            if(line_count < 64){
                line_count++;
                cur_len = 0;
            } else {
                // sem espaço: descarta chars extras
                continue;
            }
        }

        lines[line_count-1][cur_len++] = (char)chv;
        lines[line_count-1][cur_len] = 0;
    }

    // Cursor piscando (500ms). Se não tiver marker, não desenha cursor.
    int cursor_on = 0;
    if(w->focused && cursor_line >= 0 && cursor_col >= 0){
        uint32_t t = time_get_ticks();
        cursor_on = ((t / 500) & 1) ? 0 : 1;
    }

    // Seleciona as últimas linhas que cabem
    int draw_lines = (line_count < max_lines) ? line_count : max_lines;
    int start = line_count - draw_lines;

    int y = cy + ch - line_h * draw_lines;
    for(int li=0; li<draw_lines; li++){
        char tmp[241];
        kmemset(tmp, 0, sizeof(tmp));
        kmemcpy(tmp, lines[start + li], 240);

        // Aplica cursor se esta é a linha do cursor
        if(cursor_on && (start + li) == cursor_line){
            int ccol = cursor_col;
            if(ccol < 0) ccol = 0;
            if(ccol >= cols){
                // cursor caiu no wrap: tenta desenhar no fim
                ccol = cols - 1;
            }
            // garante terminador
            if(ccol >= 240) ccol = 240;
            // se cursor está além do fim atual, preenche com espaço
            int cur_l = (int)kstrlen(tmp);
            while(cur_l < ccol && cur_l < 240){ tmp[cur_l++] = ' '; }
            if(ccol < 240){
                tmp[ccol] = '_';
                if(ccol+1 < 241) tmp[ccol+1] = 0;
            }
        }

        draw_text8(cx+4, y+1, tmp, 0x00000000);
        y += line_h;
    }
}

// OTIMIZAÇÃO CRÍTICA: Não pinta o fundo cinza onde vai ser branco!
static void window_draw_one(Window* win){
    if(!win || win->closed) return;

    const int border=2;
    const int title_h=18;
    const int btn=14;

    uint32_t frame=0x00000000;
    uint32_t bg=0x00E6E6E6;
    uint32_t title= win->focused ? 0x000A246A : 0x00606060;

    // 1. Bordas (Apenas as tiras, não o meio)
    fill_rect(win->x, win->y, win->w, border, frame);               // Topo
    fill_rect(win->x, win->y+win->h-border, win->w, border, frame); // Base
    fill_rect(win->x, win->y, border, win->h, frame);               // Esq
    fill_rect(win->x+win->w-border, win->y, border, win->h, frame); // Dir
    
    // 2. Título
    fill_rect(win->x+border, win->y+border, win->w-border*2, title_h, title);
    draw_text8(win->x+border+6, win->y+border+4, win->title, 0x00FFFFFF);

    int bx = win->x + win->w - border - btn - 2;
    int by = win->y + border + 2;
    draw_close_btn(bx, by, btn, win->focused);

    // 3. Área do Cliente (Branco)
    // Antes pintava de cinza aqui e depois de branco em cima. Agora pinta direto branco.
    int cx = win->x + border;
    int cy = win->y + border + title_h;
    int cw = win->w - border*2;
    int ch = win->h - border*2 - title_h;

    // Se sobrar espaço entre titulo e cliente, pinte de cinza aqui (opcional)
    // Mas para o shell, vamos direto pro branco
    fill_rect(cx, cy, cw, ch, 0x00FFFFFF);
    
    // 4. Texto
    draw_client_text(win, cx, cy, cw, ch);
}

void window_draw_all(void){
    int drawn=0;
    while(drawn < 128){
        Window* best=NULL;
        int bestz=(int)0x80000000;
        for(Window* it=g_head; it; it=it->next){
            if(it->closed) continue;
            if(!best || it->z < bestz){ bestz=it->z; best=it; }
        }
        if(!best) break;

        int saved=best->z;
        best->z = 0x7FFFFFFF;
        window_draw_one(best);
        best->z = saved;
        drawn++;
    }
}

// Hook
void window_set_on_key(Window* win, void (*cb)(char)) { if(win) win->on_key = cb; }