
// C-20 (o outro lado): sem nada pendente, LEIA_TEXTO nao pode engolir a linha
#include <basicgol.h>

INICIO
    TEXTO_FIXO(linha, 32);
    SE LEIA_TEXTO(linha, (INTEIRO) sizeof linha) ENTAO
        ESCREVA("[");
        ESCREVA_TEXTO(linha);
        ESCREVAL("]");
    FIM_SE
FIM
