/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: math.c
 * Descricao: Implementacao de utilitarios matematicos para o kernel (i386).
 ****************************************************************************/
#include "math.h"

/* Euclides */
uint32_t gcd_u32(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

uint32_t lcm_u32(uint32_t a, uint32_t b) {
    if (a == 0 || b == 0) return 0;
    return a / gcd_u32(a, b) * b;
}

bool umul_overflow_u32(uint32_t a, uint32_t b, uint32_t* out) {
    /* 64-bit temporario: gcc vai gerar sequencia sem libc */
    uint64_t p = (uint64_t)a * (uint64_t)b;
    if (out) *out = (uint32_t)p;
    return p > 0xFFFFFFFFull;
}

/*
 * Divisao 64/64 (unsigned) por shift-subtract.
 * - Evita depender de __udivdi3 / libgcc em alguns setups freestanding.
 * - Nao e a mais rapida do mundo, mas e deterministica e pequena.
 * - d == 0: retorna 0 e rem = n (para evitar crash; trate no caller se quiser).
 */
uint64_t udiv64(uint64_t n, uint64_t d, uint64_t* rem_out) {
    if (d == 0) {
        if (rem_out) *rem_out = n;
        return 0;
    }

    uint64_t q = 0;
    uint64_t r = 0;

    for (int i = 63; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1ull);
        if (r >= d) {
            r -= d;
            q |= (1ull << i);
        }
    }

    if (rem_out) *rem_out = r;
    return q;
}
