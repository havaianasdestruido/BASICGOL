/* n16 — LEIA nao tem formato para LOGICO.
 *
 * Compilar: cc -std=c11 -I ../../../include programa.c -lm
 *
 * Esperado: erro de -Wformat. O _Generic de BG_SFMT cai no `default`
 * ("%p"), e o compilador recusa passar um LOGICO* onde se espera void**.
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
