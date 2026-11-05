
#include <basicgol.h>

REGISTRO(Ponto)
    REAL x;
    REAL y;
FIM_REGISTRO(Ponto);

PROCEDIMENTO imprime(Ponto p) {
    ESCREVA("(");
    ESCREVA_FMT("%.1f", p.x);
    ESCREVA(", ");
    ESCREVA_FMT("%.1f", p.y);
    ESCREVAL(")");
}

INICIO
    Ponto p;
    p.x = 1.5;
    p.y = -2.0;
    imprime(p);
    ESCREVA_FMT("%.1f\n", p.x + p.y);
FIM
