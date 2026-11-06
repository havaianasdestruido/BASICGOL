/*
 * biblioteca_caractere.c — classificadores ASCII independentes de locale.
 * Compilar a partir da raiz: make exemplos
 */
#include <basicgol/caractere.h>

#include <stdio.h>

int main(void)
{
    char entrada = 'b';

    (void) printf("letra=%d digito=%d maiusculo=%c\n",
                  bg_e_letra(entrada), bg_e_digito('7'),
                  bg_caractere_maiusculo(entrada));
    (void) printf("minusculo=%c pontuacao=%d\n",
                  bg_caractere_minusculo('Z'), bg_e_letra('!'));
    return 0;
}
