// Cadeia SENAO_SE com limites de faixa e OU; o primeiro ramo verdadeiro vence.
#include <basicgol.h>

INICIO
    INTEIRO nota = 0;
    PARA (nota, 4, 10) {
        ESCREVA_FMT("%d: ", nota);
        SE nota < 5 ENTAO
            ESCREVAL("reprovado");
        SENAO_SE nota < 7 ENTAO
            ESCREVAL("recuperacao");
        SENAO_SE nota == 9 OU nota == 10 ENTAO
            ESCREVAL("excelente");
        SENAO
            ESCREVAL("aprovado");
        FIM_SE
    }
FIM
