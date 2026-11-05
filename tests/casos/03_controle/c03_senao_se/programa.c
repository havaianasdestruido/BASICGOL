
// cadeia de 4 ramos com SENAO_SE
#include <basicgol.h>

PROCEDIMENTO classifica(INTEIRO nota) {
    SE nota >= 90 ENTAO
        ESCREVA("A");
    SENAO_SE nota >= 70 ENTAO
        ESCREVA("B");
    SENAO_SE nota >= 50 ENTAO
        ESCREVA("C");
    SENAO
        ESCREVA("D");
    FIM_SE
    ESCREVA_LINHA();
}

INICIO
    classifica(95);
    classifica(70);
    classifica(69);
    classifica(50);
    classifica(0);
FIM
