#include <basicgol.h>

// W-07: tamanho de vetor que e CONSTANTE vira VLA em C (C-23)
CONSTANTE INTEIRO k_n = 4;

INICIO
    VETOR(INTEIRO, v, k_n);
    v[0] = 1;
    ESCREVAL(v[0]);
FIM
