// controle_para_acucar.c — forma com acucar PARA i DE ... FACA, so no perfil M1.
// Perfil: M1 (exige o transpilador; nao compila so com macros).
// Compilar: build/basicgolc controle_para_acucar.c -o controle_para_acucar.gerado.c
//           cc -std=c11 -Wall -Wextra controle_para_acucar.gerado.c -o controle_para_acucar -I ../../include
// No perfil M0 use PARA (i, a, b) { ... } com as chaves de C (ver docs/07).
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO a = 0;
    INTEIRO b = 0;
    INTEIRO x = 0;

    PARA i DE 1 ATE 5 FACA
        ESCREVA_FMT("%d ", i);
    FIM_PARA
    ESCREVA_LINHA();                 // 1 2 3 4 5

    PARA i DE 10 ATE 1 PASSO -3 FACA
        ESCREVA_FMT("%d ", i);
    FIM_PARA
    ESCREVA_LINHA();                 // 10 7 4 1

    PARA i DE 5 ATE 1 FACA           // zero iteracoes
        ESCREVAL("nunca");
    FIM_PARA

    PARA a DE 1 ATE 2 FACA           // aninhado
        PARA b DE 1 ATE 2 FACA
            ESCREVA_FMT("%d%d ", a, b);
        FIM_PARA
    FIM_PARA
    ESCREVA_LINHA();                 // 11 12 21 22

    REPITA                           // acucar do REPITA: ATE cond;
        x = x + 1;
    ATE x >= 3;
    ESCREVAL(x);                     // 3
FIM
