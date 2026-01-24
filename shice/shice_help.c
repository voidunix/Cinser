/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: shice_help.c
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

#include "console.h"
#include "shice/shice_help.h"

void shice_cmd_help(void) {
    console_write("Shice Shell - available commands:\n");
    console_write("  help   - shows help with commands\n");
    console_write("  clear  - Clears the terminal screen\n");
    console_write("  echo   - Displays the user's text\n");
    console_write("  * echo <sintaxe>\n");
    console_write("  ver    - Shows the current version of the Cinser\n");
    console_write("  ui     - Enters Interface mode (non-functional)\n");
}
