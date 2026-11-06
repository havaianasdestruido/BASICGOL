// DIVIDE/RESTO: mesmas regras de C, mas sem divisor zero nem MIN/-1
#include <basicgol.h>

INICIO
    ESCREVAL(DIVIDE(7, 2));
    ESCREVAL(DIVIDE(-7, 2));
    ESCREVAL(DIVIDE(7, -2));
    ESCREVAL(RESTO(7, 2));
    ESCREVAL(RESTO(-7, 2));
    ESCREVAL(RESTO(7, -2));

    // o tipo do resultado é o tipo promovido dos operandos
    ESCREVAL(DIVIDE(7L, 2L));
    ESCREVAL(DIVIDE(7u, 2u));
    ESCREVAL(DIVIDE(7LL, 2LL));
    ESCREVAL(RESTO(7u, 2u));
    ESCREVA_FMT("%.2f\n", DIVIDE(7.0, 2.0));
    ESCREVA_FMT("%.2f\n", DIVIDE(-7.0, 2.0));

    // MINIMO_INTEIRO / 1 é válido; / -1 é indefinido se o quociente não couber (caso t10)
    ESCREVAL(DIVIDE(MINIMO_INTEIRO, 1) == MINIMO_INTEIRO);
    ESCREVAL(RESTO(MINIMO_INTEIRO, 2) == 0);

    // misturar sinal e natural converte para natural, como em C:
    // DIVIDE(-1, 1u) é (NATURAL)-1 / 1, não -1. É o preço de ser C.
    ESCREVAL(DIVIDE(-1, 1u) == MAXIMO_NATURAL);
FIM
