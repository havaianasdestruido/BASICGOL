// LEIA_INTEIRO / LEIA_REAL: uma linha por número, sem lixo no stdin
#include <basicgol.h>

INICIO
    INTEIRO n = 0;
    REAL r = 0.0;

    SE LEIA_INTEIRO(n) ENTAO
        ESCREVAL(n);
    SENAO
        ESCREVAL("inteiro 1 falhou");
    FIM_SE

    // a linha inválida é consumida: a próxima leitura começa na linha seguinte
    SE LEIA_INTEIRO(n) ENTAO
        ESCREVAL(n);
    SENAO
        ESCREVAL("inteiro 2 falhou");
    FIM_SE

    SE LEIA_INTEIRO(n) ENTAO
        ESCREVAL(n);
    SENAO
        ESCREVAL("inteiro 3 falhou");
    FIM_SE

    SE LEIA_REAL(r) ENTAO
        ESCREVA_FMT("%.1f\n", r);
    SENAO
        ESCREVAL("real 1 falhou");
    FIM_SE

    SE LEIA_REAL(r) ENTAO        // "2,5": locale C recusa a vírgula
        ESCREVA_FMT("%.1f\n", r);
    SENAO
        ESCREVAL("real 2 falhou");
    FIM_SE

    SE LEIA_REAL(r) ENTAO        // "1e999": estouro de faixa
        ESCREVA_FMT("%.1f\n", r);
    SENAO
        ESCREVAL("real 3 falhou");
    FIM_SE

    SE LEIA_INTEIRO(n) ENTAO
        ESCREVAL(n);
    SENAO
        ESCREVAL("inteiro 4 falhou");
    FIM_SE
FIM
