// Tipos reais: precisão binária, comparação com tolerância, NaN e infinito
#include <basicgol.h>

INICIO
    REAL_CURTO f = 1.5f;
    REAL d = 1.5;
    REAL_LONGO ld = 1.5L;

    ESCREVAL(sizeof(REAL) >= sizeof(REAL_CURTO));
    ESCREVAL(sizeof(REAL_LONGO) >= sizeof(REAL));
    ESCREVAL(TAMANHO_DE(REAL) >= 4);

    // 0.1 + 0.2 nao e 0.3 em binario: comparar com tolerancia, nao com ==
    REAL soma = 0.1 + 0.2;
    ESCREVAL(soma == 0.3);
    ESCREVAL(ABS(soma - 0.3) < EPSILON_REAL);
    ESCREVA_FMT("%.2f\n", soma);

    // impressão: %g usa 6 algarismos significativos
    ESCREVA_FMT("%.3f\n", 1.0 / 3.0);
    ESCREVA_FMT("%.0f\n", 2.6);
    ESCREVA_FMT("%.0f\n", -2.6);

    ESCREVAL(f);
    ESCREVAL(d);
    ESCREVAL(ld);

    // NaN nao e igual a nada, nem a si mesmo; infinito e maior que tudo
    ESCREVAL(NAN == NAN);
    ESCREVAL(NAN != NAN);
    ESCREVAL(INFINITY > MAXIMO_REAL);
    ESCREVAL(isnan(NAN));
    ESCREVAL(isinf(INFINITY));

    // conversão explícita para inteiro trunca em direção a zero
    ESCREVAL((INTEIRO) 3.9);
    ESCREVAL((INTEIRO) -3.9);

    // ABS/MINIMO/MAXIMO preservam o tipo real
    ESCREVA_FMT("%.1f\n", ABS(-2.5));
    ESCREVA_FMT("%.1f\n", MINIMO(2.5, 3.5));
    ESCREVA_FMT("%.1f\n", MAXIMO(2.5, 3.5));
FIM
