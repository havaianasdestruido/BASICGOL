// Passo sem sinal: a checagem do passo zero usa (p) == 0, não (p) < 0, para não
// acender -Wtype-limits (que é erro sob -Werror neste projeto).
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    NATURAL passo = 2;
    PARA_PASSO (i, 0, 6, passo) {
        ESCREVA_FMT("%d ", i);
    }
    ESCREVA_LINHA();
    PARA_PASSO (i, 6, 0, passo) {    // passo positivo sem alcançar o fim: vazio
        ESCREVAL("nunca");
    }
    ESCREVAL("fim");
FIM
