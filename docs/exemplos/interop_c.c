// interop_c.c — BASICGOL e C no mesmo arquivo, sem atrito.
// Perfil: M0 (macros) + C nativo.
// Compilar a partir da raiz: make exemplos
#include <basicgol.h>
#include <time.h>          // biblioteca C usada diretamente

// Um struct C declarado a mao, do lado de um REGISTRO em BG.
struct ponto {
    double x;
    double y;
};

// Funcao escrita em C puro, chamada por codigo BG.
static double distancia(struct ponto a, struct ponto b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);       // sqrt: <math.h>, via -lm
}

REGISTRO(Aluno)
    TEXTO nome;
    REAL nota;
FIM_REGISTRO(Aluno);

INICIO
    struct ponto p1 = { 0.0, 0.0 };       // C nativo
    struct ponto p2 = { 3.0, 4.0 };
    Aluno a;

    a.nome = "Ana";
    a.nota = 9.5;

    ESCREVA("Aluno: ");
    ESCREVA_TEXTO(a.nome);
    ESCREVA("  nota: ");
    ESCREVA_FMT("%.1f\n", a.nota);

    ESCREVA("Distancia entre (0,0) e (3,4): ");
    ESCREVA_FMT("%.1f\n", distancia(p1, p2));

    // Mistura dos dois mundos no mesmo bloco: um laco C e um laco BG.
    INTEIRO i = 0;
    for (i = 1; i <= 3; i++) {
        printf("[C] iteracao %d\n", i);
    }
    PARA (i, 1, 3) {
        ESCREVA_FMT("[BG] iteracao %d\n", i);
    }

    // Ponteiro e aritmetica de ponteiro: C nativo, sem vocabulario BG.
    TEXTO texto = "basico";
    TEXTO cursor = texto;
    INTEIRO letras = 0;
    ENQUANTO *cursor != '\0' FACA
        letras = letras + 1;
        cursor = cursor + 1;
    FIM_ENQUANTO
    ESCREVA("Letras em 'basico': ");
    ESCREVAL(letras);

    // Hora do sistema, so para provar que qualquer API C esta disponivel.
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    SE t != NULL ENTAO
        ESCREVA_FMT("Ano do sistema: %d\n", t->tm_year + 1900);
    FIM_SE
FIM
