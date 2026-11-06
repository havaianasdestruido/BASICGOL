/* texto.c — operações de texto limitadas e conversões da biblioteca. */
#include <basicgol/texto.h>

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t bg_tamanho_texto(const char *texto)
{
    return (texto == NULL) ? (size_t) 0 : strlen(texto);
}

bool bg_copia_texto(char *destino, size_t capacidade, const char *origem)
{
    size_t tamanho = bg_tamanho_texto(origem);

    if (destino == NULL || capacidade == 0) {
        return false;
    }
    if (tamanho >= capacidade) {
        destino[0] = '\0';
        return false;
    }
    if (tamanho > 0) {
        memmove(destino, origem, tamanho);
    }
    destino[tamanho] = '\0';
    return true;
}

bool bg_junta_texto(char *destino, size_t capacidade,
                    const char *primeiro, const char *segundo)
{
    size_t n1 = bg_tamanho_texto(primeiro);
    size_t n2 = bg_tamanho_texto(segundo);
    size_t total;
    char *temporario;

    if (destino == NULL || capacidade == 0) {
        return false;
    }
    if (n1 >= capacidade || n2 >= capacidade - n1) {
        destino[0] = '\0';
        return false;
    }
    total = n1 + n2; /* seguro: n1+n2 < capacidade */
    temporario = (char *) malloc(total + 1);
    if (temporario == NULL) {
        destino[0] = '\0';
        return false;
    }
    if (n1 > 0) {
        memcpy(temporario, primeiro, n1);
    }
    if (n2 > 0) {
        memcpy(temporario + n1, segundo, n2);
    }
    temporario[total] = '\0';
    memmove(destino, temporario, total + 1);
    free(temporario);
    return true;
}

bool bg_acrescenta_texto(char *destino, size_t capacidade, const char *origem)
{
    size_t n_destino = bg_tamanho_texto(destino);
    size_t n_origem = bg_tamanho_texto(origem);

    if (destino == NULL || capacidade == 0) {
        return false;
    }
    if (n_destino >= capacidade || n_origem >= capacidade - n_destino) {
        destino[0] = '\0';
        return false;
    }
    if (n_origem > 0) {
        memmove(destino + n_destino, origem, n_origem);
    }
    destino[n_destino + n_origem] = '\0';
    return true;
}

bool bg_trecho_texto(char *destino, size_t capacidade, const char *origem,
                     size_t inicio, size_t quantidade)
{
    size_t tamanho = bg_tamanho_texto(origem);

    if (destino == NULL || capacidade == 0) {
        return false;
    }
    if (inicio >= tamanho) {
        destino[0] = '\0';
        return true;
    }
    if (quantidade > tamanho - inicio) {
        quantidade = tamanho - inicio;
    }
    if (quantidade >= capacidade) {
        destino[0] = '\0';
        return false;
    }
    if (quantidade > 0) {
        memmove(destino, origem + inicio, quantidade);
    }
    destino[quantidade] = '\0';
    return true;
}

int bg_compara_texto(const char *primeiro, const char *segundo)
{
    int resultado;

    if (primeiro == NULL) {
        primeiro = "";
    }
    if (segundo == NULL) {
        segundo = "";
    }
    resultado = strcmp(primeiro, segundo);
    return (resultado < 0) ? -1 : ((resultado > 0) ? 1 : 0);
}

bool bg_texto_vazio(const char *texto)
{
    return bg_tamanho_texto(texto) == 0;
}

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wmissing-format-attribute"
#  pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
bool bg_formata_texto(char *destino, size_t capacidade,
                      const char *formato, ...)
{
    va_list argumentos;
    int escritos;

    if (destino == NULL || capacidade == 0) {
        return false;
    }
    destino[0] = '\0';
    if (formato == NULL) {
        return false;
    }

    va_start(argumentos, formato);
    escritos = vsnprintf(destino, capacidade, formato, argumentos);
    va_end(argumentos);
    if (escritos < 0 || (size_t) escritos >= capacidade) {
        destino[0] = '\0';
        return false;
    }
    return true;
}
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

static const char *bg_pula_espaco_ascii(const char *texto)
{
    while (*texto == ' ' || *texto == '\t' || *texto == '\n'
           || *texto == '\r' || *texto == '\f' || *texto == '\v') {
        ++texto;
    }
    return texto;
}

static const char *bg_pula_espaco_ascii_final(const char *texto)
{
    return bg_pula_espaco_ascii(texto);
}

bool bg_texto_para_inteiro(const char *texto, int *saida)
{
    const char *inicio;
    char *fim = NULL;
    long valor;
    int errno_anterior;
    int erro_conversao;

    if (texto == NULL || saida == NULL) {
        return false;
    }
    inicio = bg_pula_espaco_ascii(texto);
    errno_anterior = errno;
    errno = 0;
    valor = strtol(inicio, &fim, 10);
    erro_conversao = errno;
    errno = errno_anterior;
    if (fim == inicio) {
        return false;
    }
    fim = (char *) bg_pula_espaco_ascii_final(fim);
    if (*fim != '\0' || erro_conversao == ERANGE
        || valor < (long) INT_MIN || valor > (long) INT_MAX) {
        return false;
    }
    *saida = (int) valor;
    return true;
}

bool bg_texto_para_natural(const char *texto, unsigned *saida)
{
    const char *inicio;
    char *fim = NULL;
    unsigned long valor;
    int errno_anterior;
    int erro_conversao;

    if (texto == NULL || saida == NULL) {
        return false;
    }
    inicio = bg_pula_espaco_ascii(texto);
    if (*inicio == '-') {
        return false;
    }
    errno_anterior = errno;
    errno = 0;
    valor = strtoul(inicio, &fim, 10);
    erro_conversao = errno;
    errno = errno_anterior;
    if (fim == inicio) {
        return false;
    }
    fim = (char *) bg_pula_espaco_ascii_final(fim);
    if (*fim != '\0' || erro_conversao == ERANGE
        || valor > (unsigned long) UINT_MAX) {
        return false;
    }
    *saida = (unsigned) valor;
    return true;
}

bool bg_texto_para_real(const char *texto, double *saida)
{
    const char *inicio;
    char *fim = NULL;
    double valor;
    int errno_anterior;
    int erro_conversao;

    if (texto == NULL || saida == NULL) {
        return false;
    }
    inicio = bg_pula_espaco_ascii(texto);
    errno_anterior = errno;
    errno = 0;
    valor = strtod(inicio, &fim);
    erro_conversao = errno;
    errno = errno_anterior;
    if (fim == inicio) {
        return false;
    }
    fim = (char *) bg_pula_espaco_ascii_final(fim);
    if (*fim != '\0' || erro_conversao == ERANGE) {
        return false;
    }
    *saida = valor;
    return true;
}

bool bg_numero_para_inteiro(double valor, int *saida)
{
    long double ampliado;

    if (saida == NULL || valor != valor) {
        return false;
    }
    ampliado = (long double) valor;
    if (ampliado < (long double) INT_MIN || ampliado > (long double) INT_MAX) {
        return false;
    }
    *saida = (int) valor; /* conversão C: trunca em direção a zero */
    return true;
}
