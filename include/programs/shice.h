/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: shice.h
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
#ifdef __cplusplus
extern "C" {
#endif

// Inicializa e roda o shell Shice (console/TTY).
// shice_run() não retorna.
void shice_init(void);
void shice_run(void);

#ifdef __cplusplus
}
#endif
