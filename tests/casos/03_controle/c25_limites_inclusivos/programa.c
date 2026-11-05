// Limites: PARA e PARA_PASSO são inclusivos nos dois extremos, quando o passo
// cai exatamente neles; o passo que não cai no fim para antes dele.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO n = 0;

    PARA (i, 4, 4) {                 // um único valor
        n = n + 1;
        ESCREVAL(i);                 // 4
    }

    PARA (i, 1, 3) {                 // extremos incluídos
        ESCREVA_FMT("%d ", i);
    }
    ESCREVA_LINHA();                 // 1 2 3

    PARA_PASSO (i, 1, 10, 4) {       // 10 não é alcançado: 1, 5, 9
        ESCREVA_FMT("%d ", i);
    }
    ESCREVA_LINHA();                 // 1 5 9

    PARA_PASSO (i, 0, 10, 5) {       // o fim cai no passo: 0, 5, 10
        ESCREVA_FMT("%d ", i);
    }
    ESCREVA_LINHA();                 // 0 5 10

    PARA_PASSO (i, 10, 0, -5) {      // descendo, o fim também é incluído
        ESCREVA_FMT("%d ", i);
    }
    ESCREVA_LINHA();                 // 10 5 0

    PARA (i, -2, 1) {                // faixa que cruza zero
        ESCREVA_FMT("%d ", i);
    }
    ESCREVA_LINHA();                 // -2 -1 0 1

    ESCREVAL(n);                     // 1
FIM
