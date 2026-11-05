/* comum.h — declaracoes compartilhadas entre os modulos do programa.
 * Em BASICGOL, um modulo exporta com FUNCAO/PROCEDIMENTO normal e o outro
 * lado declara com EXTERNO (equivale a declarar o prototipo). */
#ifndef COMUM_H
#define COMUM_H

#include <basicgol.h>

EXTERNO FUNCAO INTEIRO mdc(INTEIRO a, INTEIRO b);
EXTERNO FUNCAO INTEIRO mmc(INTEIRO a, INTEIRO b);
EXTERNO FUNCAO LOGICO eh_primo(INTEIRO n);
EXTERNO PROCEDIMENTO tabuada(INTEIRO n);

#endif
