/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: shice_calc.h
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

#ifndef CALC_H
#define CALC_H

#include <stdint.h>

/*
 * SCALE:
 * Usado no futuro para operações que precisem de "decimais"
 * Exemplo: divisão com ponto fixo.
 *
 * Por enquanto NÃO é usado na soma, mas já fica definido
 * como base do sistema.
 */
#define SCALE 100

/*
 * Função pública do comando calc.
 * Ela será chamada pelo shell quando o usuário digitar:
 *
 *   calc 10 + 10
 */
void shice_cmd_calc(const char* line);

#endif
