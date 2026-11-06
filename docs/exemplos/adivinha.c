// adivinha.c — REPITA/ATE_QUE, sorteio, LEIA_OK e ESCOLHA.
// Perfil: M0 (macros).
// Compilar a partir da raiz: make exemplos
#include <basicgol.h>

CONSTANTE INTEIRO k_limite = 100;

INICIO
    INTEIRO alvo = 0;
    INTEIRO palpite = 0;
    INTEIRO tentativas = 0;

    /* A fixed seed repeats the sequence on the same C runtime, not across
     * runtimes. The input covers the glibc/UCRT and MSVCRT targets. */
    INICIA_SORTEIO(2026);
    alvo = SORTEIA(k_limite) + 1;         // 1 .. k_limite

    ESCREVA("Adivinhe o numero entre 1 e ");
    ESCREVAL(k_limite);

    REPITA
        ESCREVA("Seu palpite: ");
        SE NAO LEIA_OK(palpite) ENTAO
            ERRO("entrada invalida");
        FIM_SE
        tentativas = tentativas + 1;

        SE palpite < alvo ENTAO
            ESCREVAL("Muito baixo!");
        SENAO_SE palpite > alvo ENTAO
            ESCREVAL("Muito alto!");
        SENAO
            ESCREVAL("Acertou!");
        FIM_SE
    ATE_QUE(palpite == alvo);

    ESCREVA("Tentativas: ");
    ESCREVAL(tentativas);

    ESCOLHA (tentativas)
        CASO 1:
            ESCREVAL("Sorte de principiante.");
            PARE;
        CASO 2:
        CASO 3:
            ESCREVAL("Muito bom.");
            PARE;
        CONTRARIO:
            SE tentativas <= 7 ENTAO
                ESCREVAL("Dentro do esperado.");
            SENAO
                ESCREVAL("Continue treinando.");
            FIM_SE
    FIM_ESCOLHA
FIM
