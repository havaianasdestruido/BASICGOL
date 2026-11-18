#include <basicgol.h>

FUNCAO INTEIRO bg_limita(INTEIRO valor, INTEIRO minimo, INTEIRO maximo)
{
    SE valor < minimo ENTAO
        RETORNE minimo;
    FIM_SE
    SE valor > maximo ENTAO
        RETORNE maximo;
    FIM_SE
    RETORNE valor;
}
