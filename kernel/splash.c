/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: splash.c
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

#include "splash.h"
#include "console.h"
#include "delay.h"
#include <stdint.h>
#include <stddef.h>

// Arte ASCII 
static const char* const g_splash_art[] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "                   :",
    "              %#  #*                                                            #@#  #",
    "              ++% %*%                                                           .+@ **+",
    "              +==%#++%                                                         #=#=#-++",
    "            %# *:-#+-*%                                                       +=+++-=* %#",
    "            *-#*#--+*-#%                                                     *+-#=-=%*=+%",
    "            #=--=*---=--%                                                   =:-=--+#=--*",
    "          %==*#---++---++=                                              ..-++---=+---+#+=%",
    "            *-:-#------#--=+:.                                        .-+*--#=-----*=-:=+",
    "          %#**#--------*++----::                                    .:----+##--------*+*#%",
    "           #*-----::--==#*------:.        . .          .           ::::---*#==:-::-----*#",
    "           %#*#%%+-:...*##=-==-:::        %.     ::.    .#        :------=*%*...::=#%%##%",
    "            #*=-----::--@+=---==---:.     ##-----::--:--**     .:---------*%--::----:=##",
    "            #+----:--::--#*----==-:---:.  =@=-   :::  --%-  .----::------#*:--------:-=#",
    "               @%*=--.   .:*-----===--------#--=-  :--=+----------------+-. .. :-+###",
    "             %=--------:  :-+#------------*-+#*#@  @%**+-#=----------=#+-. .:--------+%",
    "                 *--:::--:.:-=+%------=-=%-----==%%===----#=--------#+=-:.:--:.:-=*",
    "               #****#=-: .-..-:=+*-=%+-+=@=-:----=-------=%=+==#==+==-:..:..:-+##**#%",
    "                  #=--=#---:-: :-::-=%=##+#+-=-=---==--:+%+@#-%=:..-: --::--%+---%",
    "                     #=--#=--:--- :--::-:-#%%--======-=%%#---::--: ---:--=#--+%",
    "                        #++*+=--.----.:-*=#::-+=----=+=::%=#-:.----.--=*#++#",
    "                            %%%%**##+=## ##:-:-++==++-:-:%  *#=+*#***%%%",
    "                                        ##%: -=#++++#--.-#**",
    "                                       %%@@# %+%*==*%+@ @@@%%",
    "                                      %@%%@@@# -*--== *@@@%%@@",
    "                                      %@@@@@@.------::.@@@@@@%",
    "                                        =---@@% .:  .%@@---:",
    "                                     =-:.::...@--:-:-% ::-: .==   ..",
    "                                 #=--= :#=----------------=%-:+-:##",
    "                             %@+++%%@%+=---------------------=#%%++++%%",
    "                                   %+--+--:--:------:-------=--+#",
    "                                  *--*#=-.-%---+:---::-#----+%#+-+%",
    "                                 #*# #=::=**--**-:-*+::#%*%=-=#",
    "                                     #=*% %+-% #---%*=::%    #@#",
    "                                           @%  ##-+   %*%",
    "                                                 %",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
};

static int splash_art_lines(void) {
    return (int)(sizeof(g_splash_art) / sizeof(g_splash_art[0]));
}

// Função auxiliar para calcular métricas da imagem ignorando espaços à esquerda
static void splash_get_metrics(int* out_padding, int* out_visual_width) {
    int n = splash_art_lines();
    int min_lead = 1000; // Começa grande
    int max_vis_w = 0;

    // 1. Descobre qual é a menor indentação (espaços à esquerda) em todas as linhas
    for (int i = 0; i < n; i++) {
        const char* s = g_splash_art[i];
        if (!s || !s[0]) continue; // Pula linhas vazias

        int lead = 0;
        while (s[lead] == ' ') lead++;

        // Se a linha for só espaços, ignora
        if (s[lead] == '\0') continue;

        if (lead < min_lead) min_lead = lead;
    }

    if (min_lead == 1000) min_lead = 0; // Caso a imagem seja vazia

    // 2. Calcula a largura visual máxima (tamanho total - indentação comum)
    for (int i = 0; i < n; i++) {
        const char* s = g_splash_art[i];
        if (!s || !s[0]) continue;

        int len = 0;
        while (s[len]) len++;

        // Largura visual é o comprimento total menos a gordura da esquerda
        int vis_w = len - min_lead;
        if (vis_w > max_vis_w) max_vis_w = vis_w;
    }

    *out_padding = min_lead;
    *out_visual_width = max_vis_w;
}

