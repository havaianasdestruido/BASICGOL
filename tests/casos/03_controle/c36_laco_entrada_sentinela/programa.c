// Laço dirigido por entrada: lê inteiros até a sentinela 0. A segunda rodada
// começa com a sentinela, então o laço executa zero vezes.
#include <basicgol.h>

INICIO
    INTEIRO valor = 0;
    INTEIRO qtd = 0;
    INTEIRO soma = 0;
    INTEIRO maximo = 0;
    INTEIRO minimo = 0;

    ESCREVAL("Rodada 1");
    SE NAO LEIA_INTEIRO(valor) ENTAO
        ERRO("entrada invalida");
    FIM_SE
    ENQUANTO valor != 0 FACA
        // primeiro valor inicializa máximo e mínimo; os demais só os atualizam
        SE qtd == 0 ENTAO
            maximo = valor;
            minimo = valor;
        FIM_SE
        SE valor > maximo ENTAO
            maximo = valor;
        FIM_SE
        SE valor < minimo ENTAO
            minimo = valor;
        FIM_SE
        soma = soma + valor;
        qtd = qtd + 1;
        SE NAO LEIA_INTEIRO(valor) ENTAO
            ERRO("entrada invalida");
        FIM_SE
    FIM_ENQUANTO
    ESCREVA("quantidade: ");
    ESCREVAL(qtd);
    ESCREVA("soma: ");
    ESCREVAL(soma);
    ESCREVA("maximo: ");
    ESCREVAL(maximo);
    ESCREVA("minimo: ");
    ESCREVAL(minimo);

    ESCREVAL("Rodada 2");
    qtd = 0;
    SE NAO LEIA_INTEIRO(valor) ENTAO
        ERRO("entrada invalida");
    FIM_SE
    ENQUANTO valor != 0 FACA
        qtd = qtd + 1;
        SE NAO LEIA_INTEIRO(valor) ENTAO
            ERRO("entrada invalida");
        FIM_SE
    FIM_ENQUANTO
    ESCREVA("quantidade: ");
    ESCREVAL(qtd);
FIM
