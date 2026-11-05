// CARACTERE: um byte, promoção para INTEIRO nas contas, helpers ASCII
#include <basicgol.h>

INICIO
    CARACTERE a = 'A';
    CARACTERE z = 'Z';

    ESCREVAL(a);
    ESCREVAL(z);
    ESCREVAL(a + 1);                 // promoção: 66, e não o caractere 'B'
    ESCREVA(a + 2); ESCREVA_LINHA(); // 67 -> 'C'
    ESCREVAL((CARACTERE) (a + 1) == 'B');
    ESCREVAL(z - a);                 // distância na tabela: 25

    // escapes de C
    ESCREVAL('\n' == 10);
    ESCREVAL('\t' == 9);
    ESCREVAL('\'' == 39);
    ESCREVAL('\\' == 92);
    ESCREVAL('\0' == 0);
    ESCREVAL('\x41' == 'A');
    ESCREVAL('\101' == 'A');

    // helpers ASCII: exatos e independentes de locale
    ESCREVAL(E_DIGITO('7'));
    ESCREVAL(E_DIGITO('a'));
    ESCREVAL(E_LETRA('a'));
    ESCREVAL(E_LETRA('_'));
    ESCREVA(CARACTERE_MAIUSCULO('q')); ESCREVA_LINHA();
    ESCREVA(CARACTERE_MINUSCULO('Q')); ESCREVA_LINHA();
    ESCREVA(CARACTERE_MAIUSCULO('1')); ESCREVA_LINHA();

    // CARACTERE ocupa exatamente 1 byte; o sinal é o da plataforma (PT-4)
    ESCREVAL(sizeof(CARACTERE) == 1);
FIM
