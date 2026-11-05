// PARA_PASSO honra o sinal do passo: negativo conta para trás
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    PARA_PASSO (i, 5, 1, -2) {
        ESCREVA_FMT("%d ", i);
    }
    ESCREVA_LINHA();

    PARA_PASSO (i, 0, 4, 2) {
        ESCREVA_FMT("%d ", i);
    }
    ESCREVA_LINHA();

    // passo negativo que nunca alcança o limite: nenhuma iteração
    PARA_PASSO (i, 1, 5, -1) {
        ESCREVAL("nao deveria aparecer");
    }
    ESCREVAL("fim");
FIM