void splash_show(uint32_t seconds) {
    // Limpa e prepara
    console_clear();
    console_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);

    int cols = console_get_cols();
    int rows = console_get_rows();
    int art_h = splash_art_lines();
    
    // Pega as métricas corrigidas (ignorando o espaço morto à esquerda)
    int padding_left, art_w;
    splash_get_metrics(&padding_left, &art_w);

    // Centraliza baseado na largura VISUAL, não na string inteira
    int start_col = (cols > art_w) ? (cols - art_w) / 2 : 0;
    int start_row = (rows > art_h) ? (rows - art_h) / 2 : 0;

    // Variável para rastrear a última linha onde algo foi realmente desenhado
    int last_y_pos = start_row;

    // Desenha a arte linha por linha
    for (int y = 0; y < art_h && (start_row + y) < rows; y++) {
        const char* line = g_splash_art[y];
        if (!line) line = "";
        
        int len = 0;
        while(line[len]) len++;
        
        if (len <= padding_left) {
            continue;
        }

        console_set_cursor(start_col, start_row + y);
        const char* visual_part = line + padding_left;
        
        for (int x = 0; visual_part[x] && (start_col + x) < cols; x++) {
            console_putc(visual_part[x]);
        }
        
        // Atualiza a posição da última linha de conteúdo
        last_y_pos = start_row + y;
    }

// --- Adição do Texto da Versão ---
    const char* version_lines[] = {
        ",ad8888ba,   88                                                  ",
        "d8\"'    `\"8b  \"\"                                                  ",
        "d8'                                                               ",
        "88             88  8b,dPPYba,   ,adPPYba,   ,adPPYba,  8b,dPPYba,  ",
        "88             88  88P'   `\"8a  I8[    \"\"  a8P_____88  88P'   \"Y8  ",
        "Y8,            88  88       88   `\"Y8ba,   8PP\"\"\"\"\"\"\"  88          ",
        " Y8a.    .a8P  88  88       88  aa    ]8I  \"8b,   ,aa  88          ",
        "  `\"Y8888Y\"'   88  88       88  `\"YbbdP\"'   `\"Ybbd8\"'  88          ",
        "",
        "Cinser Kernel v0.0.8"
    };

    int version_count = sizeof(version_lines) / sizeof(version_lines[0]);
    int text_row = last_y_pos + 2; // Começa 2 linhas abaixo da arte anterior

    console_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    for (int i = 0; i < version_count; i++) {
        if (text_row + i >= rows) break; // Evita escrever fora da tela

        // Calcula a largura real desta linha específica
        int line_len = 0;
        while (version_lines[i][line_len]) line_len++;

        // Centraliza a linha atual
        int text_col = (cols > line_len) ? (cols - line_len) / 2 : 0;

        console_set_cursor(text_col, text_row + i);
        console_write(version_lines[i]);
    }

    // --- Configurações da Barra ---
    int progress_width = 30; 
    int progress_col = (cols - (progress_width + 2)) / 2;
    int progress_row = text_row + version_count + 2;

    if (progress_row < rows) {
        for (int i = 0; i <= progress_width; i++) {
            console_set_cursor(progress_col, progress_row);
            
            // Borda lateral
            console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            console_putc('[');

            // Preenchimento com o NOVO caractere 128 (Cast para unsigned char)
            console_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            for (int j = 0; j < i; j++) {
                console_putc('\x80');   // OU console_putc((char)128);
            }
            
            // Espaço vazio (caractere 32 é o espaço ' ')
            console_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
            for (int j = i; j < progress_width; j++) {
                console_putc('.'); // Ou use ' ' para ficar vazio
            }

            console_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            console_putc(']');

            delay_ms(150); 
        }
    }

    // Segura na tela
    delay_time(seconds);
}