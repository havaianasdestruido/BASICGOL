
// SE dentro de SE dentro de SE
#include <basicgol.h>

INICIO
    INTEIRO a = 1;
    INTEIRO b = 2;
    INTEIRO c = 3;

    SE a == 1 ENTAO
        SE b == 2 ENTAO
            SE c == 3 ENTAO
                ESCREVAL("tres niveis");
            SENAO
                ESCREVAL("c errado");
            FIM_SE
        SENAO
            ESCREVAL("b errado");
        FIM_SE
    SENAO
        ESCREVAL("a errado");
    FIM_SE
FIM
