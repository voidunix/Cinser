/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: memory.h
 * Descricao: Interface do gerenciador de memoria (PMM + heap do kernel).
 * Copyright (C) 2026 Tervia Corporation.
 *
 * Este programa e um software livre: voce pode redistribui-lo e/ou
 * modifica-lo sob os termos da Licenca Publica Geral GNU como publicada
 * pela Free Software Foundation, bem como a versao 3 da Licenca.
 *
 * Este programa e distribuido na esperanca de que possa ser util,
 * mas SEM NENHUMA GARANTIA; sem uma garantia implicita de ADEQUACAO
 * a qualquer MERCADO ou APLICACAO EM PARTICULAR. Veja a
 * Licenca Publica Geral GNU para mais detalhes.
 ****************************************************************************/

#pragma once
#include <stdint.h>

// Inicializa PMM/heap usando Multiboot v1.
// Mantem a mesma assinatura usada no projeto atual.
void memory_init(uint32_t multiboot_magic, uint32_t mb_info_ptr);

// PMM (4KiB pages) - retornam endereco fisico (identity-mapped no seu kernel atual)
uint32_t pmm_alloc_page(void);
void     pmm_free_page(uint32_t paddr);

// Heap do kernel
void* kmalloc(uint32_t size);
void* kmalloc_aligned(uint32_t size, uint32_t align);
void  kfree(void *ptr);

// Estatisticas
uint32_t memory_total_kib(void);
uint32_t memory_used_kib(void);
uint32_t memory_free_kib(void);

// String pronta pra exibir no VGA
const char* meminfo_str(void);
