// controle_acumuladores.c — contadores, acumuladores, maior/menor e zero iteracoes.
// Perfil: M0 (macros).
// Compilar a partir da raiz: make exemplos
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO soma = 0;
    INTEIRO contador = 0;
    INTEIRO produto = 1;
    INTEIRO maior = 0;
    INTEIRO menor = 0;
    INTEIRO vazio = 0;
    REAL media = 0.0;

    // Acumulador simples: 1 + 2 + ... + 100.
    PARA (i, 1, 100) {
        soma = soma + i;
    }
    ESCREVA("Soma de 1 a 100: ");
    ESCREVAL(soma);

    // Contador com condicao: quantos multiplos de 3 ha entre 1 e 30.
    PARA (i, 1, 30) {
        SE i MOD 3 == 0 ENTAO
            contador = contador + 1;
        FIM_SE
    }
    ESCREVA("Multiplos de 3 ate 30: ");
    ESCREVAL(contador);

    // Acumulador multiplicativo com ENQUANTO: 10!.
    i = 1;
    ENQUANTO i <= 10 FACA
        produto = produto * i;
        i = i + 1;
    FIM_ENQUANTO
    ESCREVA("10! = ");
    ESCREVAL(produto);

    // Maior e menor: o primeiro valor inicializa os dois; o laço só compara.
    VETOR(INTEIRO, valores, 6);
    valores[0] = 4;  valores[1] = -7; valores[2] = 12;
    valores[3] = 0;  valores[4] = 9;  valores[5] = -1;
    maior = valores[0];
    menor = valores[0];
    PARA (i, 1, 5) {
        SE valores[i] > maior ENTAO
            maior = valores[i];
        FIM_SE
        SE valores[i] < menor ENTAO
            menor = valores[i];
        FIM_SE
    }
    ESCREVA("Maior: ");
    ESCREVA(maior);
    ESCREVA(" | menor: ");
    ESCREVAL(menor);

    // Media com acumulador REAL: a divisao e feita com 4.0 para nao truncar.
    soma = 0;
    PARA (i, 0, 3) {
        soma = soma + valores[i];
    }
    media = soma / 4.0;
    ESCREVA("Media de 4, -7, 12, 0: ");
    ESCREVA_FMT("%.2f", media);
    ESCREVA_LINHA();

    // Zero iteracoes: o acumulador fica no valor inicial.
    PARA (i, 5, 1) {
        vazio = vazio + 1;
    }
    ESCREVA("Laco vazio, acumulador: ");
    ESCREVAL(vazio);

    // Contagem regressiva com passo negativo: 10, 7, 4, 1.
    soma = 0;
    PARA_PASSO (i, 10, 1, -3) {
        ESCREVA_FMT("%d ", i);
        soma = soma + i;
    }
    ESCREVA_LINHA();
    ESCREVA("Soma da contagem regressiva: ");
    ESCREVAL(soma);
FIM
