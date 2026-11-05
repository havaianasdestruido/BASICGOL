
// queda intencional de um caso para o seguinte (sem PARE).
// Em C11 com -Wextra, queda intencional precisa ser anotada: /* fall through */
#include <basicgol.h>

INICIO
    ESCOLHA (2)
        CASO 1:
            ESCREVA("um ");
            /* fall through */
        CASO 2:
            ESCREVA("dois ");
            /* fall through */
        CASO 3:
            ESCREVA("tres ");
            PARE;
        CONTRARIO:
            ESCREVA("outro ");
    FIM_ESCOLHA
    ESCREVA_LINHA();
FIM
