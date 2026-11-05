// ENQUANTO é macro sem argumentos: a condição pode ter vírgula de nível
// superior (operador vírgula) sem quebrar a expansão. ESCOLHA e ATE_QUE, que
// são macros com argumento, exigem parênteses extras — ver caso 06 n19/n20.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO n = 0;
    ENQUANTO (i++, i < 4) FACA
        n = n + 1;
    FIM_ENQUANTO
    ESCREVAL(n);                     // 3
    ESCREVAL(i);                     // 4
FIM
