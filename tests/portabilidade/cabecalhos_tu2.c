/* cabecalhos_tu2.c — unidade de tradução 2: módulos antes do agregador e
 * seção de extensões ligada.
 *
 * `BG_USAR_EXTENSOES` produz `static inline` no cabeçalho: cada unidade ganha
 * a sua cópia interna, sem símbolo externo e sem conflito na ligação. Ter a
 * flag numa unidade e não na outra também precisa funcionar.
 *
 * Compilar: ver cabecalhos_tu1.c.
 */
#define BG_USAR_EXTENSOES
#include <basicgol/matematica.h>
#include <basicgol/consola.h>
#include <basicgol/texto.h>
#include <basicgol/caractere.h>
#include <basicgol/utilidades.h>
#include <basicgol/biblioteca.h>
#include <basicgol.h>
#include <basicgol.h>

int bg_tu2_resultado(void)
{
    int soma = 0;
    int produto = 0;
    int diferenca = 0;

    if (!SOMA_SEGURA(1, 2, &soma) || soma != 3) {
        return 0;
    }
    if (!BG_MULTIPLICA_SEGURA(2, 3, &produto) || produto != 6) {
        return 0;
    }
    if (!bg_subtrai_segura(1, 2, &diferenca) || diferenca != -1) {
        return 0;
    }
    /* estouro detectado em vez de comportamento indefinido */
    if (SOMA_SEGURA(INT_MAX, 1, &soma)) {
        return 0;
    }
    return 8;
}
