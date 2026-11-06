/* cabecalhos_principal.c — liga as três unidades de tradução e confere.
 *
 * Compilar: ver cabecalhos_tu1.c.
 */
#include <basicgol.h>

int bg_tu1_resultado(void);
int bg_tu2_resultado(void);
int bg_tu3_resultado(void);

/* Esta unidade NÃO liga BG_USAR_EXTENSOES: nem a flag nem os nomes curtos
 * da seção 2b podem existir aqui (a flag é por unidade de tradução). */
#ifdef BG_TEM_EXTENSOES
#  error "BG_TEM_EXTENSOES so existe na unidade que define BG_USAR_EXTENSOES"
#endif
#ifdef SOMA_SEGURA
#  error "SOMA_SEGURA nao deveria existir nesta unidade"
#endif

/* Macro de versão precisa servir de condição de _Static_assert. */
_Static_assert(BG_VERSAO_MINIMA(1, 0), "BASICGOL 1.0 ou superior");
_Static_assert(BG_BIBLIOTECA_VERSAO_MAIOR >= 1, "biblioteca 1.x");

INICIO
    EXTERNO INTEIRO bg_tu1_resultado(void);
    INTEIRO a = bg_tu1_resultado();
    INTEIRO b = bg_tu2_resultado();
    INTEIRO c = bg_tu3_resultado();

    AFIRME(a == 7, "TU1 (inclusao multipla) devolveu 7");
    AFIRME(b == 8, "TU2 (extensoes) devolveu 8");
    AFIRME(c == 7, "TU3 (BG_SEM_APELIDOS) devolveu 7");

    ESCREVA_FMT("cabecalhos: ok (%d %d %d)\n", a, b, c);
FIM
