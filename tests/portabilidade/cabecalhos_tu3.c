/* cabecalhos_tu3.c — unidade de tradução 3: BG_SEM_APELIDOS.
 *
 * A flag só vale por unidade de tradução (é macro), então esta unidade convive
 * com as outras, que usam os nomes curtos. Aqui só a forma prefixada existe.
 *
 * Compilar: ver cabecalhos_tu1.c.
 */
#define BG_SEM_APELIDOS
#include <basicgol.h>

#ifdef SE
#  error "nome curto SE nao deveria existir com BG_SEM_APELIDOS"
#endif
#ifdef ESCREVAL
#  error "nome curto ESCREVAL nao deveria existir com BG_SEM_APELIDOS"
#endif
#ifdef DIVIDE
#  error "nome curto DIVIDE nao deveria existir com BG_SEM_APELIDOS"
#endif

int bg_tu3_resultado(void)
{
    int total = 0;
    int i = 0;

    BG_SE 1 BG_ENTAO
        total = BG_DIVIDE(9, 3);
    BG_FIM_SE

    BG_PARA (i, 1, 2) {
        total = total + i;
    }
    BG_SE total == 6 BG_ENTAO
        total = total + 1;
    BG_FIM_SE

    return BG_TAMANHO_TEXTO("tu3") == 3u ? total : 0;   /* 7 */
}
