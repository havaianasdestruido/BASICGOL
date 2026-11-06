// primos.c — ENQUANTO, PARE, MOD e funcoes auxiliares.
// Perfil: M0 (macros).
// Compilar a partir da raiz: make exemplos
#include <basicgol.h>

// Testa se n e primo. 0 e 1 nao sao primos; 2 e o unico par primo.
FUNCAO LOGICO eh_primo(INTEIRO n) {
    INTEIRO d = 2;

    SE n < 2 ENTAO
        RETORNE FALSO;
    FIM_SE

    ENQUANTO d * d <= n FACA
        SE n MOD d == 0 ENTAO
            RETORNE FALSO;
        FIM_SE
        d = d + 1;
    FIM_ENQUANTO

    RETORNE VERDADEIRO;
}

INICIO
    INTEIRO limite = 0;
    INTEIRO n = 2;
    INTEIRO achados = 0;

    ESCREVA("Primos ate qual valor? ");
    SE NAO LEIA_OK(limite) ENTAO
        ERRO("entrada invalida");
    FIM_SE

    ENQUANTO n <= limite FACA
        SE eh_primo(n) ENTAO
            ESCREVA_FMT("%d ", n);
            achados = achados + 1;
            SE achados MOD 10 == 0 ENTAO
                ESCREVA_LINHA();
            FIM_SE
        FIM_SE
        n = n + 1;
    FIM_ENQUANTO

    ESCREVA_LINHA();
    ESCREVA("Total de primos: ");
    ESCREVAL(achados);

    // Exemplo de PARE e PROSSIGA: procurar o primeiro multiplo de 7 acima de 100.
    n = 101;
    ENQUANTO VERDADEIRO FACA
        SE n MOD 7 != 0 ENTAO
            n = n + 1;
            PROSSIGA;
        FIM_SE
        PARE;
    FIM_ENQUANTO
    ESCREVA("Primeiro multiplo de 7 acima de 100: ");
    ESCREVAL(n);
FIM
