
// um valor de cada tipo pela tabela de formatos
#include <basicgol.h>

INICIO
    LOGICO b = VERDADEIRO;
    CARACTERE c = 'Z';
    INTEIRO i = -42;
    NATURAL u = 4000000000u;
    INTEIRO_LONGO l = -1234567890L;
    INTEIRO64 g = 9000000000;
    REAL_CURTO f = 1.5f;
    REAL d = 2.25;
    REAL_LONGO ld = 3.5L;
    TEXTO s = "texto";

    ESCREVAL(b);
    ESCREVAL(c);
    ESCREVAL(i);
    ESCREVAL(u);
    ESCREVAL(l);
    ESCREVAL(g);
    ESCREVAL(f);
    ESCREVAL(d);
    ESCREVAL(ld);
    ESCREVAL(s);
    ESCREVAL(TAMANHO_VETOR("abcd") - 1);   /* so para exercitar size_t */
FIM
