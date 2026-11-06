// vetores.c — VETOR, TAMANHO_VETOR e o idioma "vetor + tamanho".
// Perfil: M0 (macros).
// Compilar a partir da raiz: make exemplos
#include <basicgol.h>

// Soma os n primeiros elementos.
FUNCAO INTEIRO soma_de(INTEIRO v[], INTEIRO n) {
    INTEIRO i = 0;
    INTEIRO total = 0;

    PARA (i, 0, n - 1) {
        total = total + v[i];
    }
    RETORNE total;
}

// Media dos n primeiros elementos (0.0 para n <= 0).
FUNCAO REAL media_de(INTEIRO v[], INTEIRO n) {
    SE n <= 0 ENTAO
        RETORNE 0.0;
    FIM_SE
    RETORNE (REAL) soma_de(v, n) / (REAL) n;
}

// Maior elemento.
FUNCAO INTEIRO maior_de(INTEIRO v[], INTEIRO n) {
    INTEIRO i = 0;
    INTEIRO maior = v[0];

    PARA (i, 1, n - 1) {
        SE v[i] > maior ENTAO
            maior = v[i];
        FIM_SE
    }
    RETORNE maior;
}

PROCEDIMENTO imprime(INTEIRO v[], INTEIRO n) {
    INTEIRO i = 0;

    PARA (i, 0, n - 1) {
        ESCREVA_FMT("%4d", v[i]);
    }
    ESCREVA_LINHA();
}

INICIO
    VETOR(INTEIRO, notas, 8);
    INTEIRO n = (INTEIRO) TAMANHO_VETOR(notas);
    INTEIRO i = 0;

    // Sem inicializador de vetor na sintaxe BG: preencha no laco.
    PARA (i, 0, n - 1) {
        notas[i] = (i * 7 + 3) MOD 11;
    }

    ESCREVA("Vetor: ");
    imprime(notas, n);

    ESCREVA("Soma:   ");
    ESCREVAL(soma_de(notas, n));

    ESCREVA("Media:  ");
    ESCREVA_FMT("%.2f\n", media_de(notas, n));

    ESCREVA("Maior:  ");
    ESCREVAL(maior_de(notas, n));
FIM
