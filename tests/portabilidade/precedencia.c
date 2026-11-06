/* precedencia.c — precedência, associatividade e aninhamento de macros.
 *
 * Parte da suíte de portabilidade da Fase 6 (tests/run_portabilidade.sh).
 * Cada verificação é uma asserção sobre o valor calculado; a primeira que
 * falhar encerra com `arquivo:linha: erro: ...` e código 1. Sucesso: imprime
 * uma linha e sai 0. Só inteiros são impressos — nada de formato de ponto
 * flutuante, que varia entre bibliotecas C.
 *
 * Compilar: cc -std=c11 -Wall -Wextra -Wformat=2 -Werror -I include \
 *              tests/portabilidade/precedencia.c build/libbasicgol.a -lm
 */
#include <basicgol.h>

static INTEIRO chamadas = 0;

FUNCAO INTEIRO soma(INTEIRO a, INTEIRO b) { RETORNE a + b; }

FUNCAO LOGICO sempre_verdadeiro(void) {
    chamadas = chamadas + 1;
    RETORNE VERDADEIRO;
}

INICIO
    INTEIRO a = 2;
    INTEIRO b = 3;
    INTEIRO c = 4;
    INTEIRO i = 0;
    LOGICO x = VERDADEIRO;
    LOGICO y = FALSO;
    INTEIRO v[3] = {7, 8, 9};

    /* ---------------------------------------------------- aritmética */
    AFIRME(a + b * c == 14, "a + b * c deve ser 14");
    AFIRME((a + b) * c == 20, "(a + b) * c deve ser 20");
    AFIRME(10 MOD 3 * 2 == 2, "10 MOD 3 * 2 deve ser 2 (MOD antes de *)");
    AFIRME(2 + 10 MOD 3 == 3, "2 + 10 MOD 3 deve ser 3");
    AFIRME(-7 / 2 == -3, "-7 / 2 trunca em direção a zero");
    AFIRME(-7 MOD 2 == -1, "-7 MOD 2 tem o sinal do dividendo");
    AFIRME(-2 * -3 == 6, "menos com menos");
    AFIRME((!(a - b)) == 0, "operador C ! convive com NAO");
    AFIRME((NAO (a - b)) == 0, "NAO (a - b) também");

    /* -------------------------------------------------------- lógica */
    AFIRME((NAO x E y) == FALSO, "NAO liga mais forte que E");
    AFIRME((NAO (x E y)) == VERDADEIRO, "NAO (x E y)");
    AFIRME((x OU (y E y)) == VERDADEIRO, "E liga mais forte que OU");
    AFIRME(((x E y) OU x) == VERDADEIRO, "(x E y) OU x");
    AFIRME((((NAO y) E x) OU y) == VERDADEIRO, "((NAO y) E x) OU y");
    AFIRME(((a < b) E (b < c)) == VERDADEIRO, "comparações com E");
    AFIRME((IGUAL(a + b, 5)) == VERDADEIRO, "IGUAL com expressão");
    AFIRME((DIFERENTE(soma(a, b), a + b)) == FALSO, "DIFERENTE com chamada");
    AFIRME((NAO IGUAL(a, b)) == VERDADEIRO, "NAO sobre IGUAL");
    AFIRME((OU_EXCLUSIVO(x, y)) == VERDADEIRO, "OU_EXCLUSIVO(1,0)");
    AFIRME((LOGICO_DE(0.5)) == VERDADEIRO, "LOGICO_DE(0.5)");
    AFIRME((NAO LOGICO_DE(0.0)) == VERDADEIRO, "LOGICO_DE(0.0) normaliza");

    /* -------------------------------------- macros aninhadas (C-04) */
    AFIRME(MAXIMO(soma(a, b), 7) == 7, "MAXIMO(chamada_com_virgula, 7)");
    AFIRME(MINIMO(soma(a, b), MAXIMO(a, c)) == 4, "MINIMO aninhado com MAXIMO");
    AFIRME(ABS(MINIMO(a - 10, 0)) == 8, "ABS(MINIMO(...))");
    AFIRME(DIVIDE(soma(a, b), a) == 2, "DIVIDE com chamada e divisão truncada");
    AFIRME(RESTO(soma(a, b) * 2, c) == 2, "RESTO(10, 4)");
    AFIRME(ABS(DIVIDE(RESTO(17, 5), 2)) == 1, "três macros aninhadas");
    AFIRME(POTENCIA(2, 3) + 1 == 9, "POTENCIA antes de +");
    AFIRME(ARREDONDA(2.5) == 3, "ARREDONDA(2.5) afasta do zero");
    AFIRME(ARREDONDA(-2.5) == -3, "ARREDONDA(-2.5) afasta do zero");
    AFIRME(TAMANHO_VETOR(v) * 2 == 6, "TAMANHO_VETOR em expressão");
    AFIRME(BITS_DE(CARACTERE) == 8, "BITS_DE(CARACTERE)");
    AFIRME(TAMANHO_DE(INTEIRO8) == 1, "TAMANHO_DE(INTEIRO8)");
    AFIRME(TEXTOS_IGUAIS("ab", "ab") E (NAO TEXTOS_DIFERENTES("ab", "ab")),
           "TEXTOS_IGUAIS/DIFERENTES");

    /* ------------------------------------- avaliação única (P-11) */
    chamadas = 0;
    SE FALSO E sempre_verdadeiro() ENTAO
        ESCREVAL("nunca");
    FIM_SE
    SE VERDADEIRO OU sempre_verdadeiro() ENTAO
        ESCREVAL("");
    FIM_SE
    AFIRME(chamadas == 0, "E/OU fazem curto-circuito");

    i = 0;
    AFIRME((ABS(++i) == 1) E (i == 1), "ABS avalia o argumento uma vez");
    AFIRME((MINIMO(++i, 100) == 2) E (i == 2), "MINIMO avalia cada argumento uma vez");
    AFIRME((MAXIMO(++i, 0) == 3) E (i == 3), "MAXIMO avalia cada argumento uma vez");
    AFIRME((DIVIDE(++i, 1) == 4) E (i == 4), "DIVIDE avalia cada argumento uma vez");

    /* ------------------------------- atribuição e precedência mista */
    a = b = 3;
    AFIRME((a == 3) E (b == 3), "atribuição encadeada associa à direita");
    a += 2 * 3;
    AFIRME(a == 9, "+= com produto à direita");
    a = (b > 2) ? 10 : 20;
    AFIRME(a == 10, "ternário");
    AFIRME((x E y) == FALSO, "E devolve 0/1");

    ESCREVAL("precedencia: ok");
FIM
