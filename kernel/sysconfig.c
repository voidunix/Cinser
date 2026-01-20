/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: sysconfig.c
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

#include <stdint.h>
#include "sysconfig.h"
#include "memory.h"

static char g_cpu_brand[49];
static uint32_t g_base_mhz = 0;
static uint32_t g_max_mhz = 0;
static char g_cpu_str[80];

static inline void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(leaf), "c"(subleaf));
    if (a) *a = eax;
    if (b) *b = ebx;
    if (c) *c = ecx;
    if (d) *d = edx;
}

static void str_copy(char *dst, const char *src) {
    while (*src) *dst++ = *src++;
    *dst = 0;
}

static void str_trim_right(char *s) {
    // remove trailing spaces
    int len = 0;
    while (s[len]) len++;
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t')) {
        s[len-1] = 0;
        len--;
    }
}

static void u32_to_dec(uint32_t v, char *out) {
    char tmp[11];
    int i = 0;
    if (v == 0) {
        out[0] = '0';
        out[1] = 0;
        return;
    }
    while (v > 0 && i < 10) {
        tmp[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    int o = 0;
    while (i > 0) out[o++] = tmp[--i];
    out[o] = 0;
}

static void build_cpu_string(void) {
    // Default: just brand
    str_copy(g_cpu_str, g_cpu_brand);

    if (g_base_mhz == 0) {
        return;
    }

    // Append: " @ X.XXGHz"
    char mhz_str[12];
    u32_to_dec(g_base_mhz, mhz_str);

    // Convert MHz to GHz with 2 decimals
    uint32_t ghz_int = g_base_mhz / 1000u;
    uint32_t rem = g_base_mhz % 1000u;
    uint32_t ghz_dec2 = (rem + 5u) / 10u; // 0..100

    char int_str[12];
    u32_to_dec(ghz_int, int_str);

    char dec_str[4];
    dec_str[0] = (char)('0' + (ghz_dec2 / 10u) % 10u);
    dec_str[1] = (char)('0' + (ghz_dec2 % 10u));
    dec_str[2] = 0;

    // concat
    char *p = g_cpu_str;
    while (*p) p++;
    *p++ = ' ';
    *p++ = '@';
    *p++ = ' ';
	const char *q = int_str;
    while (*q) *p++ = *q++;
    *p++ = '.';
    *p++ = dec_str[0];
    *p++ = dec_str[1];
    *p++ = 'G';
    *p++ = 'H';
    *p++ = 'z';
    *p = 0;
}

void sysconfig_init(void) {
    // Brand string via extended CPUID leaves
    uint32_t max_ext = 0;
    cpuid(0x80000000u, 0, &max_ext, 0, 0, 0);

    for (int i = 0; i < 48; i++) g_cpu_brand[i] = 0;

    if (max_ext >= 0x80000004u) {
        uint32_t *p = (uint32_t*)g_cpu_brand;
        cpuid(0x80000002u, 0, &p[0], &p[1], &p[2], &p[3]);
        cpuid(0x80000003u, 0, &p[4], &p[5], &p[6], &p[7]);
        cpuid(0x80000004u, 0, &p[8], &p[9], &p[10], &p[11]);
        g_cpu_brand[48] = 0;
        str_trim_right(g_cpu_brand);
    } else {
        str_copy(g_cpu_brand, "Unknown CPU");
    }

    // Frequência: CPUID leaf 0x16 (nem todo CPU tem)
    uint32_t max_basic = 0;
    cpuid(0u, 0, &max_basic, 0, 0, 0);
    if (max_basic >= 0x16u) {
        uint32_t a, b, c, d;
        cpuid(0x16u, 0, &a, &b, &c, &d);
        // a=base MHz, b=max MHz
        g_base_mhz = a;
        g_max_mhz = b;
    } else {
        g_base_mhz = 0;
        g_max_mhz = 0;
    }

    build_cpu_string();
}

const char* sysconfig_cpu_brand(void) {
    return g_cpu_brand;
}

uint32_t sysconfig_cpu_base_mhz(void) {
    return g_base_mhz;
}

uint32_t sysconfig_cpu_max_mhz(void) {
    return g_max_mhz;
}

const char* sysconfig_cpu_str(void) {
    return g_cpu_str;
}

uint32_t sysconfig_mem_total_kib(void) {
    return memory_total_kib();
}

