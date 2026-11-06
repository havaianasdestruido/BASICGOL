/* n21 — LEIA nao aceita TEXTO (const char *).
 *
 * Compilar: cc -std=c11 -I ../../../include programa.c -lm
 *
 * Esperado: erro de compilacao. `&(t)` e `const char **`, que nao casa com
 * nenhuma associacao de BG_SFMT; antes da Fase 6 caia em "%p" e o scanf
 * gravava um ponteiro dentro da variavel, sem diagnostico fora do -Wformat.
 * Para ler texto use LEIA_TEXTO(buf, tam) com um TEXTO_FIXO.
 */
#include <basicgol.h>

INICIO
    TEXTO nome = "?";

    LEIA(nome);
    ESCREVAL(nome);
FIM
