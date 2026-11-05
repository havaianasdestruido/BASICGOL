
#include <basicgol.h>

INICIO
    INTEIRO n = 0;
    REAL r = 0.0;
    CARACTERE c = '?';

    SE LEIA_OK(n) ENTAO
        ESCREVA("int ");
        ESCREVAL(n);
    FIM_SE
    SE LEIA_OK(r) ENTAO
        ESCREVA_FMT("real %.2f\n", r);
    FIM_SE
    SE LEIA_OK(c) ENTAO
        ESCREVA("char ");
        ESCREVAL(c);
    FIM_SE
FIM
