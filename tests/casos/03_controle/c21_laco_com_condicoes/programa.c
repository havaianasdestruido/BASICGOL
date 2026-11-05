
// laco com varias condicoes, PARE e contadores
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO achados = 0;
    ENQUANTO VERDADEIRO FACA
        i = i + 1;
        SE i > 50 ENTAO
            PARE;
        FIM_SE
        SE i MOD 7 == 0 E i > 10 ENTAO
            achados = achados + 1;
        FIM_SE
    FIM_ENQUANTO
    ESCREVAL(i);        // 51
    ESCREVAL(achados);  // 14,21,28,35,42,49 = 6
FIM
