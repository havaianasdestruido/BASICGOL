
// tipos de largura exata e limites garantidos
#include <basicgol.h>

INICIO
    INTEIRO8  pequeno = 100;
    INTEIRO16 medio = 30000;
    INTEIRO32 exato = 2000000000;
    INTEIRO64 grande = 9000000000;
    NATURAL8  byte = 250;

    // verificacoes portaveis (imprimem 1 quando verdadeiras)
    ESCREVAL(sizeof(INTEIRO8) == 1);
    ESCREVAL(sizeof(INTEIRO16) == 2);
    ESCREVAL(sizeof(INTEIRO32) == 4);
    ESCREVAL(sizeof(INTEIRO64) == 8);
    ESCREVAL(sizeof(CARACTERE) == 1);
    ESCREVAL(sizeof(INTEIRO) >= 2);
    ESCREVAL(sizeof(REAL) >= sizeof(REAL_CURTO));

    ESCREVAL(pequeno);
    ESCREVAL(medio);
    ESCREVAL(exato);
    ESCREVAL(grande);
    ESCREVAL(byte);
FIM
