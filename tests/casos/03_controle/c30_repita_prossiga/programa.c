// PROSSIGA dentro de REPITA: pula para o teste ATE_QUE, que continua sendo
// feito. Se o teste fosse pulado, este laço giraria para sempre.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO pares = 0;
    REPITA
        i = i + 1;
        SE i MOD 2 == 1 ENTAO
            PROSSIGA;
        FIM_SE
        pares = pares + 1;
    ATE_QUE(i >= 6);
    ESCREVAL(i);                     // 6
    ESCREVAL(pares);                 // 3
FIM
