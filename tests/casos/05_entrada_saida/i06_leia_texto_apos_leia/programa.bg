
// C-20: depois de LEIA, o '\n' pendente nao pode estragar LEIA_TEXTO
#include <basicgol.h>

INICIO
    INTEIRO idade = 0;
    TEXTO_FIXO(nome, 32);

    SE LEIA_OK(idade) ENTAO
        ESCREVA("idade ");
        ESCREVAL(idade);
    FIM_SE
    SE LEIA_TEXTO(nome, (INTEIRO) sizeof nome) ENTAO
        ESCREVA("nome [");
        ESCREVA_TEXTO(nome);
        ESCREVAL("]");
    FIM_SE
FIM
