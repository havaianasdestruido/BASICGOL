// Zero iterações: cada laço de controle, com a condição falsa desde o início,
// não executa o corpo nem uma vez. REPITA é a exceção: executa ao menos uma.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO n = 0;

    PARA (i, 5, 1) {                 // fim < inicio
        n = n + 1;
    }
    ESCREVAL(n);                     // 0

    PARA_PASSO (i, 1, 5, -1) {       // passo negativo sem alcançar o fim
        n = n + 1;
    }
    ESCREVAL(n);                     // 0

    ENQUANTO n > 100 FACA            // condição falsa logo na entrada
        n = n + 1;
    FIM_ENQUANTO
    ESCREVAL(n);                     // 0

    INTEIRO um = 1;
    REPITA                           // testa depois: roda uma vez
        n = n + 1;
    ATE_QUE(um == 1);
    ESCREVAL(n);                     // 1
FIM
