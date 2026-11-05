
// caracteres, escapes e comparacao
#include <basicgol.h>

INICIO
    CARACTERE letra = 'B';
    CARACTERE quebra = '\n';
    TEXTO nome = "BASICGOL";

    ESCREVAL(letra);
    ESCREVAL((INTEIRO) letra);
    ESCREVAL((INTEIRO) 'A');
    ESCREVAL((INTEIRO) quebra);
    ESCREVAL(TAMANHO_TEXTO(nome));
    ESCREVAL(nome[0]);
    ESCREVAL(nome[7]);
    SE letra == 'B' ENTAO
        ESCREVAL("letra ok");
    FIM_SE
FIM
