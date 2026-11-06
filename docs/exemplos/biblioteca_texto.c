/*
 * biblioteca_texto.c — cópia limitada, concatenação, trecho e conversão.
 * Compilar a partir da raiz: make exemplos
 */
#include <basicgol/texto.h>

#include <stdio.h>

int main(void)
{
    char texto[32];
    char trecho[8];
    int codigo = 0;

    if (!bg_copia_texto(texto, sizeof texto, "BASIC")
        || !bg_acrescenta_texto(texto, sizeof texto, "GOL")
        || !bg_trecho_texto(trecho, sizeof trecho, texto, 5, 3)
        || !bg_texto_para_inteiro("42", &codigo)) {
        return 1;
    }
    (void) printf("texto: %s (%zu bytes)\n", texto, bg_tamanho_texto(texto));
    (void) printf("trecho: %s\n", trecho);
    (void) printf("codigo: %d\n", codigo);
    return 0;
}
