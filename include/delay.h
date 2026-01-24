/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: delay.h
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
#include <stdint.h>

/*
  delay.h - simple sleep/delay utilities for Cinser

  How it works:
    - delay_tick() MUST be called from your timer IRQ (IRQ0) handler.
    - delay_init(tps) should be called after you initialize PIT/time with the same TPS.

  Notes:
    - delay_ticks()/delay_ms()/delay_time() require interrupts enabled (sti),
      otherwise the CPU will halt forever because no IRQ wakes it.
*/

void delay_init(uint32_t ticks_per_sec);
void delay_tick(void);

uint32_t delay_get_ticks(void);

void delay_ticks(uint32_t ticks);
void delay_time(uint32_t seconds);
void delay_ms(uint32_t ms);
