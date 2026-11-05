#include <basicgol.h>

// W-06: TAMANHO_VETOR de parametro mede o ponteiro, nao o vetor (C-18)
PROCEDIMENTO mostra(INTEIRO v[], INTEIRO n) {
    ESCREVAL(TAMANHO_VETOR(v));
    ESCREVAL(n);
}

INICIO
    VETOR(INTEIRO, dados, 4);
    dados[0] = 1;
    mostra(dados, TAMANHO_VETOR(dados));
FIM
