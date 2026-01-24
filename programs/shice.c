/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: shice.c
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
#include "keyboard.h"
#include "delay.h"
#include "time.h"
#include "programs/shice.h"
#include "shice/shice_help.h"
#include "shice/shice_sinfetch.h"
#include "sysconfig.h"
#include "memory.h"

// Opcional: permitir entrar no UI se o usuario digitar "ui"
#include "desktop.h"

// util: string ops (freestanding)
static int streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return (*a == 0 && *b == 0);
}

static int starts_with(const char* s, const char* prefix) {
    while (*prefix) {
        if (*s++ != *prefix++) return 0;
    }
    return 1;
}

static void put_prompt(void) {
    console_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	console_write(" \n");
    console_write("shice");
    console_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    console_write("> ");
}

static void newline(void){ console_putc('\n'); }

// Linha com cursor piscando (software) usando ticks do PIT.
//
// Importante:
// - O console agora trata '\b' (backspace) em drivers/console.c.
// - Aqui usamos o classico: \b ' ' \b para apagar na tela.
// - O HLT acorda tanto por IRQ do teclado quanto do PIT, entao o blink funciona sem busy-loop.
static int read_line(char* buf, int maxlen) {
    int len = 0;

    // PIT em 1000 Hz por padrao => 500 ticks ~= 500 ms
    const uint32_t BLINK_TICKS = 500;
    uint32_t last_blink = time_get_ticks();
    int cursor_on = 0;

    // mostra cursor no inicio
    console_putc('_');
    console_putc('\b');
    cursor_on = 1;

    for (;;) {
        if (keyboard_haschar()) {
            int ch = keyboard_getchar();
            if (ch < 0) {
                __asm__ volatile("hlt");
                continue;
            }

            char c = (char)ch;

            // esconde cursor antes de imprimir/alterar
            if (cursor_on) {
                console_putc(' ');
                console_putc('\b');
                cursor_on = 0;
            }

            // Enter
            if (c == '\r' || c == '\n') {
                newline();
                buf[len] = 0;
                return len;
            }

            // Backspace (8) or DEL (127)
            if (c == 8 || c == 127) {
                if (len > 0) {
                    len--;
                    buf[len] = 0;
                    console_putc('\b');
                    console_putc(' ');
                    console_putc('\b');
                }
                // mostra cursor de novo
                console_putc('_');
                console_putc('\b');
                cursor_on = 1;
                continue;
            }

            // Ignore non-printables
            if ((unsigned char)c < 32 || (unsigned char)c > 126) {
                console_putc('_');
                console_putc('\b');
                cursor_on = 1;
                continue;
            }

            if (len < maxlen - 1) {
                buf[len++] = c;
                buf[len] = 0;
                console_putc(c);
            }

            // mostra cursor
            console_putc('_');
            console_putc('\b');
            cursor_on = 1;
            continue;
        }

        // Blink quando estiver ocioso
        uint32_t now = time_get_ticks();
        if ((now - last_blink) >= BLINK_TICKS) {
            last_blink = now;
            if (cursor_on) {
                console_putc(' ');
                console_putc('\b');
                cursor_on = 0;
            } else {
                console_putc('_');
                console_putc('\b');
                cursor_on = 1;
            }
        }

        __asm__ volatile("hlt");
    }
}

static void cmd_clear(void) {
    console_clear();
}

static void cmd_ver(void) {
    console_write("Cinser Kernel 0.0.8 (Running Shice)\n");
    console_write("CPU: ");
    console_write(sysconfig_cpu_str());
    console_write("\n");
    console_write("RAM: ");
    console_write(sysconfig_mem_total_str());
    console_write("\n");
}

static void cmd_echo(const char* line) {
    // line begins with "echo"
    const char* p = line + 4;
    while (*p == ' ') p++;
    console_write(p);
    newline();
}

static void shice_banner(void) {
    console_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
	console_write(" \n");
    console_write("Tervia Cinser - Shice Shell\n");
    console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    console_write("Digite 'help' para ver os comandos.\n\n");
    console_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void shice_init(void) {
    // nothing heavy yet, but keep it for future
    shice_banner();
}

void shice_run(void) {
    shice_init();

    char line[128];

    for (;;) {
        put_prompt();
        int n = read_line(line, (int)sizeof(line));
        if (n <= 0) continue;

        // Trim leading spaces
        char* s = line;
        while (*s == ' ') s++;

        if (streq(s, "help")) { shice_cmd_help(); continue; }
        if (streq(s, "clear")) { cmd_clear(); continue; }
        if (streq(s, "ver")) { cmd_ver(); continue; }
        if (streq(s, "sinfetch")) { shice_cmd_sinfetch(); continue; }
        if (starts_with(s, "echo")) { cmd_echo(s); continue; }

        if (streq(s, "ui")) {
            console_write("Entrando no desktop UI...\n");
            delay_ms(250);
            desktop_init();

            // Reusa a logica do kernel: loop do desktop
            int need_redraw = 1;
            uint32_t last_frame_time = 0;

            for (;;) {
                __asm__ volatile("hlt");

                if (keyboard_haschar()) {
                    while (keyboard_haschar()) {
                        int ch = keyboard_getchar();
                        if (ch > 0) {
                            desktop_key((char)ch);
                            need_redraw = 1;
                        }
                    }
                }

                if (need_redraw) {
                    uint32_t now = time_get_ticks();
                    if ((now - last_frame_time) < 16) continue;
                    last_frame_time = now;
                    desktop_draw();
                    need_redraw = 0;
                }
            }
        }

        console_write("Comando desconhecido. Digite 'help'.\n");
    }
}
