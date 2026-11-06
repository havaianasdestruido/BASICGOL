/*
 * basicgol/caractere.h — classificação e conversão de caracteres ASCII.
 * Não depende de locale nem chama funções de <ctype.h>.
 */
#ifndef BASICGOL_CARACTERE_H
#define BASICGOL_CARACTERE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** true apenas para os bytes ASCII '0'...'9'. */
bool bg_e_digito(char caractere);
/** true apenas para as letras ASCII A-Z ou a-z. */
bool bg_e_letra(char caractere);
/** Converte ASCII a-z para A-Z; demais bytes são devolvidos sem alteração. */
char bg_caractere_maiusculo(char caractere);
/** Converte ASCII A-Z para a-z; demais bytes são devolvidos sem alteração. */
char bg_caractere_minusculo(char caractere);

#ifdef __cplusplus
}
#endif

#endif /* BASICGOL_CARACTERE_H */
