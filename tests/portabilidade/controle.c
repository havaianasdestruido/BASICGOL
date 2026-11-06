/* controle.c — expansão de controle de fluxo, aninhamento e limites.
 *
 * Parte da suíte de portabilidade da Fase 6 (tests/run_portabilidade.sh).
 * Usa apenas construtores do perfil M0 (sem o açúcar `PARA i DE …`), de modo
 * que o mesmo fonte roda pelos dois caminhos: compilado direto (M0) e
 * transpilado por build/basicgolc (M1). O runner compara as duas saídas.
 *
 * Compilar: cc -std=c11 -Wall -Wextra -Wformat=2 -Werror -I include \
 *              tests/portabilidade/controle.c build/libbasicgol.a -lm
 */
#include <basicgol.h>

FUNCAO INTEIRO soma(INTEIRO a, INTEIRO b) { RETORNE a + b; }

INICIO
    INTEIRO i;
    INTEIRO j;
    INTEIRO total;
    INTEIRO iteracoes;
    LOGICO achou;
    INTEIRO alvo;

    /* ------------------------------------------------ PARA inclusivo */
    total = 0;
    PARA (i, 1, 3) {
        total = total + i;
    }
    AFIRME(total == 6, "PARA (i,1,3) soma 6");

    total = 0;
    PARA (i, 3, 1) {
        total = total + 1;              /* limite menor que o início: 0 voltas */
    }
    AFIRME(total == 0, "PARA (i,3,1) não itera");

    total = 0;
    PARA (i, 0, 2) {
        PARA (j, 0, 2) {
            total = total + 1;
        }
    }
    AFIRME(total == 9, "PARA aninhado conta 3x3");

    /* ---------------------------------------------- PARA_PASSO e passo */
    total = 0;
    PARA_PASSO(i, 0, 4, 2) {
        total = total + i;
    }
    AFIRME(total == 6, "PARA_PASSO(0..4, 2) soma 0+2+4");

    total = 0;
    PARA_PASSO(i, 4, 0, -2) {
        total = total + i;
    }
    AFIRME(total == 6, "PARA_PASSO(4..0, -2) soma 4+2+0");

    total = 0;
    PARA_PASSO(i, 1, 1, 1) {
        total = total + 1;
    }
    AFIRME(total == 1, "PARA_PASSO de um passo só");

    /* ------------------------------------------- ENQUANTO com vírgula */
    i = 0;
    iteracoes = 0;
    ENQUANTO (i = i + 1, i < 5) FACA
        iteracoes = iteracoes + 1;
    FIM_ENQUANTO
    AFIRME(iteracoes == 4, "ENQUANTO com vírgula conta 4 voltas");

    i = 0;
    ENQUANTO i < 0 FACA
        i = i + 1;                      /* zero voltas */
    FIM_ENQUANTO
    AFIRME(i == 0, "ENQUANTO com condição falsa não entra");

    /* ------------------------------- REPITA / ATE_QUE com PROSSIGA */
    i = 0;
    total = 0;
    REPITA
        i = i + 1;
        SE i == 2 ENTAO
            PROSSIGA;
        FIM_SE
        total = total + i;
    ATE_QUE(i >= 4);
    AFIRME(total == 8, "REPITA com PROSSIGA soma 1+3+4");
    AFIRME(i == 4, "REPITA termina em i == 4");

    i = 0;
    total = 0;
    REPITA
        i = i + 1;
        total = total + i;
    ATE_QUE(NAO (i < 3));
    AFIRME(total == 6 E i == 3, "ATE_QUE com NAO");

    /* ------------------------------------------- ESCOLHA e PARE */
    total = 0;
    PARA (i, 1, 3) {
        ESCOLHA (i)
            CASO 1:
                SE VERDADEIRO ENTAO
                    PARE;               /* sai do ESCOLHA, não do PARA */
                FIM_SE
            CASO 2:
                total = total + 2;
                PARE;
            CONTRARIO:
                total = total + 3;
        FIM_ESCOLHA
        total = total + 10;             /* sempre executa */
    }
    AFIRME(total == 35, "ESCOLHA com PARE dentro de SE (35)");

    total = 0;
    ESCOLHA (soma(1, 2))                /* vírgula de nível superior em macro */
        CASO 3:
            total = 30;
            PARE;
        CONTRARIO:
            total = 99;
    FIM_ESCOLHA
    AFIRME(total == 30, "ESCOLHA com chamada de dois argumentos");

    /* normaliza para a mesma superfície o aninhamento de ESCOLHA */
    total = 0;
    ESCOLHA (2)
        CASO 2:
            ESCOLHA (5)
                CASO 5:
                    total = 25;
                    PARE;
                CONTRARIO:
                    total = 12;
            FIM_ESCOLHA
            total = total + 1;
            PARE;
        CONTRARIO:
            total = 0;
    FIM_ESCOLHA
    AFIRME(total == 26, "ESCOLHA aninhada");

    /* --------------------------------- SE / SENAO_SE / SENAO encadeados */
    total = 0;
    SE 1 == 2 ENTAO
        total = 10;
    SENAO_SE 2 == 2 ENTAO
        total = 20;
    SENAO_SE 3 == 3 ENTAO
        total = 30;
    SENAO
        total = 40;
    FIM_SE
    AFIRME(total == 20, "SENAO_SE escolhe o segundo ramo");

    total = 0;
    SE 1 == 2 ENTAO
    FIM_SE                          /* bloco vazio */
    SE VERDADEIRO ENTAO
        total = 1;
    FIM_SE
    AFIRME(total == 1, "SE vazio não interfere");

    /* ------------------------------- saída de dois laços com bandeira */
    achou = FALSO;
    alvo = -1;
    PARA (i, 0, 2) {
        PARA (j, 0, 2) {
            SE i * 3 + j == 4 ENTAO
                achou = VERDADEIRO;
                alvo = i * 10 + j;
                PARE;
            FIM_SE
        }
        SE achou ENTAO
            PARE;
        FIM_SE
    }
    AFIRME(achou E alvo == 11, "saída de dois laços com bandeira");

    /* ------------------------------------ PROSSIGA em PARA (avança i) */
    total = 0;
    PARA (i, 0, 4) {
        SE i MOD 2 == 0 ENTAO
            PROSSIGA;
        FIM_SE
        total = total + i;
    }
    AFIRME(total == 4, "PROSSIGA em PARA soma ímpares 1+3");

    /* -------------------------------------------------- aninhamento */
    total = 0;
    i = 0;
    ENQUANTO i < 3 FACA
        j = 0;
        ENQUANTO j < 3 FACA
            SE (i + j) MOD 3 == 0 ENTAO
                total = total + 1;
            SENAO
                total = total + 10;
            FIM_SE
            j = j + 1;
        FIM_ENQUANTO
        i = i + 1;
    FIM_ENQUANTO
    AFIRME(total == 63, "dois ENQUANTO com SE/SENAO");

    ESCREVAL("controle: ok");
FIM
