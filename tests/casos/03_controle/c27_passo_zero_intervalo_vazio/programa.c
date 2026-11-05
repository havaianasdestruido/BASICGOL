// Passo zero é erro mesmo com intervalo vazio: o problema é o passo, não o
// intervalo. Sem a checagem, PARA_PASSO(i, 5, 1, 0) entraria num laço infinito.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO passo = 0;
    ESCREVAL("antes");
    PARA_PASSO (i, 5, 1, passo) {
        ESCREVAL("nao deveria aparecer");
    }
    ESCREVAL("depois");
FIM
