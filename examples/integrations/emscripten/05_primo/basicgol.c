#include <basicgol.h>

FUNCAO INTEIRO bg_e_primo(INTEIRO n)
{
    INTEIRO divisor = 2;
    SE n < 2 ENTAO
        RETORNE 0;
    FIM_SE
    ENQUANTO divisor <= n / divisor FACA
        SE n MOD divisor == 0 ENTAO
            RETORNE 0;
        FIM_SE
        divisor = divisor + 1;
    FIM_ENQUANTO
    RETORNE 1;
}
