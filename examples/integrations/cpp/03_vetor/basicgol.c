#include <basicgol.h>
#include "api.h"

FUNCAO INTEIRO bg_soma_vetor(const INTEIRO valores[], INTEIRO quantidade)
{
    INTEIRO soma = 0;
    INTEIRO i = 0;
    PARA (i, 0, quantidade - 1) {
        soma = soma + valores[i];
    }
    RETORNE soma;
}
