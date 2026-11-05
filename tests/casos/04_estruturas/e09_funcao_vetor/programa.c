
// vetor e passado por referencia: a funcao modifica o original
#include <basicgol.h>

PROCEDIMENTO dobra(INTEIRO v[], INTEIRO n) {
    INTEIRO i = 0;
    PARA (i, 0, n - 1) {
        v[i] = v[i] * 2;
    }
}

FUNCAO INTEIRO soma(INTEIRO v[], INTEIRO n) {
    INTEIRO i = 0;
    INTEIRO total = 0;
    PARA (i, 0, n - 1) {
        total = total + v[i];
    }
    RETORNE total;
}

INICIO
    VETOR(INTEIRO, v, 4);
    INTEIRO i = 0;
    PARA (i, 0, 3) {
        v[i] = i + 1;
    }
    dobra(v, (INTEIRO) TAMANHO_VETOR(v));
    ESCREVAL(v[0]);
    ESCREVAL(v[3]);
    ESCREVAL(soma(v, (INTEIRO) TAMANHO_VETOR(v)));
FIM
