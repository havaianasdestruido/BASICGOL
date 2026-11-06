/* n16 — LEIA nao tem formato para LOGICO.
 *
 * Compilar: cc -std=c11 -I ../../../include programa.c -lm
 *
 * Esperado: erro de compilacao. O _Generic de BG_SFMT nao define `default`,
 * entao um destino sem associação valida e violacao de restricao (C11
 * 6.5.1.1p3) — erro, nao aviso. Antes da Fase 6 o `default` era "%p", e o
 * scanf gravava 8 bytes dentro de um LOGICO de 1 byte (stack-buffer-overflow
 * detectado pelo ASan); quem barrava era o -Wformat, que so e erro com
 * -Werror.
 *
 * Ler um LOGICO seria ambiguo (0/1, FALSO/VERDADEIRO, V/N): a linguagem
 * prefere o erro de compilacao a adivinhar.
 */
#include <basicgol.h>

INICIO
    LOGICO ligado;

    LEIA(ligado);

    SE ligado
    ENTAO
        ESCREVAL("ligado");
    FIM_SE
FIM
