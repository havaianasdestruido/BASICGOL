// Propriedades de tipo provadas em tempo de compilação (Fase 4)
#include <basicgol.h>

// Cada _Static_assert falha o build se o mapeamento documentado mudar.
_Static_assert(_Generic((INTEIRO) 0, int: 1, default: 0), "INTEIRO deve ser int");
_Static_assert(_Generic((INTEIRO_CURTO) 0, short: 1, default: 0), "INTEIRO_CURTO");
_Static_assert(_Generic((INTEIRO_LONGO) 0, long: 1, default: 0), "INTEIRO_LONGO");
_Static_assert(_Generic((NATURAL) 0, unsigned: 1, default: 0), "NATURAL");
_Static_assert(_Generic((NATURAL_CURTO) 0, unsigned short: 1, default: 0), "NATURAL_CURTO");
_Static_assert(_Generic((NATURAL_LONGO) 0, unsigned long: 1, default: 0), "NATURAL_LONGO");
_Static_assert(_Generic((REAL_CURTO) 0, float: 1, default: 0), "REAL_CURTO");
_Static_assert(_Generic((REAL) 0, double: 1, default: 0), "REAL");
_Static_assert(_Generic((REAL_LONGO) 0, long double: 1, default: 0), "REAL_LONGO");
_Static_assert(_Generic((LOGICO) 0, _Bool: 1, default: 0), "LOGICO deve ser bool");
_Static_assert(_Generic((CARACTERE) 0, char: 1, default: 0), "CARACTERE deve ser char");
_Static_assert(_Generic((TEXTO) 0, const char *: 1, default: 0), "TEXTO");

_Static_assert(sizeof(INTEIRO8) == 1 && sizeof(INTEIRO16) == 2, "largura exata");
_Static_assert(sizeof(INTEIRO32) == 4 && sizeof(INTEIRO64) == 8, "largura exata");
_Static_assert(sizeof(NATURAL8) == 1 && sizeof(NATURAL64) == 8, "largura exata");
_Static_assert(sizeof(CARACTERE) == 1, "CARACTERE ocupa 1 byte");
_Static_assert(sizeof(LOGICO) >= 1, "LOGICO ocupa ao menos 1 byte");
_Static_assert(BITS_DE(INTEIRO16) == 16, "BITS_DE");
_Static_assert(TAMANHO_DE(INTEIRO64) == 8, "TAMANHO_DE");
_Static_assert(MAXIMO_INTEIRO == INT_MAX && MINIMO_INTEIRO == INT_MIN, "faixa de INTEIRO");
_Static_assert(MAXIMO_NATURAL == UINT_MAX, "faixa de NATURAL");
_Static_assert(MAXIMO_REAL == DBL_MAX, "faixa de REAL");
_Static_assert(EPSILON_REAL == DBL_EPSILON, "epsilon de REAL");

INICIO
    // larguras observáveis (imprimem 1)
    ESCREVAL(BITS_DE(INTEIRO) >= 16);
    ESCREVAL(BITS_DE(NATURAL) == BITS_DE(INTEIRO));
    ESCREVAL(BITS_DE(REAL) >= 32);
    ESCREVAL(sizeof(INTEIRO_CURTO) <= sizeof(INTEIRO));
    ESCREVAL(sizeof(INTEIRO) <= sizeof(INTEIRO_LONGO));
    ESCREVAL(sizeof(REAL_CURTO) <= sizeof(REAL));
    ESCREVAL(sizeof(REAL) <= sizeof(REAL_LONGO));

    // o sinal vem do tipo: tipos com sinal têm negativos, naturais não
    ESCREVAL((INTEIRO) -1 < 0);
    ESCREVAL((NATURAL) -1 > 0);

    // a promoção existe: CARACTERE e LOGICO viram INTEIRO em aritmética
    ESCREVAL(_Generic((CARACTERE) 'a' + 1, int: 1, default: 0));
    ESCREVAL(_Generic((LOGICO) 1 + 1, int: 1, default: 0));

    ESCREVAL("propriedades ok");
FIM
