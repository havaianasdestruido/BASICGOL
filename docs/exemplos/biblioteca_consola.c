/*
 * biblioteca_consola.c — demonstra leitura limitada e saída literal.
 * Compilar a partir da raiz: make exemplos
 */
#include <basicgol/consola.h>

int main(void)
{
    char nome[32];

    if (!bg_escreva_texto(stdout, "Nome: ")) {
        return 1;
    }
    if (!bg_leia_linha(stdin, nome, sizeof nome)) {
        (void) bg_escreva_linha(stderr, "entrada ausente ou longa demais");
        return 1;
    }
    if (!bg_escreva_texto(stdout, "Ola, ")
        || !bg_escreva_linha(stdout, nome)) {
        return 1;
    }
    return 0;
}
