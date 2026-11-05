
// PROSSIGA pula os multiplos de 3
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO soma = 0;
    ENQUANTO i < 10 FACA
        i = i + 1;
        SE i MOD 3 == 0 ENTAO
            PROSSIGA;
        FIM_SE
        soma = soma + i;
    FIM_ENQUANTO
    ESCREVAL(soma);     // 1+2+4+5+7+8+10 = 37
FIM
