// Precedência: a de C, documentada em docs/06 (§4). Cada linha traz o
// agrupamento que o compilador aplica — nenhuma tabela própria.
#include <basicgol.h>

INICIO
    ESCREVAL(2 + 3 * 4);            // 14   = 2 + (3*4)
    ESCREVAL((2 + 3) * 4);          // 20
    ESCREVAL(10 - 2 - 3);           // 5    = (10-2)-3
    ESCREVAL(100 / 10 / 2);         // 5    = (100/10)/2
    ESCREVAL(7 MOD 2 + 1);          // 2    = (7%2)+1
    ESCREVAL(1 + 2 * 3 MOD 4);      // 3    = 1+((2*3)%4)
    ESCREVAL(-7 / 2 + 1);           // -2   = (-7/2)+1
    ESCREVAL(2 + 3 < 6);            // 1    = (2+3) < 6
    // E liga mais forte que OU. O compilador exige os parênteses (-Wparentheses),
    // que é a recomendação R-6 da especificação: escrever 1 E 0 OU 1 é a mesma
    // expressão, mas a suíte compila com -Werror e ensina a forma explícita.
    ESCREVAL((1 E 0) OU 1);         // 1
    ESCREVAL(1 OU (0 E 0));         // 1
    ESCREVAL(NAO 0 E 0);            // 0    = (!0) && 0
    ESCREVAL(NAO (0 E 0));          // 1
    ESCREVAL(1 << 3);               // 8    (bit a bit é C nativo)
    ESCREVAL(1 + 1 == 2);           // 1    = (1+1) == 2
    ESCREVAL((1 + 2) * (3 + 4));    // 21

    // atribuição é a mais fraca: comparação acontece antes
    INTEIRO x = 5;
    LOGICO igual = x == 5;
    ESCREVAL(igual);
    x = 5 + 1;
    ESCREVAL(x);

    // parênteses mudam o resultado, não a regra
    ESCREVAL(-(2 + 3));
    ESCREVAL((INTEIRO) 2.9 + 1);
    ESCREVAL((INTEIRO) (2.9 + 1));
FIM
