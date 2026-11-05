/* t15 — LEIA com os tipos que o `scanf` precisa conhecer.
 *
 * Compilar: cc -std=c11 -I ../../../include programa.c -lm
 * Entrada:  entrada.txt (uma linha por valor, na ordem das leituras)
 *
 * O formato de leitura e escolhido por _Generic a partir do tipo do
 * destino: sem isso, `LEIA` de um NATURAL8 gravaria um ponteiro dentro de
 * um byte. Todo tipo escalar de BASICGOL tem formato proprio, exceto
 * LOGICO (recusado pelo compilador: ver 06_negativos/n16).
 */
#include <basicgol.h>

INICIO
    NATURAL8      a;
    NATURAL16     b;
    NATURAL_CURTO c;
    REAL_CURTO    d;
    REAL_LONGO    e;
    CARACTERE     f;

    LEIA(a);
    LEIA(b);
    LEIA(c);
    LEIA(d);
    LEIA(e);
    LEIA(f);

    ESCREVA("NATURAL8       "); ESCREVAL(a);
    ESCREVA("NATURAL16      "); ESCREVAL(b);
    ESCREVA("NATURAL_CURTO  "); ESCREVAL(c);
    ESCREVA("REAL_CURTO     "); ESCREVAL(d);
    ESCREVA("REAL_LONGO     "); ESCREVAL(e);
    ESCREVA("CARACTERE      "); ESCREVAL(f);
FIM
