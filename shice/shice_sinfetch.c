/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: shice_sinfetch.c
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

#include "console.h"
#include "sysconfig.h"
#include "memory.h"
#include "shice/shice_sinfetch.h"

// Ajuste aqui se quiser outro nome fixo
#define SINFETCH_OS_NAME      "Cinser OS"
#define SINFETCH_KERNEL_NAME  "Cinser Kernel"
#define SINFETCH_KERNEL_VER   "0.0.8"

// Bootloader (hoje o Cinser inicializa via Multiboot v1; GRUB2 é o caso comum)
#define SINFETCH_BOOTLOADER   "GRUB2 MULTIBOOT"

// ---------------- ICON ----------------
static const char* g_sin_icon[] = {
"   % %#    ..                     ...:% %   ",
"   #%#% = ...                    ... =#%#%  ",
"  %%###%== ...        :..      ... .=###%%% ",
"  %######...       =  :..     .. ::+###%##  ",
" %#%######+-.:.     -=-.: = ...==+######%## ",
" %###**#%##*+=..  ==-..-=   ..-+**#%#**###% ",
"  %%#++*%###*+-.. +-:-=.:+ =..+*####*++#%%@ ",
"  %##++*#%###**+:=%##***##+=+**###%#*++##%  ",
"   %##*++*#######*##%++%##***#####*++*##%   ",
"   %@%#=*###%####%*##%%##*%####%%##*+#%@%   ",
"    ###*+++*#%%#%%##****##%##%%#*++**###    ",
"       ###*+*+***#%######%#***+*+*###       ",
"         #%####*#%**####**%#*####%%%        ",
"        = ==  == @*+%%%%*#@@-= =   =        ",
"              = @@@#*%##*@@@                ",
"              = @#%%+##+%@#% =              ",
"             =+***+*#**#*****+++            ",
"           %####%##########%%###@           ",
"              ##%*##***+*#*###              ",
"             @%%**%#%+#*#%%##%@             ",
"               %% %% *##*                   ",
"                     @@                     ",
"                                             ",
NULL
};

static int stlen(const char* s){
    int n = 0;
    if(!s) return 0;
    while(s[n]) n++;
    return n;
}

// trim de espaços à direita pra medir largura "visual"
static int visual_len(const char* s){
    int n = stlen(s);
    while(n > 0 && s[n-1] == ' ') n--;
    return n;
}

static int icon_lines(void){
    int n = 0;
    while (g_sin_icon[n] != NULL) {
        n++;
        if (n > 512) break;
    }
    return n;
}

static int icon_maxw(void){
    int h = icon_lines();
    int w = 0;
    for(int i=0;i<h;i++){
        int lw = visual_len(g_sin_icon[i]);
        if(lw > w) w = lw;
    }
    return w;
}

static void put_line_clamped(const char* s, int max_cols){
    if(!s) return;
    for(int i=0; s[i] && i < max_cols; i++){
        console_putc(s[i]);
    }
}

static void write_label_value(const char* label, const char* value){
    console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    console_write(label);
    console_write(": ");
    console_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    console_write(value ? value : "");
}

