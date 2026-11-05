// fatorial_recursivo.c — recursao, INTEIRO64 e formatacao de inteiros largos.
// Perfil: M0 (macros).
// Compilar: cc -std=c11 -Wall -Wextra fatorial_recursivo.c -o fatorial
#include <basicgol.h>

// Fatorial recursivo com tipo de largura exata (estoura em 21!).
FUNCAO INTEIRO64 fatorial(INTEIRO n) {
    SE n <= 1 ENTAO
        RETORNE 1;
    FIM_SE
    RETORNE (INTEIRO64) n * fatorial(n - 1);
}

// Fibonacci iterativo: mostra que nem tudo precisa ser recursivo.
FUNCAO INTEIRO64 fibonacci(INTEIRO n) {
    INTEIRO64 a = 0;
    INTEIRO64 b = 1;
    INTEIRO64 proximo = 0;
    INTEIRO i = 0;

    SE n <= 0 ENTAO
        RETORNE 0;
    FIM_SE

    PARA (i, 1, n - 1) {
        proximo = a + b;
        a = b;
        b = proximo;
    }
    RETORNE b;
}

INICIO
    INTEIRO i = 0;

    PARA (i, 0, 10) {
        ESCREVA_FMT("fatorial(%2d) = %lld\n", i, (long long) fatorial(i));
    }

    ESCREVA_LINHA();
    PARA (i, 0, 10) {
        ESCREVA_FMT("fib(%2d) = %lld\n", i, (long long) fibonacci(i));
    }
FIM
