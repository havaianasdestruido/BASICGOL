// controle_condicoes.c — SE, SENAO_SE, SENAO aninhados, E/OU/NAO e ordem das condicoes.
// Perfil: M0 (macros).
// Compilar a partir da raiz: make exemplos
//
// Nao ha "dangling else": cada SE tem o seu FIM_SE, entao cada SENAO fecha o SE
// em que esta. Dentro de um SE externo, um SE interno so termina no seu proprio FIM_SE.
#include <basicgol.h>

// Classifica um triangulo pelos lados. A ordem importa: primeiro se existe,
// depois o tipo. Os ramos do SE de fora protegem os de dentro.
FUNCAO TEXTO triangulo(INTEIRO a, INTEIRO b, INTEIRO c) {
    SE a + b <= c OU a + c <= b OU b + c <= a ENTAO
        RETORNE "nao forma triangulo";
    FIM_SE

    SE a == b E b == c ENTAO
        RETORNE "equilatero";
    SENAO_SE a == b OU b == c OU a == c ENTAO
        RETORNE "isosceles";
    SENAO
        RETORNE "escaleno";
    FIM_SE

    RETORNE "?";   // inalcancavel; existe para o compilador nao reclamar
}

INICIO
    INTEIRO x = 0;

    ESCREVAL("-- triangulos --");
    ESCREVA("3,4,5: ");   ESCREVAL(triangulo(3, 4, 5));
    ESCREVA("2,2,2: ");   ESCREVAL(triangulo(2, 2, 2));
    ESCREVA("5,5,8: ");   ESCREVAL(triangulo(5, 5, 8));
    ESCREVA("1,2,3: ");   ESCREVAL(triangulo(1, 2, 3));
    ESCREVA("4,4,9: ");   ESCREVAL(triangulo(4, 4, 9));

    ESCREVAL("-- sinal e paridade --");
    PARA (x, -2, 2) {
        ESCREVA_FMT("%d: ", x);
        SE x > 0 ENTAO
            SE x MOD 2 == 0 ENTAO
                ESCREVAL("positivo par");
            SENAO
                ESCREVAL("positivo impar");
            FIM_SE
        SENAO_SE x < 0 ENTAO
            ESCREVAL("negativo");
        SENAO
            ESCREVAL("zero");
        FIM_SE
    }
FIM
