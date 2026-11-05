// SE aninhado dentro de SENAO: o M1 recusa "SENAO" seguido de "SE" (C-05), então
// coloca-se um comando antes do SE aninhado. Este caso fixa esse padrão nos dois caminhos.
#include <basicgol.h>

INICIO
    INTEIRO a = 0;
    INTEIRO b = 1;
    SE a > 0 ENTAO
        ESCREVAL("pos");
    SENAO
        ESCREVAL("nao positivo");
        SE b > 0 ENTAO
            ESCREVAL("b positivo");
        FIM_SE
    FIM_SE
FIM
