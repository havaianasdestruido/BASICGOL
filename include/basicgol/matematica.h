/*
 * basicgol/matematica.h — operações matemáticas usadas pela sintaxe BASICGOL.
 * As rotinas de potência, raiz, piso, teto e arredondamento usam libm.
 */
#ifndef BASICGOL_MATEMATICA_H
#define BASICGOL_MATEMATICA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Divisão inteira/real verificada. Divisor zero e divisão assinada cujo
 * quociente não cabe (em geral MIN/-1) chamam bg_erro com arquivo/linha. */
int bg_divide_i(int a, int b, const char *arquivo, int linha);
long bg_divide_l(long a, long b, const char *arquivo, int linha);
long long bg_divide_ll(long long a, long long b, const char *arquivo, int linha);
unsigned bg_divide_u(unsigned a, unsigned b, const char *arquivo, int linha);
unsigned long bg_divide_ul(unsigned long a, unsigned long b,
                           const char *arquivo, int linha);
unsigned long long bg_divide_ull(unsigned long long a, unsigned long long b,
                                 const char *arquivo, int linha);
float bg_divide_f(float a, float b, const char *arquivo, int linha);
double bg_divide_d(double a, double b, const char *arquivo, int linha);
long double bg_divide_ld(long double a, long double b,
                         const char *arquivo, int linha);

/* Resto inteiro verificado; zero e quociente a/b não representável são fatais. */
int bg_resto_i(int a, int b, const char *arquivo, int linha);
long bg_resto_l(long a, long b, const char *arquivo, int linha);
long long bg_resto_ll(long long a, long long b, const char *arquivo, int linha);
unsigned bg_resto_u(unsigned a, unsigned b, const char *arquivo, int linha);
unsigned long bg_resto_ul(unsigned long a, unsigned long b,
                          const char *arquivo, int linha);
unsigned long long bg_resto_ull(unsigned long long a, unsigned long long b,
                                const char *arquivo, int linha);

/* Primitivas escolhidas por ABS/MINIMO/MAXIMO. ABS é fatal somente quando o
 * resultado positivo não cabe no tipo assinado (normalmente no mínimo). */
int bg_abs_i(int valor);
long bg_abs_l(long valor);
long long bg_abs_ll(long long valor);
float bg_abs_f(float valor);
double bg_abs_d(double valor);
int bg_min_i(int a, int b);
long bg_min_l(long a, long b);
double bg_min_d(double a, double b);
int bg_max_i(int a, int b);
long bg_max_l(long a, long b);
double bg_max_d(double a, double b);
long long bg_round_l(double valor);

#ifdef __cplusplus
}
#endif

#endif /* BASICGOL_MATEMATICA_H */
