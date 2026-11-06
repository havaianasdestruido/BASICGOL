/* caractere.c — operações estritamente ASCII e independentes de locale. */
#include <basicgol/caractere.h>

bool bg_e_digito(char caractere)
{
    return caractere >= '0' && caractere <= '9';
}

bool bg_e_letra(char caractere)
{
    return (caractere >= 'A' && caractere <= 'Z')
        || (caractere >= 'a' && caractere <= 'z');
}

char bg_caractere_maiusculo(char caractere)
{
    return (caractere >= 'a' && caractere <= 'z')
        ? (char) (caractere - 'a' + 'A') : caractere;
}

char bg_caractere_minusculo(char caractere)
{
    return (caractere >= 'A' && caractere <= 'Z')
        ? (char) (caractere - 'A' + 'a') : caractere;
}
