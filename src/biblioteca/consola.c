/* consola.c — entrada, saída e diagnóstico da biblioteca BASICGOL. */
#include <basicgol/consola.h>
#include <basicgol/texto.h>

#include <errno.h>
#include <stdlib.h>

_Noreturn void bg_erro(const char *arquivo, int linha, const char *mensagem)
{
    (void) fflush(stdout);
    (void) fprintf(stderr, "%s:%d: erro: %s\n",
                   (arquivo != NULL) ? arquivo : "?",
                   linha,
                   (mensagem != NULL) ? mensagem : "");
    exit(1);
}

int bg_passo_zero(const char *arquivo, int linha)
{
    bg_erro(arquivo, linha, "PARA_PASSO: passo zero (o laco nunca avancaria)");
    return 0; /* inalcançável; mantém compatibilidade com compiladores C11 */
}

bool bg_leia_linha(FILE *entrada, char *destino, size_t capacidade)
{
    size_t usados = 0;
    bool leu_algum = false;
    bool truncou = false;
    bool invalida = false;
    bool falha_io = false;

    if (destino != NULL && capacidade > 0) {
        destino[0] = '\0';
    }
    if (entrada == NULL || destino == NULL || capacidade == 0) {
        return false;
    }

    for (;;) {
        int c = fgetc(entrada);

        if (c == EOF) {
            falha_io = ferror(entrada) != 0;
            break;
        }
        leu_algum = true;
        if (c == '\n') {
            break;
        }
        if (c == '\r') {
            int seguinte = fgetc(entrada);

            if (seguinte != '\n' && seguinte != EOF
                && ungetc(seguinte, entrada) == EOF) {
                falha_io = true;
            }
            if (seguinte == EOF && ferror(entrada)) {
                falha_io = true;
            }
            break;
        }
        if (c == '\0') {
            invalida = true;
            continue;
        }
        if (usados < capacidade - 1) {
            destino[usados++] = (char) c;
        } else {
            truncou = true;
        }
    }

    if (falha_io || !leu_algum || truncou || invalida) {
        destino[0] = '\0';
        return false;
    }
    destino[usados] = '\0';
    return true;
}

bool bg_leia_texto(char *destino, size_t capacidade)
{
    int c;

    if (destino != NULL && capacidade > 0) {
        destino[0] = '\0';
    }
    if (destino == NULL || capacidade == 0) {
        return false;
    }

    /* O formato scanf deixa normalmente o fim da linha por consumir. */
    c = getchar();
    if (c == '\n') {
        return bg_leia_linha(stdin, destino, capacidade);
    }
    if (c == '\r') {
        int seguinte = getchar();

        if (seguinte != '\n' && seguinte != EOF
            && ungetc(seguinte, stdin) == EOF) {
            return false;
        }
        if (seguinte == EOF && ferror(stdin)) {
            return false;
        }
        return bg_leia_linha(stdin, destino, capacidade);
    }
    if (c == EOF) {
        return false;
    }
    if (ungetc(c, stdin) == EOF) {
        return false;
    }
    return bg_leia_linha(stdin, destino, capacidade);
}

bool bg_leia_inteiro_de(FILE *entrada, int *saida)
{
    char linha[BG_TAMANHO_LINHA_NUMERO];

    if (entrada == NULL || saida == NULL
        || !bg_leia_linha(entrada, linha, sizeof linha)) {
        return false;
    }
    return bg_texto_para_inteiro(linha, saida);
}

bool bg_leia_real_de(FILE *entrada, double *saida)
{
    char linha[BG_TAMANHO_LINHA_NUMERO];

    if (entrada == NULL || saida == NULL
        || !bg_leia_linha(entrada, linha, sizeof linha)) {
        return false;
    }
    return bg_texto_para_real(linha, saida);
}

bool bg_leia_inteiro(int *saida)
{
    char linha[BG_TAMANHO_LINHA_NUMERO];

    if (saida == NULL || !bg_leia_texto(linha, sizeof linha)) {
        return false;
    }
    return bg_texto_para_inteiro(linha, saida);
}

bool bg_leia_real(double *saida)
{
    char linha[BG_TAMANHO_LINHA_NUMERO];

    if (saida == NULL || !bg_leia_texto(linha, sizeof linha)) {
        return false;
    }
    return bg_texto_para_real(linha, saida);
}

bool bg_escreva_texto(FILE *saida, const char *texto)
{
    if (saida == NULL || texto == NULL) {
        return false;
    }
    return fputs(texto, saida) >= 0;
}

bool bg_escreva_linha(FILE *saida, const char *texto)
{
    if (saida == NULL || texto == NULL || fputs(texto, saida) < 0) {
        return false;
    }
    return fputc('\n', saida) != EOF;
}
