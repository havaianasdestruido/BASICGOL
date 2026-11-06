// controle_entrada.c — laços dirigidos por entrada: validacao com REPITA e sentinela com ENQUANTO.
// Perfil: M0 (macros). Entrada: controle_entrada.entrada.txt
// Compilar a partir da raiz: make exemplos
//
// LEIA_INTEIRO le uma linha inteira: uma linha invalida nao deixa lixo no stdin,
// entao o laco que pede de novo nao trava (diferente de scanf sobre texto).
#include <basicgol.h>

// Le notas ate a sentinela 0 e mostra a media. Com a sentinela logo na primeira
// leitura, o laço executa zero vezes e a media nao e calculada.
//
// Nota (Fase 6, DF-08): procedimento sem parametros se declara com (VAZIO), nao
// com () vazio. Em C, `()` significa "parametros nao especificados" (protótipo
// antigo, obsoleto no C23) e o Clang 20 recusa com -Wstrict-prototypes, que faz
// parte do conjunto adotado. GCC aceita em silencio; o Clang nao — e o codigo
// portatil e o que os dois aceitam.
PROCEDIMENTO mostra_media(VAZIO) {
    INTEIRO nota = 0;
    INTEIRO qtd = 0;
    INTEIRO soma = 0;

    ESCREVAL("Digite notas, 0 encerra:");
    SE NAO LEIA_INTEIRO(nota) ENTAO
        ERRO("entrada invalida");
    FIM_SE
    ENQUANTO nota != 0 FACA
        soma = soma + nota;
        qtd = qtd + 1;
        SE NAO LEIA_INTEIRO(nota) ENTAO
            ERRO("entrada invalida");
        FIM_SE
    FIM_ENQUANTO

    SE qtd == 0 ENTAO
        ESCREVAL("nenhuma nota");
    SENAO
        ESCREVA("media: ");
        ESCREVA_FMT("%.2f", soma / (qtd * 1.0));
        ESCREVA_LINHA();
    FIM_SE
}

INICIO
    INTEIRO escolha = 0;

    // 1) REPITA: pede ate receber um numero de 1 a 5. A condicao so e testada
    //    depois do corpo, entao a primeira tentativa sempre acontece.
    REPITA
        ESCREVA("Escolha um numero de 1 a 5: ");
        SE NAO LEIA_INTEIRO(escolha) ENTAO
            ESCREVAL("entrada invalida");
            escolha = 0;
        SENAO_SE escolha < 1 OU escolha > 5 ENTAO
            ESCREVAL("fora da faixa");
        FIM_SE
    ATE_QUE(escolha >= 1 E escolha <= 5);
    ESCREVA("Voce escolheu ");
    ESCREVAL(escolha);

    // 2) Sentinela: a rodada com notas 7 e 8 e depois 0.
    mostra_media();

    // 3) Sentinela logo de cara: zero iteracoes.
    mostra_media();
FIM
