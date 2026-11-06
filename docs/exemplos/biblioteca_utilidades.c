/*
 * biblioteca_utilidades.c — semente explícita e limite do sorteio.
 * rand() não é criptográfico e sua sequência varia entre implementações C.
 * Compilar a partir da raiz: make exemplos
 */
#include <basicgol/utilidades.h>

#include <stdio.h>

int main(void)
{
    unsigned sorteado;
    unsigned primeiro;

    bg_inicia_sorteio(1234U);
    if (!bg_sorteia_ate(10U, &sorteado)) {
        return 1;
    }
    (void) printf("dentro de [0,10): %s\n",
                  (sorteado < 10U) ? "sim" : "nao");
    primeiro = sorteado;

    bg_inicia_sorteio(1234U);
    if (!bg_sorteia_ate(10U, &sorteado)) {
        return 1;
    }
    (void) printf("mesma semente, mesmo valor neste processo: %s\n",
                  (primeiro == sorteado) ? "sim" : "nao");
    return 0;
}
