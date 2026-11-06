// controle_lacos.c — laços aninhados, PROSSIGA/PARE, REPITA e termino garantido.
// Perfil: M0 (macros).
// Compilar a partir da raiz: make exemplos
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO j = 0;
    INTEIRO n = 6;
    INTEIRO passos = 0;
    INTEIRO num = 2026;
    INTEIRO soma = 0;

    // 1) Laços aninhados: a tabela de produtos, pulando a diagonal (j == i).
    //    PROSSIGA afeta só o laço de j, o mais interno.
    ESCREVAL("Produtos fora da diagonal:");
    PARA (i, 1, 3) {
        PARA (j, 1, 4) {
            SE j == i ENTAO
                PROSSIGA;
            FIM_SE
            ESCREVA_FMT("%4d", i * j);
        }
        ESCREVA_LINHA();
    }

    // 2) ENQUANTO com contador: a sequencia de Collatz de 6 ate 1.
    //    O laço termina porque a sequencia termina (o exemplo mostra o contador).
    ENQUANTO n != 1 FACA
        SE n MOD 2 == 0 ENTAO
            n = n / 2;
        SENAO
            n = 3 * n + 1;
        FIM_SE
        passos = passos + 1;
    FIM_ENQUANTO
    ESCREVA("Collatz de 6 ate 1: ");
    ESCREVA(passos);
    ESCREVAL(" passos");

    // 3) REPITA executa ao menos uma vez e testa no fim: soma os digitos de 2026.
    REPITA
        soma = soma + num MOD 10;
        num = num / 10;
    ATE_QUE(num == 0);
    ESCREVA("Soma dos digitos de 2026: ");
    ESCREVAL(soma);

    // 4) PARE sai do laço mais interno. Para sair de dois laços, use uma bandeira.
    INTEIRO achou = 0;
    INTEIRO ai = 0;
    INTEIRO aj = 0;
    PARA (i, 1, 9) {
        PARA (j, 1, 9) {
            SE i * j == 12 ENTAO
                achou = 1;
                ai = i;
                aj = j;
                PARE;
            FIM_SE
        }
        SE achou ENTAO
            PARE;
        FIM_SE
    }
    ESCREVA("Primeiro par com produto 12: ");
    ESCREVA(ai);
    ESCREVA(" x ");
    ESCREVAL(aj);
FIM
