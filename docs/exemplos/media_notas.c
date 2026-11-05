// media_notas.c — funcao, condicional encadeado e formatacao de reais.
// Perfil: M0 (macros).
// Compilar: cc -std=c11 -Wall -Wextra media_notas.c -o media_notas
#include <basicgol.h>

// Media aritmetica simples: dois valores.
FUNCAO REAL media(REAL a, REAL b) {
    RETORNE (a + b) / 2.0;
}

// Devolve a situacao como texto (TEXTO e imutavel: podemos devolver literais).
FUNCAO TEXTO situacao(REAL m) {
    SE m >= 7.0 ENTAO
        RETORNE "Aprovado";
    SENAO_SE m >= 5.0 ENTAO
        RETORNE "Recuperacao";
    SENAO
        RETORNE "Reprovado";
    FIM_SE
    RETORNE "?";   // inalcancavel; existe para o compilador nao reclamar
}

INICIO
    REAL nota1 = 0.0;
    REAL nota2 = 0.0;
    REAL m = 0.0;

    ESCREVA("Nota 1: ");
    SE NAO LEIA_OK(nota1) ENTAO
        ERRO("entrada invalida para a nota 1");
    FIM_SE

    ESCREVA("Nota 2: ");
    SE NAO LEIA_OK(nota2) ENTAO
        ERRO("entrada invalida para a nota 2");
    FIM_SE

    m = media(nota1, nota2);

    ESCREVA("Media: ");
    ESCREVA_FMT("%.2f", m);
    ESCREVA("  Situacao: ");
    ESCREVAL(situacao(m));

    // Atencao: ESCREVAL usa %g; para duas casas decimais use ESCREVA_FMT("%.2f").
FIM
