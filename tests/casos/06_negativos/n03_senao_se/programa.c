#include <basicgol.h>

// "SENAO SE" nao encadeia: o C ve "} else { if (...) { ... }" e fica um '{' aberto
INICIO
    SE 1 ENTAO
        ESCREVAL("a");
    SENAO SE 2 ENTAO
        ESCREVAL("b");
    FIM_SE
FIM
