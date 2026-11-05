// cadastro.c — REGISTRO, vetor de registros, LEIA_TEXTO e busca.
// Perfil: M0 (macros).
// Compilar: cc -std=c11 -Wall -Wextra cadastro.c -o cadastro
#include <basicgol.h>

CONSTANTE INTEIRO k_max_alunos = 30;

REGISTRO(Aluno)
    INTEIRO matricula;
    TEXTO_FIXO(nome, 40);
    REAL nota;
FIM_REGISTRO(Aluno);

// Procura a matricula e devolve o indice, ou -1.
FUNCAO INTEIRO busca_por_matricula(Aluno turma[], INTEIRO n, INTEIRO matricula) {
    INTEIRO i = 0;

    PARA (i, 0, n - 1) {
        SE turma[i].matricula == matricula ENTAO
            RETORNE i;
        FIM_SE
    }
    RETORNE -1;
}

FUNCAO REAL media_da_turma(Aluno turma[], INTEIRO n) {
    INTEIRO i = 0;
    REAL total = 0.0;

    SE n <= 0 ENTAO
        RETORNE 0.0;
    FIM_SE

    PARA (i, 0, n - 1) {
        total = total + turma[i].nota;
    }
    RETORNE total / (REAL) n;
}

INICIO
    // Atencao (C-23): em C, "static const" NAO e expressao constante, entao
    // VETOR(Aluno, turma, k_max_alunos) viraria um VLA. Use um literal.
    VETOR(Aluno, turma, 30);
    INTEIRO quantidade = 0;
    INTEIRO matricula = 0;
    INTEIRO posicao = 0;

    ESCREVA("Quantos alunos (1 a 30)? ");
    SE NAO LEIA_OK(quantidade) OU quantidade < 1 OU quantidade > k_max_alunos ENTAO
        ERRO("quantidade invalida");
    FIM_SE

    // Leitura dos dados, um aluno por vez.
    INTEIRO i = 0;
    PARA (i, 0, quantidade - 1) {
        ESCREVA("Matricula do aluno ");
        ESCREVA(i + 1);
        ESCREVA(": ");
        SE NAO LEIA_OK(turma[i].matricula) ENTAO
            ERRO("matricula invalida");
        FIM_SE

        ESCREVA("Nome: ");
        // LEIA_TEXTO descarta o resto da linha deixado por LEIA (ver C-20).
        SE NAO LEIA_TEXTO(turma[i].nome, (INTEIRO) sizeof turma[i].nome) ENTAO
            ERRO("falha ao ler o nome");
        FIM_SE

        ESCREVA("Nota: ");
        SE NAO LEIA_OK(turma[i].nota) ENTAO
            ERRO("nota invalida");
        FIM_SE
    }

    ESCREVAL("");
    ESCREVA_FMT("%-6s %-20s %6s\n", "MATR.", "NOME", "NOTA");
    PARA (i, 0, quantidade - 1) {
        ESCREVA_FMT("%-6d %-20s %6.2f\n",
                    turma[i].matricula, turma[i].nome, turma[i].nota);
    }

    ESCREVA("Media da turma: ");
    ESCREVA_FMT("%.2f\n", media_da_turma(turma, quantidade));

    // Busca interativa.
    ENQUANTO VERDADEIRO FACA
        ESCREVA("Buscar matricula (0 encerra): ");
        SE NAO LEIA_OK(matricula) OU matricula == 0 ENTAO
            PARE;
        FIM_SE

        posicao = busca_por_matricula(turma, quantidade, matricula);
        SE posicao >= 0 ENTAO
            ESCREVA("Encontrado: ");
            ESCREVA_TEXTO(turma[posicao].nome);
            ESCREVA("  nota ");
            ESCREVA_FMT("%.2f\n", turma[posicao].nota);
        SENAO
            ESCREVAL("Matricula nao encontrada.");
        FIM_SE
    FIM_ENQUANTO

    ESCREVAL("Fim do cadastro.");
FIM
