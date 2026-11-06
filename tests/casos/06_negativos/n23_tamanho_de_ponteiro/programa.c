/* n23 — TAMANHO_VETOR de um ponteiro (C-18).
 *
 * Compilar: cc -std=c11 -Wall -Wextra -Werror -I ../../../include programa.c -lm
 *
 * Esperado: aviso de -Wsizeof-pointer-div, que com -Werror é erro. Não é um
 * erro do cabeçalho: `sizeof(p) / sizeof(p[0])` é C válido, só não significa o
 * que o aluno quer. Os dois compiladores avisam:
 *   GCC 12:   division 'sizeof (int *) / sizeof (int)' does not compute the
 *             number of array elements [-Wsizeof-pointer-div]
 *   Clang 20: 'sizeof (p)' will return the size of the pointer, not the array
 *             itself [-Wsizeof-pointer-div]
 * Passe o vetor (não o ponteiro) ou guarde o tamanho numa constante.
 */
#include <basicgol.h>

INICIO
    VETOR(INTEIRO, v, 4);
    INTEIRO *p = v;

    v[0] = 1;
    ESCREVAL(TAMANHO_VETOR(p));
FIM
