// ENQUANTO com PROSSIGA: a condição é reavaliada e o contador já foi
// incrementado antes do PROSSIGA, então o laço termina.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO soma = 0;
    ENQUANTO i < 10 FACA
        i = i + 1;
        SE i == 5 ENTAO
            PROSSIGA;                // o 5 não entra na soma
        FIM_SE
        soma = soma + i;
    FIM_ENQUANTO
    ESCREVAL(soma);                  // 1..10 = 55, menos 5 = 50
FIM
