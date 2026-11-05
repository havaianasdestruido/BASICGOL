#include <basicgol.h>

// ATE_QUE(cond) e um do/while: o ';' e obrigatorio logo depois do ')'.
INICIO
    INTEIRO a = 0;
    REPITA
        a = a + 1;
    ATE_QUE(a > 3)
    ESCREVAL(a);
FIM
