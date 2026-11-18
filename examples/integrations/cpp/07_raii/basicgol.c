#include <basicgol.h>
#include "api.h"
#include <stdlib.h>
#include <string.h>

FUNCAO CARACTERE *bg_duplica_texto(TEXTO texto)
{
    size_t tamanho = texto == NULL ? 0u : strlen(texto);
    CARACTERE *copia = (CARACTERE *) malloc(tamanho + 1u);
    SE copia == NULL ENTAO
        RETORNE NULL;
    FIM_SE
    SE tamanho != 0u ENTAO
        memcpy(copia, texto, tamanho);
    FIM_SE
    copia[tamanho] = '\0';
    RETORNE copia;
}

FUNCAO VAZIO bg_libera_texto(CARACTERE *texto)
{
    free(texto);
}
