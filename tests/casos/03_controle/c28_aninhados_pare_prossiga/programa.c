// PARE e PROSSIGA atuam só sobre o laço MAIS INTERNO que os contém.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO j = 0;
    INTEIRO total = 0;

    // PARE: o laço interno para em j == 3 (j = 1 e 2 contam); o externo segue.
    PARA (i, 1, 3) {
        PARA (j, 1, 5) {
            SE j == 3 ENTAO
                PARE;
            FIM_SE
            total = total + 1;
        }
        total = total + 100;
    }
    ESCREVAL(total);                 // 3*2 + 3*100 = 306

    // PROSSIGA: pula só o j par, dentro de cada i; o +10 externo roda 3 vezes.
    total = 0;
    PARA (i, 1, 3) {
        PARA (j, 1, 4) {
            SE j MOD 2 == 0 ENTAO
                PROSSIGA;
            FIM_SE
            total = total + 1;       // j = 1 e 3
        }
        total = total + 10;
    }
    ESCREVAL(total);                 // 3*2 + 3*10 = 36
FIM
