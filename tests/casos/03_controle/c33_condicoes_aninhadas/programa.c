// Condições aninhadas. Não há "dangling else": cada SE pede o seu FIM_SE, então
// cada SENAO fecha o SE que ele está dentro, e nunca o de dentro por engano.
#include <basicgol.h>

INICIO
    INTEIRO a = 0;
    INTEIRO b = 0;

    PARA (a, 0, 1) {
        PARA (b, 0, 1) {
            SE a == 1 ENTAO
                SE b == 1 ENTAO
                    ESCREVAL("a e b");
                SENAO
                    ESCREVAL("so a");        // SENAO do SE interno (b)
                FIM_SE
            SENAO
                ESCREVAL("nem a");           // SENAO do SE externo (a)
            FIM_SE
        }
    }
FIM
