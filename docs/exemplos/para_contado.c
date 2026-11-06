// para_contado.c — forma acucarada do laco contado.
// Perfil: M1 (exige basicgolc). O caminho M0 usa PARA (i, a, b) { ... }.
// Compilar a partir da raiz: make exemplos
#include <basicgol.h>

INICIO
    INTEIRO i = 0;

    ESCREVAL("Contagem crescente:");
    PARA i DE 1 ATE 5 FACA
        ESCREVA_FMT("%d ", i);
    FIM_PARA
    ESCREVA_LINHA();

    ESCREVAL("Contagem decrescente:");
    PARA i DE 5 ATE 1 PASSO -1 FACA
        ESCREVA_FMT("%d ", i);
    FIM_PARA
    ESCREVA_LINHA();

    ESCREVAL("Pares de 0 a 10:");
    PARA i DE 0 ATE 10 PASSO 2 FACA
        ESCREVA_FMT("%d ", i);
    FIM_PARA
    ESCREVA_LINHA();

    // Forma equivalente, valida tambem no caminho M0:
    ESCREVAL("Mesma coisa sem acucar:");
    PARA (i, 0, 10) {
        SE i MOD 2 == 0 ENTAO
            ESCREVA_FMT("%d ", i);
        FIM_SE
    }
    ESCREVA_LINHA();
FIM
