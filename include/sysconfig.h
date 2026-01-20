/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: sysconfig.h
 * Descrição: Informações do sistema (CPU, memória).
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

// Inicializa cache de infos (pode ser chamado após memory_init).
void sysconfig_init(void);

// CPU brand string (estática)
const char* sysconfig_cpu_brand(void);

// Frequência base em MHz, se disponível (0 se indisponível)
uint32_t sysconfig_cpu_base_mhz(void);
uint32_t sysconfig_cpu_max_mhz(void);

// Retorna string pronta: "<CPU BRAND> @ X.XXGHz" ou só o brand.
const char* sysconfig_cpu_str(void);

// Memória total do PC em KiB (derivado do memory manager)
uint32_t sysconfig_mem_total_kib(void);

