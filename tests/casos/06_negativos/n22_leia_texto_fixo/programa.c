/* n22 — LEIA nao aceita TEXTO_FIXO (buffer de char).
 *
 * Compilar: cc -std=c11 -I ../../../include programa.c -lm
 *
 * Esperado: erro de compilacao. `&(buf)` e `char (*)[N]`, nao `char *`; antes
 * da Fase 6 caia em "%p" e o scanf escrevia um ponteiro (8 bytes) dentro do
 * buffer — com TEXTO_FIXO de menos de 8 bytes isso e estouro de pilha.
 * A forma correta e LEIA_TEXTO(buf, TAMANHO_DE(buf)).
 */
#include <basicgol.h>

INICIO
    TEXTO_FIXO(buf, 8);

    LEIA(buf);
    ESCREVAL(buf);
FIM
