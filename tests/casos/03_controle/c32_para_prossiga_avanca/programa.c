// PROSSIGA dentro de PARA: a iteração é pulada, mas a variável de controle
// avança. Ao fim do laço ela vale fim + 1 (a definição do for de C).
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO vistos = 0;
    PARA (i, 1, 5) {
        SE i < 5 ENTAO
            PROSSIGA;
        FIM_SE
        vistos = vistos + 1;
    }
    ESCREVAL(vistos);                // 1 (só o 5)
    ESCREVAL(i);                     // 6
FIM
