
// caso com variavel local (bloco C explicito)
#include <basicgol.h>

INICIO
    ESCOLHA (3)
        CASO 3: {
            INTEIRO quadrado = 3 * 3;
            ESCREVAL(quadrado);
            PARE;
        }
        CONTRARIO:
            ESCREVAL("outro");
    FIM_ESCOLHA
FIM
