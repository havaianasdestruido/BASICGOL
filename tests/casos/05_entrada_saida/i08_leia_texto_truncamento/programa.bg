
// linha maior que o buffer: devolve FALSO e NAO dessincroniza a proxima leitura
#include <basicgol.h>

INICIO
    TEXTO_FIXO(pequeno, 5);
    TEXTO_FIXO(depois, 16);

    SE LEIA_TEXTO(pequeno, (INTEIRO) sizeof pequeno) ENTAO
        ESCREVAL("coube");
    SENAO
        ESCREVAL("truncou");
    FIM_SE

    SE LEIA_TEXTO(depois, (INTEIRO) sizeof depois) ENTAO
        ESCREVA("proxima [");
        ESCREVA_TEXTO(depois);
        ESCREVAL("]");
    FIM_SE
FIM
