#include <basicgol.h>

// ATE_QUE é macro com argumento: vírgula de nível superior quebra a chamada.
// Forma correta: ATE_QUE((a, b));
INICIO
    INTEIRO a = 0;
    INTEIRO b = 0;
    REPITA
        a = a + 1;
    ATE_QUE(a = a + 1, b > 2);
FIM
