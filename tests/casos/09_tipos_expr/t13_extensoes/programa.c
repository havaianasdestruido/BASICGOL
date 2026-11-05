// Extensões opcionais: só existem com BG_USAR_EXTENSOES e GCC/Clang (D-22)
#define BG_USAR_EXTENSOES
#include <basicgol.h>

#ifndef BG_TEM_EXTENSOES
#error "BG_USAR_EXTENSOES deveria definir BG_TEM_EXTENSOES"
#endif

INICIO
    INTEIRO r = 0;

    ESCREVAL(SOMA_SEGURA(2, 3, &r));
    ESCREVAL(r);
    ESCREVAL(SUBTRAI_SEGURA(2, 3, &r));
    ESCREVAL(r);

    // estouro: devolve FALSO e o resultado não deve ser usado
    ESCREVAL(SOMA_SEGURA(MAXIMO_INTEIRO, 1, &r));
    ESCREVAL(SUBTRAI_SEGURA(MINIMO_INTEIRO, 1, &r));
    ESCREVAL(MULTIPLICA_SEGURA(1000000, 1000000, &r));
    ESCREVAL(MULTIPLICA_SEGURA(3, 4, &r));
    ESCREVAL(r);
FIM