static const char* ltrim(const char* s) {
    if (!s) return "";
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

static void write_u32_dec(uint32_t v) {
    // imprime uint32 sem stdlib
    char buf[11];
    int i = 0;

    if (v == 0) {
        console_putc('0');
        return;
    }

    while (v > 0 && i < 10) {
        buf[i++] = (char)('0' + (v % 10u));
        v /= 10u;
    }
    while (i--) console_putc(buf[i]);
}

static void write_cpu_base_clock_ghz(void) {
    // O usuário pediu BASE (fixo)
    uint32_t mhz = sysconfig_cpu_base_mhz();

    if (mhz == 0) {
        console_write("Unknown");
        return;
    }

    // mostra em GHz com 2 casas: X.YY GHz
    // ex: 3200 MHz => 3.20 GHz
    uint32_t ghz_int = mhz / 1000u;
    uint32_t rem = mhz % 1000u;
    uint32_t frac = rem / 10u; // 2 casas (0..99)

    write_u32_dec(ghz_int);
    console_putc('.');
    console_putc((char)('0' + (frac / 10u)));
    console_putc((char)('0' + (frac % 10u)));
    console_write("GHz");
}

static void write_16_colors_blocks_2rows(int x, int y) {
    // linha de cima
    console_set_cursor(x, y);
    for (int c = 0; c < 16; c++) {
        console_set_color((uint8_t)c, VGA_COLOR_BLACK);
        console_putc('\x80');
        console_putc('\x80');
    }

    // linha de baixo (mesmas cores)
    console_set_cursor(x, y + 1);
    for (int c = 0; c < 16; c++) {
        console_set_color((uint8_t)c, VGA_COLOR_BLACK);
        console_putc('\x80');
        console_putc('\x80');
    }

    console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

// Comando: sinfetch
void shice_cmd_sinfetch(void){
    int cols = console_get_cols();
    int rows = console_get_rows();

    // dados
    const char* cpu = ltrim(sysconfig_cpu_brand());      // separado do clock
    const char* ram_total = sysconfig_mem_total_str();

    char used_buf[32];
    char total_buf[32];
    sysconfig_format_kib(memory_used_kib(), used_buf, (uint32_t)sizeof(used_buf));
    sysconfig_format_kib(memory_total_kib(), total_buf, (uint32_t)sizeof(total_buf));

    // labels alinhados (estilo neofetch)
    const char* l0 = "OS       ";
    const char* l1 = "Kernel   ";
    const char* l2 = "CPU      ";
    const char* l3 = "CPU Clock";
    const char* l4 = "RAM      ";
    const char* l5 = "RAM Use  ";
    const char* l6 = "Boot     ";
    const char* l7 = "Colors   ";

    // layout do ícone
    int h = icon_lines();
    int w = icon_maxw();
    int gap = 2;

    // começa abaixo do prompt atual (não sobrescreve o topo)
    int cur_col = 0, cur_row = 0;
    console_get_cursor(&cur_col, &cur_row);

    int start_col = 0;          // encosta na esquerda
    int start_row = cur_row + 1;

    // se não couber, empurra pra cima
    if (start_row + h >= rows) {
        start_row = rows - h;
        if (start_row < 0) start_row = 0;
    }

    // specs ficam à direita do ícone
    int spec_col  = start_col + w + gap;
    int spec_row0 = start_row + 1;   // bom pro ícone pequeno

    // desenha
    for (int i = 0; i < h; i++) {
        int y = start_row + i;
        if (y < 0 || y >= rows) continue;

        // ícone
        console_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        console_set_cursor(start_col, y);
        put_line_clamped(g_sin_icon[i], cols - start_col);

        // se não cabe specs, pula
        if (spec_col >= cols) continue;

        // specs (8 linhas)
        if (y == spec_row0 + 0) {
            console_set_cursor(spec_col, y);
            write_label_value(l0, SINFETCH_OS_NAME);
        }
        if (y == spec_row0 + 1) {
            console_set_cursor(spec_col, y);
            console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            console_write(l1);
            console_write(": ");
            console_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            console_write(SINFETCH_KERNEL_NAME);
            console_write(" ");
            console_write(SINFETCH_KERNEL_VER);
        }
        if (y == spec_row0 + 3) {
            console_set_cursor(spec_col, y);
            write_label_value(l2, cpu);
        }
        if (y == spec_row0 + 5) {
            console_set_cursor(spec_col, y);
            write_label_value(l4, ram_total);
        }
        if (y == spec_row0 + 7) {
            console_set_cursor(spec_col, y);
            console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            console_write(l5);
            console_write(": ");
            console_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            console_write(used_buf);
            console_write("/");
            console_write(total_buf);
        }
        if (y == spec_row0 + 9) {
            console_set_cursor(spec_col, y);
            write_label_value(l6, SINFETCH_BOOTLOADER);
        }
        if (y == spec_row0 + 11) {
            console_set_cursor(spec_col, y);
            console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            console_write(l7);
            console_write(": ");
            int x = spec_col + stlen(l7) + 2; // depois do "Colors   : "
            write_16_colors_blocks_2rows(x, y);
        }
    }

    console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    console_write("\n");
}