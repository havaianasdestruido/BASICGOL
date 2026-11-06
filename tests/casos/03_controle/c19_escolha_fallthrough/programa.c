// Queda intencional de um caso para o seguinte (sem PARE).
//
// Em C11 com -Wextra, a queda intencional precisa ser anotada. O comentário
// `/* fall through */` NÃO serve aqui: o rótulo `CASO` é uma macro e o GCC
// perde a anotação na expansão, continuando a avisar (-Wimplicit-fallthrough).
// A anotação que funciona nos dois compiladores é o atributo `fallthrough`,
// que este caso usa — antes da Fase 6 o caso desligava o aviso com
// `-Wno-implicit-fallthrough` em cflags.txt, escondendo também as quedas
// ACIDENTAIS.
#include <basicgol.h>

#if defined(__GNUC__) && (__GNUC__ >= 7)
#  define cai_para_proximo __attribute__((fallthrough))
#else
#  define cai_para_proximo ((void) 0)
#endif

INICIO
    ESCOLHA (2)
        CASO 1:
            ESCREVA("um ");
            cai_para_proximo;
        CASO 2:
            ESCREVA("dois ");
            cai_para_proximo;
        CASO 3:
            ESCREVA("tres ");
            PARE;
        CONTRARIO:
            ESCREVA("outro ");
    FIM_ESCOLHA
    ESCREVA_LINHA();
FIM
