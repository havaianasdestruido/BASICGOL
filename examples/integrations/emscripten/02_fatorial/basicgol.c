#include <basicgol.h>

FUNCAO INTEIRO bg_fatorial(INTEIRO n)
{
    INTEIRO resultado = 1;
    INTEIRO i = 0;
    SE n < 0 OU n > 12 ENTAO
        RETORNE -1;
    FIM_SE
    PARA (i, 2, n) {
        resultado = resultado * i;
    }
    RETORNE resultado;
}
