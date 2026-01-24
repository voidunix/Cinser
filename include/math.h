/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: math.h
 * Descricao: Utilitarios matematicos/bits/alinhamento para o kernel (i386).
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
#include <stddef.h>
#include <stdbool.h>

/* -----------------------------
 * Macros basicas (com cuidado)
 * ----------------------------- */

/* Min/Max genericos (GCC/Clang). Evitam dupla avaliacao. */
#ifndef KMIN
#define KMIN(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a < _b ? _a : _b; })
#endif

#ifndef KMAX
#define KMAX(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })
#endif

#ifndef KCLAMP
#define KCLAMP(x, lo, hi) ({ __typeof__(x) _x = (x); __typeof__(lo) _lo = (lo); __typeof__(hi) _hi = (hi); \
                             _x < _lo ? _lo : (_x > _hi ? _hi : _x); })
#endif

/* -----------------------------
 * Bits / potencias de 2
 * ----------------------------- */

static inline bool is_pow2_u32(uint32_t x) {
    return x && ((x & (x - 1u)) == 0u);
}

static inline uint32_t round_down_pow2_u32(uint32_t x) {
    if (x == 0) return 0;
    /* deixa apenas o bit mais significativo */
    uint32_t r = 1u << (31 - __builtin_clz(x));
    return r;
}

static inline uint32_t round_up_pow2_u32(uint32_t x) {
    if (x <= 1) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

/* log2 floor para x>0 */
static inline uint32_t log2_floor_u32(uint32_t x) {
    return 31u - (uint32_t)__builtin_clz(x);
}

/* -----------------------------
 * Alinhamento (power-of-two)
 * ----------------------------- */

static inline uintptr_t align_down(uintptr_t x, uintptr_t align) {
    /* align deve ser potencia de 2 */
    return x & ~(align - 1u);
}

static inline uintptr_t align_up(uintptr_t x, uintptr_t align) {
    return (x + (align - 1u)) & ~(align - 1u);
}

/* -----------------------------
 * Divisoes utilitarias
 * ----------------------------- */

static inline uint32_t div_ceil_u32(uint32_t a, uint32_t b) {
    return (a + b - 1u) / b;
}

static inline uint64_t div_ceil_u64(uint64_t a, uint64_t b) {
    return (a + b - 1ull) / b;
}

/* -----------------------------
 * Comparacao segura para contadores que podem overflow (tick, etc.)
 * Retorna true se (now - deadline) >= 0 na aritmetica modular.
 * ----------------------------- */
static inline bool time_reached_u32(uint32_t now, uint32_t deadline) {
    return (int32_t)(now - deadline) >= 0;
}

static inline bool time_reached_u64(uint64_t now, uint64_t deadline) {
    return (int64_t)(now - deadline) >= 0;
}

/* -----------------------------
 * Funcoes (implementadas em math.c)
 * ----------------------------- */

uint32_t gcd_u32(uint32_t a, uint32_t b);
uint32_t lcm_u32(uint32_t a, uint32_t b);

/* Multiplicacao com overflow detectado: retorna true se overflow ocorreu. */
bool umul_overflow_u32(uint32_t a, uint32_t b, uint32_t* out);

/* Divisao/mod 64-bit sem depender de libgcc (algoritmo de shift). */
uint64_t udiv64(uint64_t n, uint64_t d, uint64_t* rem_out);
