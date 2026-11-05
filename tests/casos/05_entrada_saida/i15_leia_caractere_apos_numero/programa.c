// LEIA(char) depois de LEIA(inteiro) não captura o '\n' pendente:
// a tabela de formatos usa " %c" (pula espaços em branco) — decisão da Fase 2.
#include <basicgol.h>

INICIO
    INTEIRO n = 0;
    CARACTERE c = '?';
    ESCREVA("numero: ");
    SE LEIA_OK(n) ENTAO
        ESCREVA("letra: ");
        SE LEIA_OK(c) ENTAO
            ESCREVA_FMT("lido %d e '%c'\n", n, c);
        SENAO
            ESCREVAL("sem letra");
        FIM_SE
    SENAO
        ESCREVAL("sem numero");
    FIM_SE
FIM
