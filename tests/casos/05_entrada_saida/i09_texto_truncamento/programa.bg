
// COPIA/JUNTA informam truncamento e deixam o destino vazio (RB-3)
#include <basicgol.h>

INICIO
    TEXTO_FIXO(a, 4);
    TEXTO_FIXO(b, 8);
    LOGICO ok = FALSO;

    ok = COPIA_TEXTO(a, (INTEIRO) sizeof a, "abcdef");
    ESCREVAL(ok);            // 0
    ESCREVAL(TAMANHO_TEXTO(a));  // 0 (destino vazio)

    ok = COPIA_TEXTO(a, (INTEIRO) sizeof a, "abc");
    ESCREVAL(ok);            // 1
    ESCREVAL(a);

    ok = JUNTA_TEXTO(b, (INTEIRO) sizeof b, "abcd", "efgh");
    ESCREVAL(ok);            // 0
    ok = JUNTA_TEXTO(b, (INTEIRO) sizeof b, "abc", "de");
    ESCREVAL(ok);            // 1
    ESCREVAL(b);
FIM
