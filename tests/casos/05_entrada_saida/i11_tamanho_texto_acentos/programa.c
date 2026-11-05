// TAMANHO_TEXTO conta BYTES: em UTF-8, 'ação' tem 4 caracteres e 6 bytes
#include <basicgol.h>

INICIO
    ESCREVAL(TAMANHO_TEXTO("abc"));     // 3
    ESCREVAL(TAMANHO_TEXTO(""));        // 0
    ESCREVAL(TAMANHO_TEXTO("ação"));    // 6 bytes: a + ç(2) + ã(2) + o
    ESCREVAL(TAMANHO_TEXTO(NULL));      // 0 (robusto)
FIM
