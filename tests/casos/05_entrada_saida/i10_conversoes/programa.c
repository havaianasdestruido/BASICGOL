
// conversao texto->numero: aceita o texto inteiro, rejeita sobra
#include <basicgol.h>

INICIO
    INTEIRO n = 0;
    REAL r = 0.0;

    ESCREVAL(TEXTO_PARA_INTEIRO("42", &n));      // 1
    ESCREVAL(n);
    ESCREVAL(TEXTO_PARA_INTEIRO("-7", &n));      // 1
    ESCREVAL(n);
    ESCREVAL(TEXTO_PARA_INTEIRO("  13  ", &n));  // 1
    ESCREVAL(n);
    ESCREVAL(TEXTO_PARA_INTEIRO("4x2", &n));     // 0
    ESCREVAL(TEXTO_PARA_INTEIRO("", &n));        // 0
    ESCREVAL(TEXTO_PARA_INTEIRO("abc", &n));     // 0

    ESCREVAL(TEXTO_PARA_REAL("2.75", &r));       // 1
    ESCREVA_FMT("%.2f\n", r);
    ESCREVAL(TEXTO_PARA_REAL("2,75", &r));       // 0 (locale C)
    ESCREVAL(TEXTO_PARA_REAL("1e3", &r));        // 1
    ESCREVA_FMT("%.0f\n", r);
FIM
