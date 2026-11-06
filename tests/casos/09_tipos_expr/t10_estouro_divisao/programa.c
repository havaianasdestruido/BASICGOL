// DIVIDE evita UB quando MINIMO_INTEIRO / -1 não cabe no tipo.
#include <basicgol.h>

INICIO
    INTEIRO minimo = MINIMO_INTEIRO;
    INTEIRO quociente = 0;

    if (MINIMO_INTEIRO < -MAXIMO_INTEIRO) {
        ESCREVA("MIN/-1: nao representavel");
        quociente = DIVIDE(minimo, -1);
        RETORNE 9; /* DIVIDE precisa falhar nesta representação. */
    }

    quociente = DIVIDE(minimo, -1);
    if (quociente != MAXIMO_INTEIRO) {
        RETORNE 9;
    }
    ESCREVA("MIN/-1: representavel: ");
    ESCREVAL("ok");
FIM
