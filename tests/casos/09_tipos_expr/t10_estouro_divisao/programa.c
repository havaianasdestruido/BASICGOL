// MINIMO_INTEIRO / -1 estoura o int: DIVIDE acusa antes da UB
#include <basicgol.h>

INICIO
    INTEIRO minimo = MINIMO_INTEIRO;
    INTEIRO c = DIVIDE(minimo, -1);
    ESCREVAL("nao deveria aparecer");
    ESCREVAL(c);
FIM
