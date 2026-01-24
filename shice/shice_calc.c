/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: shice_calc.c
 * Descrição: Comando "calc" do SHICE (shell) - base para calculadora.
 *
 * Atualmente suporta apenas soma:
 *   > calc 10 + 10
 *   20
 *
 * Observações (kernel-friendly):
 * - Não usa float.
 * - Faz parse de números inteiros (uint64_t) com limite de 20 dígitos.
 * - Já deixa SCALE definido (para futuras divisões em ponto fixo).
 ****************************************************************************/

#include "shice/shice_calc.h"
#include "console.h"
#include <stdint.h>

/*
 * SCALE (ponto fixo):
 * Futuramente você vai usar isso para tratar "casas decimais" sem float.
 *
 * Exemplo de ideia (não implementado aqui):
 *   1.23 seria armazenado como 123 (com SCALE=100)
 *
 * Como o header pode também definir SCALE, protegemos para não redefinir.
 */
#ifndef SCALE
#define SCALE 100
#endif

static void newline(void){ console_putc('\n'); }

/* ============================================================================
 * 1) Helpers básicos (texto)
 * ============================================================================
 * Funções pequenas para mexer na string de entrada.
 * Não fazem matemática; só ajudam a "andar" na string.
 */

static int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

static void skip_spaces(const char** p) {
    while (**p == ' ')
        (*p)++;
}

/* ============================================================================
 * 2) Parse de número (uint64_t, até 20 dígitos)
 * ============================================================================
 * Lê uma sequência de dígitos e converte para uint64_t.
 *
 * Exemplo: se *p aponta para "1234 + 5", após parse:
 *  - out = 1234
 *  - *p passa a apontar para " + 5"
 *
 * Retorna:
 *  - 1 se OK
 *  - 0 se erro (nenhum dígito, mais de 20 dígitos, ou overflow)
 */

static int parse_u64_20digits(const char** p, uint64_t* out) {
    uint64_t value = 0;
    int digits = 0;

    while (is_digit(**p)) {
        uint64_t d = (uint64_t)(**p - '0');

        /*
         * Proteção contra overflow:
         * Antes de fazer: value = value * 10 + d
         * garantimos que "value * 10" não estoura.
         */
        if (value > UINT64_MAX / 10ULL)
            return 0;

        value = value * 10ULL + d;

        (*p)++;
        digits++;

        /* Limite explícito de 20 algarismos */
        if (digits > 20)
            return 0;
    }

    /* Se não leu nenhum dígito, erro (ex: string começa com '+') */
    if (digits == 0)
        return 0;

    *out = value;
    return 1;
}

/* ============================================================================
 * 3) Operações matemáticas (cada uma em sua função)
 * ============================================================================
 * Aqui você vai colocar +, -, *, /.
 * A ideia é: o parser pega a, op, b; e essas funções fazem a conta.
 *
 * Retorna:
 *  - 1 se OK
 *  - 0 se erro (overflow, etc.)
 */

static int calc_add_u64(uint64_t a, uint64_t b, uint64_t* out) {
    /* Overflow na soma: se a > MAX - b, vai estourar */
    if (a > UINT64_MAX - b)
        return 0;

    *out = a + b;
    return 1;
}

static int calc_sub_u64(uint64_t a, uint64_t b, uint64_t* out) {
    if (a < b)
        return 0; 

    *out = a - b;
    return 1;
}

static int calc_mul_u64(uint64_t a, uint64_t b, uint64_t* out) {
    if (b != 0 && a > UINT64_MAX / b)
        return 0; 

    *out = a * b;
    return 1;
}

/*
 * Dispatcher (escolhe qual operação executar)
 * Por enquanto só '+'. Amanhã você adiciona '-', '*', '/' aqui.
 */

static int calc_exec_u64(uint64_t a, char op, uint64_t b, uint64_t* out) {
    switch (op) {
        case '+':
            return calc_add_u64(a, b, out);

        case '-':
            return calc_sub_u64(a, b, out);

        case '*':
            return calc_mul_u64(a, b, out);

        default:
            return 0;
    }
}

/* ============================================================================
 * 4) Função pública do comando: shice_cmd_calc
 * ============================================================================
 * Entrada esperada:
 *   calc <numero> + <numero>
 *
 * Exemplos:
 *   calc 10 + 10
 *   calc 0001 + 9
 *
 * Erros tratados:
 * - números inválidos
 * - mais de 20 dígitos
 * - caracteres sobrando no final
 * - overflow na soma
 */

void shice_cmd_calc(const char* line) {
    /* line começa com "calc", então pulamos 4 chars */
    const char* p = line + 4;

    uint64_t a = 0;
    uint64_t b = 0;
    uint64_t result = 0;
    char op = 0;

    /* 1) Pular espaços depois do comando */
    skip_spaces(&p);

    /* 2) Parse do primeiro número */
    if (!parse_u64_20digits(&p, &a)) {
        console_write("calc error: invalid first number (max 20 digits)");
        newline();
        return;
    }

    /* 3) Pular espaços e ler operador */
    skip_spaces(&p);
    op = *p;

    if (op == '\0') {
        console_write("calc error: missing operator");
        newline();
        return;
    }

    /* Por enquanto, só '+' */
    if (op != '+' && op != '-' && op != '*') {
        console_write("calc error: only '+', '-' and '*' is supported for now");
        newline();
        return;
    }

    /* Consome o operador */
    p++;

    /* 4) Pular espaços e parse do segundo número */
    skip_spaces(&p);
    if (!parse_u64_20digits(&p, &b)) {
        console_write("calc error: invalid second number (max 20 digits)");
        newline();
        return;
    }

    /* 5) Não pode sobrar lixo no final */
    skip_spaces(&p);
    if (*p != '\0') {
        console_write("calc error: unexpected characters after expression");
        newline();
        return;
    }

    /* 6) Executa a conta */
    if (!calc_exec_u64(a, op, b, &result)) {
        console_write("calc error: math error (overflow or unsupported op)");
        newline();
        return;
    }

    /* 7) Imprime resultado */
    print_u64(result);
    newline();
}
