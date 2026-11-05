
#include <basicgol.h>

ENUMERACAO(Cor)
    VERMELHO = 1,
    VERDE = 2,
    AZUL = 4
FIM_ENUMERACAO(Cor);

FUNCAO TEXTO nome_da(Cor c) {
    ESCOLHA (c)
        CASO VERMELHO:
            RETORNE "vermelho";
        CASO VERDE:
            RETORNE "verde";
        CASO AZUL:
            RETORNE "azul";
        CONTRARIO:
            RETORNE "desconhecida";
    FIM_ESCOLHA
    RETORNE "?";
}

INICIO
    Cor c = VERDE;
    ESCREVAL((INTEIRO) c);
    ESCREVAL(nome_da(c));
    ESCREVAL(nome_da(VERMELHO));
    ESCREVAL(nome_da((Cor) 99));
FIM
