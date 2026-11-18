#include <basicgol.h>
#include "api.h"
#include <limits.h>
#include <string.h>

FUNCAO INTEIRO bg_tamanho_em_bytes(TEXTO texto)
{
    size_t bytes = texto == NULL ? 0u : strlen(texto);
    RETORNE bytes > (size_t) INT_MAX ? INT_MAX : (INTEIRO) bytes;
}
