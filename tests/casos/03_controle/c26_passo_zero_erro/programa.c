// Passo zero em tempo de execução: o laço recusa com a linha do aluno, em vez
// de girar para sempre. (Passo literal 0 é recusado já pelo transpilador.)
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO passo = 0;
    ESCREVAL("antes");
    PARA_PASSO (i, 1, 10, passo) {
        ESCREVAL("nao deveria aparecer");
    }
    ESCREVAL("depois");
FIM
