
#include <basicgol.h>

FUNCAO INTEIRO64 fatorial(INTEIRO n) {
    SE n <= 1 ENTAO
        RETORNE 1;
    FIM_SE
    RETORNE (INTEIRO64) n * fatorial(n - 1);
}

FUNCAO INTEIRO fib(INTEIRO n) {
    SE n < 2 ENTAO
        RETORNE n;
    FIM_SE
    RETORNE fib(n - 1) + fib(n - 2);
}

INICIO
    ESCREVAL((long long) fatorial(5));
    ESCREVAL((long long) fatorial(10));
    ESCREVAL(fib(10));
    ESCREVAL(fib(15));
FIM
