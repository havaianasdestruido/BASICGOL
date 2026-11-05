// A Fase 2 mostrou que a vírgula dentro de parênteses NÃO quebra a macro:
// "ESCOLHA(f(a, b))" é um único argumento (correção de C-04).
#include <basicgol.h>

FUNCAO INTEIRO soma(INTEIRO a, INTEIRO b) { RETORNE a + b; }

INICIO
    ESCOLHA (soma(1, 2))
        CASO 3:
            ESCREVAL("tres");
            PARE;
        CONTRARIO:
            ESCREVAL("outro");
    FIM_ESCOLHA

    ESCOLHA (soma(2, 2) * 2)
        CASO 8:
            ESCREVAL("oito");
            PARE;
        CONTRARIO:
            ESCREVAL("outro");
    FIM_ESCOLHA
FIM
