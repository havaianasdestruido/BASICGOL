#include <basicgol.h>
#include <stddef.h>

FUNCAO INTEIRO bg_asm_soma_vetor(const INTEIRO valores[], size_t quantidade);

INICIO
    INTEIRO valores[] = { 4, 8, 15, 16, 23, 42 };
    ESCREVA_FMT("Soma do vetor: %d\n",
                bg_asm_soma_vetor(valores, TAMANHO_VETOR(valores)));
FIM
