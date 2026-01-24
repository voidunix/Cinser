/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: time.c
 * Descricao: Servicos de tempo (CMOS/RTC + contagem por IRQ0/PIT).
 ****************************************************************************/

#include <stdint.h>
#include "time.h"
#include "io.h"

// Frequência base do oscilador do chip PIT (1.193182 MHz)
#define PIT_FREQUENCY 1193180

static volatile uint32_t g_ticks = 0;
static volatile uint32_t g_ticks_per_sec = 1000;
static volatile int g_ready = 0;

static volatile rtc_time_t g_time;

// Buffer estatico: "HH:MM:SS DD/MM/YYYY" + '\0' => 20
static char g_time_str[20];
static volatile int g_time_str_dirty = 1;

static inline void put2(char *p, uint8_t v) {
    p[0] = (char)('0' + (v / 10));
    p[1] = (char)('0' + (v % 10));
}

static inline void put4(char *p, uint16_t v) {
    p[0] = (char)('0' + (v / 1000) % 10);
    p[1] = (char)('0' + (v / 100)  % 10);
    p[2] = (char)('0' + (v / 10)   % 10);
    p[3] = (char)('0' + (v % 10));
}

static void rebuild_time_str(void) {
    // "HH:MM:SS DD/MM/YYYY"
    put2(&g_time_str[0],  g_time.hour);
    g_time_str[2] = ':';
    put2(&g_time_str[3],  g_time.min);
    g_time_str[5] = ':';
    put2(&g_time_str[6],  g_time.sec);
    g_time_str[8] = ' ';
    put2(&g_time_str[9],  g_time.day);
    g_time_str[11] = '/';
    put2(&g_time_str[12], g_time.mon);
    g_time_str[14] = '/';
    put4(&g_time_str[15], g_time.year);
    g_time_str[19] = '\0';
}

static void tick_one_second(void) {
    g_time.sec++;
    if (g_time.sec >= 60) {
        g_time.sec = 0;
        g_time.min++;
        if (g_time.min >= 60) {
            g_time.min = 0;
            g_time.hour++;
            if (g_time.hour >= 24) {
                g_time.hour = 0;
                // g_time.day++; // Calendário completo requer lógica de bissexto
            }
        }
    }
    rebuild_time_str();
    g_time_str_dirty = 1;
}

// Handler chamado pelo IRQ0
void time_tick(void) {
    g_ticks++;

    if (!g_ready) return;

    // Atualiza o relógio a cada segundo real
    if ((g_ticks_per_sec != 0) && ((g_ticks % g_ticks_per_sec) == 0)) {
        tick_one_second();
    }
}

// AQUI ESTAVA O ERRO: Faltava programar o chip PIT!
void time_init(uint32_t pit_ticks_per_sec) {
    if (pit_ticks_per_sec == 0) pit_ticks_per_sec = 1000;
    
    // 1. Atualiza a variável do Kernel
    g_ticks_per_sec = pit_ticks_per_sec;

    // 2. Calcula o Divisor (Hardware)
    uint32_t divisor = PIT_FREQUENCY / pit_ticks_per_sec;

    // 3. Envia Comandos para o Chip PIT (Isso que faltava)
    outb(0x43, 0x36); // Modo 3 (Square Wave)
    
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, l);    // Low Byte
    outb(0x40, h);    // High Byte

    // 4. Inicializa RTC
    rtc_time_t t;
    cmos_read_rtc(&t);
    g_time = t;

    rebuild_time_str();
    g_time_str_dirty = 1;
    g_ready = 1;
}

rtc_time_t time_now(void) {
    rtc_time_t t = g_time;
    return t;
}

const char *time_datetime_str(void) {
    return g_time_str;
}

int time_has_update(void) {
    return g_time_str_dirty ? 1 : 0;
}

void time_consume_update(void) {
    g_time_str_dirty = 0;
}

// Retorna o número de Ticks (ms) desde o boot
// Adicione isso no final do arquivo time.c
uint32_t time_get_ticks(void) {
    return g_ticks;
}