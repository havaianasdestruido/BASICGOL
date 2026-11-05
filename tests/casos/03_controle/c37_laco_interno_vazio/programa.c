// Um laço interno com zero iterações não encerra o externo: o corpo do externo
// roda normalmente a cada volta.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO j = 0;
    PARA (i, 1, 3) {
        PARA (j, 5, 4) {             // vazio
            ESCREVAL("nunca");
        }
        PARA_PASSO (j, 1, 3, -1) {   // vazio por passo negativo
            ESCREVAL("nunca");
        }
        ESCREVAL(i);
    }
FIM
