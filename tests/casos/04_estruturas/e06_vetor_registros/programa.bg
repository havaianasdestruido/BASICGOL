
#include <basicgol.h>

REGISTRO(Aluno)
    TEXTO nome;
    REAL nota;
FIM_REGISTRO(Aluno);

INICIO
    VETOR(Aluno, turma, 3);
    INTEIRO i = 0;
    REAL total = 0.0;

    turma[0].nome = "Ana";  turma[0].nota = 9.0;
    turma[1].nome = "Bia";  turma[1].nota = 6.0;
    turma[2].nome = "Caio"; turma[2].nota = 7.5;

    PARA (i, 0, 2) {
        ESCREVA_TEXTO(turma[i].nome);
        ESCREVA(": ");
        ESCREVA_FMT("%.1f\n", turma[i].nota);
        total = total + turma[i].nota;
    }
    ESCREVA_FMT("media %.2f\n", total / 3.0);
FIM
