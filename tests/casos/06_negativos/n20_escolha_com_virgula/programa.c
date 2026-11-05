#include <basicgol.h>

// ESCOLHA(expr) recebe UMA expressão: vírgula de nível superior não é permitida.
INICIO
    INTEIRO a = 1;
    INTEIRO b = 2;
    ESCOLHA (a, b)
        CASO 1:
            ESCREVAL("um");
            PARE;
    FIM_ESCOLHA
FIM
