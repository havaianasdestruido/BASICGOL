#include <basicgol.h>
#include "api.h"

FUNCAO INTEIRO bg_aplica_callback(INTEIRO valor, bg_callback callback)
{
    SE callback == NULL ENTAO
        RETORNE 0;
    FIM_SE
    RETORNE callback(valor);
}
