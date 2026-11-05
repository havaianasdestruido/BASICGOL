// Divisão por zero inteira é indefinida em C; DIVIDE a converte em erro
#include <basicgol.h>

INICIO
    INTEIRO a = 10;
    INTEIRO b = 0;
    INTEIRO c = DIVIDE(a, b);
    ESCREVAL("nao deveria aparecer");
    ESCREVAL(c);
FIM
