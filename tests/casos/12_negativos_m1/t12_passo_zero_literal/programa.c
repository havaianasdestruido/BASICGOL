#include <basicgol.h>

// PASSO 0 literal nunca avança: o transpilador recusa antes de gerar o laço.
INICIO
    INTEIRO i = 0;
    PARA i DE 1 ATE 3 PASSO 0 FACA
        ESCREVAL(i);
    FIM_PARA
FIM
