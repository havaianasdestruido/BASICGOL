
#include <basicgol.h>

PROCEDIMENTO menu(INTEIRO opcao) {
    ESCOLHA (opcao)
        CASO 1:
            ESCREVAL("novo");
            PARE;
        CASO 2:
            ESCREVAL("abrir");
            PARE;
        CONTRARIO:
            ESCREVAL("invalido");
    FIM_ESCOLHA
}

INICIO
    menu(1);
    menu(2);
    menu(9);
FIM
