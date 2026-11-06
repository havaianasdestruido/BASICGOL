/*
 * matemática.c — implementação das primitivas matemáticas de BASICGOL.
 *
 * As operações comuns são encaminhadas a <math.h>. Os únicos diagnósticos
 * fatais são casos em que as operações inteiras de C teriam comportamento
 * indefinido ou em que o resultado de ABS não cabe no tipo de retorno.
 */
#include <basicgol/consola.h>
#include <basicgol/matematica.h>

#include <limits.h>
#include <math.h>

int bg_divide_i(int a, int b, const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "divisao por zero");
    }
    if (a == INT_MIN && b == -1 && INT_MIN < -INT_MAX) {
        bg_erro(arquivo, linha, "estouro em divisao inteira (INT_MIN / -1)");
    }
    return a / b;
}

long bg_divide_l(long a, long b, const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "divisao por zero");
    }
    if (a == LONG_MIN && b == -1 && LONG_MIN < -LONG_MAX) {
        bg_erro(arquivo, linha, "estouro em divisao inteira (LONG_MIN / -1)");
    }
    return a / b;
}

long long bg_divide_ll(long long a, long long b, const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "divisao por zero");
    }
    if (a == LLONG_MIN && b == -1 && LLONG_MIN < -LLONG_MAX) {
        bg_erro(arquivo, linha, "estouro em divisao inteira (LLONG_MIN / -1)");
    }
    return a / b;
}

unsigned bg_divide_u(unsigned a, unsigned b, const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "divisao por zero");
    }
    return a / b;
}

unsigned long bg_divide_ul(unsigned long a, unsigned long b,
                           const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "divisao por zero");
    }
    return a / b;
}

unsigned long long bg_divide_ull(unsigned long long a, unsigned long long b,
                                 const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "divisao por zero");
    }
    return a / b;
}

float bg_divide_f(float a, float b, const char *arquivo, int linha)
{
    if (b == 0.0f) {
        bg_erro(arquivo, linha, "divisao por zero (real)");
    }
    return a / b;
}

double bg_divide_d(double a, double b, const char *arquivo, int linha)
{
    if (b == 0.0) {
        bg_erro(arquivo, linha, "divisao por zero (real)");
    }
    return a / b;
}

long double bg_divide_ld(long double a, long double b,
                         const char *arquivo, int linha)
{
    if (b == 0.0L) {
        bg_erro(arquivo, linha, "divisao por zero (real)");
    }
    return a / b;
}

int bg_resto_i(int a, int b, const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "resto por zero");
    }
    if (a == INT_MIN && b == -1 && INT_MIN < -INT_MAX) {
        bg_erro(arquivo, linha, "estouro em resto inteiro (INT_MIN % -1)");
    }
    return a % b;
}

long bg_resto_l(long a, long b, const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "resto por zero");
    }
    if (a == LONG_MIN && b == -1 && LONG_MIN < -LONG_MAX) {
        bg_erro(arquivo, linha, "estouro em resto inteiro (LONG_MIN % -1)");
    }
    return a % b;
}

long long bg_resto_ll(long long a, long long b, const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "resto por zero");
    }
    if (a == LLONG_MIN && b == -1 && LLONG_MIN < -LLONG_MAX) {
        bg_erro(arquivo, linha, "estouro em resto inteiro (LLONG_MIN % -1)");
    }
    return a % b;
}

unsigned bg_resto_u(unsigned a, unsigned b, const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "resto por zero");
    }
    return a % b;
}

unsigned long bg_resto_ul(unsigned long a, unsigned long b,
                          const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "resto por zero");
    }
    return a % b;
}

unsigned long long bg_resto_ull(unsigned long long a, unsigned long long b,
                                const char *arquivo, int linha)
{
    if (b == 0) {
        bg_erro(arquivo, linha, "resto por zero");
    }
    return a % b;
}

int bg_abs_i(int valor)
{
    unsigned magnitude;

    if (valor >= 0) {
        return valor;
    }
    /* -(valor+1) é sempre representável; a magnitude é calculada sem UB. */
    magnitude = (unsigned) (-(valor + 1)) + 1U;
    if (magnitude > (unsigned) INT_MAX) {
        bg_erro("matematica", 0, "ABS: valor absoluto de INT_MIN nao cabe em int");
    }
    return (int) magnitude;
}

long bg_abs_l(long valor)
{
    unsigned long magnitude;

    if (valor >= 0L) {
        return valor;
    }
    magnitude = (unsigned long) (-(valor + 1L)) + 1UL;
    if (magnitude > (unsigned long) LONG_MAX) {
        bg_erro("matematica", 0, "ABS: valor absoluto de LONG_MIN nao cabe em long");
    }
    return (long) magnitude;
}

long long bg_abs_ll(long long valor)
{
    unsigned long long magnitude;

    if (valor >= 0LL) {
        return valor;
    }
    magnitude = (unsigned long long) (-(valor + 1LL)) + 1ULL;
    if (magnitude > (unsigned long long) LLONG_MAX) {
        bg_erro("matematica", 0,
                "ABS: valor absoluto de LLONG_MIN nao cabe em long long");
    }
    return (long long) magnitude;
}

float bg_abs_f(float valor)
{
    return fabsf(valor);
}

double bg_abs_d(double valor)
{
    return fabs(valor);
}

int bg_min_i(int a, int b)
{
    return (a < b) ? a : b;
}

long bg_min_l(long a, long b)
{
    return (a < b) ? a : b;
}

double bg_min_d(double a, double b)
{
    return fmin(a, b);
}

int bg_max_i(int a, int b)
{
    return (a > b) ? a : b;
}

long bg_max_l(long a, long b)
{
    return (a > b) ? a : b;
}

double bg_max_d(double a, double b)
{
    return fmax(a, b);
}

long long bg_round_l(double valor)
{
    return llround(valor);
}
