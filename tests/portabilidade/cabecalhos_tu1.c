/* cabecalhos_tu1.c — unidade de tradução 1: inclusão múltipla e ordem.
 *
 * Inclui o agregador, cada módulo e o agregador de novo, em ordens diferentes.
 * As guardas de inclusão têm de evitar redefinição de tipos/macros, e um
 * programa de várias unidades de tradução não pode ter conflito de símbolos
 * (este arquivo define só `bg_tu1_resultado`).
 *
 * Compilar (junto com cabecalhos_tu2.c e cabecalhos_principal.c):
 *   cc -std=c11 -Wall -Wextra -Wformat=2 -Werror -I include \
 *      tests/portabilidade/cabecalhos_tu1.c tests/portabilidade/cabecalhos_tu2.c \
 *      tests/portabilidade/cabecalhos_principal.c build/libbasicgol.a -lm
 */
#include <basicgol.h>
#include <basicgol/biblioteca.h>
#include <basicgol/consola.h>
#include <basicgol/consola.h>       /* de novo: a guarda tem de valer */
#include <basicgol/texto.h>
#include <basicgol/texto.h>
#include <basicgol/caractere.h>
#include <basicgol/matematica.h>
#include <basicgol/utilidades.h>
#include <basicgol/biblioteca.h>
#include <basicgol.h>               /* de novo: idem */
#include <basicgol.h>

/* Também os nomes curtos, definidos na primeira inclusão, continuam valendo. */
int bg_tu1_resultado(void)
{
    char dst[8];
    int valor = 0;

    if (!bg_e_digito('7') || !E_DIGITO('9')) {
        return 0;
    }
    if (bg_tamanho_texto("abc") != 3u || TAMANHO_TEXTO("abcd") != 4u) {
        return 0;
    }
    if (!bg_copia_texto(dst, sizeof dst, "tu1")) {
        return 0;
    }
    if (bg_compara_texto(dst, "tu1") != 0) {
        return 0;
    }
    if (bg_divide_i(14, 2, __FILE__, __LINE__) != 7) {
        return 0;
    }
    if (!bg_sorteia_ate(1u, (unsigned *) &valor) && valor != 0) {
        return 0;
    }
    return 7;
}
