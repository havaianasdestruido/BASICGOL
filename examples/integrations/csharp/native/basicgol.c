#include <basicgol.h>
#include "export.h"
#include <limits.h>
#include <stddef.h>
#include <string.h>

BG_EXPORT FUNCAO INTEIRO bg_soma(INTEIRO a, INTEIRO b)
{
    RETORNE a + b;
}

BG_EXPORT FUNCAO INTEIRO64 bg_fatorial(INTEIRO n)
{
    INTEIRO64 resultado = 1;
    INTEIRO i = 0;
    SE n < 0 OU n > 20 ENTAO
        RETORNE -1;
    FIM_SE
    PARA (i, 2, n) {
        resultado = resultado * i;
    }
    RETORNE resultado;
}

BG_EXPORT FUNCAO REAL bg_media3(REAL a, REAL b, REAL c)
{
    RETORNE (a + b + c) / 3.0;
}

BG_EXPORT FUNCAO INTEIRO bg_soma_vetor(const INTEIRO valores[], INTEIRO quantidade)
{
    INTEIRO soma = 0;
    INTEIRO i = 0;
    SE valores == NULL OU quantidade < 0 ENTAO
        RETORNE 0;
    FIM_SE
    PARA (i, 0, quantidade - 1) {
        soma = soma + valores[i];
    }
    RETORNE soma;
}

BG_EXPORT FUNCAO INTEIRO bg_comprimento_utf8(TEXTO texto)
{
    size_t bytes = texto == NULL ? 0u : strlen(texto);
    RETORNE bytes > (size_t) INT_MAX ? INT_MAX : (INTEIRO) bytes;
}

BG_EXPORT FUNCAO TEXTO bg_mensagem_utf8(VAZIO)
{
    RETORNE "Olá da biblioteca BASICGOL!";
}

typedef INTEIRO (*bg_callback)(INTEIRO valor);

BG_EXPORT FUNCAO INTEIRO bg_aplica_callback(INTEIRO valor, bg_callback callback)
{
    SE callback == NULL ENTAO
        RETORNE 0;
    FIM_SE
    RETORNE callback(valor);
}
