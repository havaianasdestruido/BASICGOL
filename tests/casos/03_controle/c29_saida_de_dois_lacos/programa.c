// BASICGOL não tem PARE com rótulo: para sair de DOIS laços usa-se uma
// bandeira. Este caso fixa o padrão e a ordem da busca (i externo, j interno).
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO j = 0;
    INTEIRO achou = 0;
    INTEIRO ai = 0;
    INTEIRO aj = 0;

    PARA (i, 1, 4) {
        PARA (j, 1, 4) {
            SE i * j == 6 ENTAO
                achou = 1;
                ai = i;
                aj = j;
                PARE;
            FIM_SE
        }
        SE achou ENTAO
            PARE;
        FIM_SE
    }
    ESCREVAL(ai);                    // 2 (primeiro par com produto 6)
    ESCREVAL(aj);                    // 3

    // sem par que dê 7: a bandeira nunca liga e os dois laços terminam sozinhos
    achou = 0;
    PARA (i, 1, 4) {
        PARA (j, 1, 4) {
            SE i * j == 7 ENTAO
                achou = 1;
                PARE;
            FIM_SE
        }
        SE achou ENTAO
            PARE;
        FIM_SE
    }
    ESCREVAL(achou);                 // 0
FIM
