// Tipos inteiros: representação, largura, faixa e aritmética definida (Fase 4)
#include <basicgol.h>

INICIO
    // --- os tipos com sinal existem, com a largura que o nome promete
    INTEIRO8  i8  = 127;
    INTEIRO16 i16 = 32767;
    INTEIRO32 i32 = 2147483647;
    INTEIRO64 i64 = 9223372036854775807;
    INTEIRO_CURTO curto = -1;
    INTEIRO_LONGO longo = -1;
    NATURAL_CURTO ncurto = 65535;
    NATURAL_LONGO nlongo = 4000000000UL;

    ESCREVAL(BITS_DE(INTEIRO8) == 8);
    ESCREVAL(BITS_DE(INTEIRO16) == 16);
    ESCREVAL(BITS_DE(INTEIRO32) == 32);
    ESCREVAL(BITS_DE(INTEIRO64) == 64);
    ESCREVAL(TAMANHO_DE(CARACTERE) == 1);
    ESCREVAL(sizeof(INTEIRO_CURTO) <= sizeof(INTEIRO));
    ESCREVAL(sizeof(INTEIRO) <= sizeof(INTEIRO_LONGO));

    ESCREVAL(i8);
    ESCREVAL(i16);
    ESCREVAL(i32);
    ESCREVAL(i64);
    ESCREVAL(curto);
    ESCREVAL(longo);
    ESCREVAL(ncurto);
    ESCREVAL(nlongo);

    // --- divisão inteira trunca em direção a zero; o resto segue o dividendo
    ESCREVAL(7 / 2);
    ESCREVAL(-7 / 2);
    ESCREVAL(7 / -2);
    ESCREVAL(-8 / 2);
    ESCREVAL(7 MOD 2);
    ESCREVAL(-7 MOD 2);
    ESCREVAL(7 MOD -2);

    // --- natural estoura em módulo 2^N, o que C define (não é indefinido)
    NATURAL8 n8 = 255;
    NATURAL16 n16 = 65535;
    NATURAL32 n32 = 4294967295u;
    n8 = n8 + 1;
    n16 = n16 + 1;
    n32 = n32 + 1u;
    ESCREVAL(n8);
    ESCREVAL(n16);
    ESCREVAL(n32);

    // conversão de negativo para natural é módulo 2^N, também definida
    ESCREVAL((NATURAL) -1 == MAXIMO_NATURAL);
FIM
