
#include <basicgol.h>

INICIO
    TEXTO_FIXO(nome, 16);
    TEXTO_FIXO(saudacao, 32);

    COPIA_TEXTO(nome, (INTEIRO) sizeof nome, "mundo");
    ESCREVAL(nome);

    JUNTA_TEXTO(saudacao, (INTEIRO) sizeof saudacao, "ola, ", nome);
    ESCREVAL(saudacao);
    ESCREVAL(TAMANHO_TEXTO(saudacao));
FIM
