/*
 * basicgol_depurador.h — o lado C do depurador opcional do playground.
 *
 * Nada aqui faz parte da linguagem BASICGOL. Este cabeçalho só existe no
 * playground, é injetado com `-include` **apenas no arquivo do usuário** e
 * **apenas quando o depurador está ligado** na página (ele vem desligado). Com
 * o depurador desligado, a compilação é exatamente a de antes: nenhuma linha
 * daqui entra no programa.
 *
 * Como funciona: a página instrumenta o fonte inserindo, no começo de cada
 * comando, uma chamada que descreve a linha e as variáveis visíveis:
 *
 *     if (bgdbg_abre(12)) { BGDBG_VAR("soma", soma); bgdbg_fecha(); } soma = soma + i;
 *
 * O quadro (frame) sai pelo descritor 3 — nunca pela saída do programa, que
 * continua byte a byte igual à de uma execução normal:
 *
 *     l 12                 linha do arquivo prestes a executar
 *     v soma inteiro 42    uma variável visível (nome, tipo, valor)
 *     v nome texto "Ana"
 *     e                    fim do quadro: o programa espera um comando
 *
 * A página responde um byte:
 *
 *     c   continue (ela não quer parar nesta linha)
 *     s   idem — quem decide parar é o lado de fora, nunca o programa
 *     x   desligue o depurador pelo resto da execução (limite de eventos)
 *     q   encerre o programa agora (botão Parar)
 *
 * Fora do playground não há ninguém no descritor 3: o `write` falha, o
 * depurador se desliga sozinho e o programa roda como se nada houvesse.
 */
#ifndef BASICGOL_DEPURADOR_H
#define BASICGOL_DEPURADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BGDBG_DESCRITOR     3   /* canal do depurador (nem stdout nem stderr) */
#define BGDBG_QUADRO     1024   /* tamanho máximo de um quadro, em bytes */
#define BGDBG_MAX_TEXTO    48   /* caracteres de TEXTO mostrados por variável */
#define BGDBG_SAIDA_PARADO 130  /* código de saída do botão Parar */

struct bgdbg_sessao {
    int desligado;
    int usado;
    char quadro[BGDBG_QUADRO];
};

/* O estado vive dentro da função: um cabeçalho injetado não deve criar
 * variável global em cada unidade de compilação. */
static inline struct bgdbg_sessao *bgdbg_sessao(void)
{
    static struct bgdbg_sessao sessao;
    return &sessao;
}

/** Acrescenta texto ao quadro; nova linha e retorno viram espaço. */
static inline void bgdbg_junta(const char *texto)
{
    struct bgdbg_sessao *s = bgdbg_sessao();
    int i = 0;
    while (texto[i] != '\0' && s->usado < BGDBG_QUADRO - 2) {
        char c = texto[i++];
        s->quadro[s->usado++] = (c == '\n' || c == '\r') ? ' ' : c;
    }
    s->quadro[s->usado] = '\0';
}

static inline void bgdbg_termina_linha(void)
{
    struct bgdbg_sessao *s = bgdbg_sessao();
    if (s->usado < BGDBG_QUADRO - 1) s->quadro[s->usado++] = '\n';
    s->quadro[s->usado] = '\0';
}

/** Abre o quadro da linha. Devolve 0 quando o depurador está desligado. */
static inline int bgdbg_abre(int linha)
{
    struct bgdbg_sessao *s = bgdbg_sessao();
    char cabecalho[32];
    if (s->desligado) return 0;
    s->usado = 0;
    s->quadro[0] = '\0';
    snprintf(cabecalho, sizeof cabecalho, "l %d", linha);
    bgdbg_junta(cabecalho);
    bgdbg_termina_linha();
    return 1;
}

/** Uma variável do quadro: `v <nome> <tipo> <valor>`. */
static inline void bgdbg_campo(const char *nome, const char *tipo, const char *valor)
{
    bgdbg_junta("v ");
    bgdbg_junta(nome);
    bgdbg_junta(" ");
    bgdbg_junta(tipo);
    bgdbg_junta(" ");
    bgdbg_junta(valor);
    bgdbg_termina_linha();
}

static inline void bgdbg_var_logico(const char *nome, int x)
{
    bgdbg_campo(nome, "logico", x ? "VERDADEIRO" : "FALSO");
}

static inline void bgdbg_var_caractere(const char *nome, int x)
{
    char b[24];
    if (x >= 32 && x < 127) snprintf(b, sizeof b, "'%c' (%d)", (char) x, x);
    else snprintf(b, sizeof b, "(%d)", x);
    bgdbg_campo(nome, "caractere", b);
}

static inline void bgdbg_var_inteiro(const char *nome, long long x)
{
    char b[32];
    snprintf(b, sizeof b, "%lld", x);
    bgdbg_campo(nome, "inteiro", b);
}

static inline void bgdbg_var_natural(const char *nome, unsigned long long x)
{
    char b[32];
    snprintf(b, sizeof b, "%llu", x);
    bgdbg_campo(nome, "natural", b);
}

static inline void bgdbg_var_real(const char *nome, double x)
{
    char b[48];
    snprintf(b, sizeof b, "%g", x);
    bgdbg_campo(nome, "real", b);
}

static inline void bgdbg_var_real_longo(const char *nome, long double x)
{
    bgdbg_var_real(nome, (double) x);
}

/* `%.*s` limita a leitura: um TEXTO_FIXO ainda sem terminador não faz o
 * depurador passar do fim do vetor. */
static inline void bgdbg_var_texto(const char *nome, const char *x)
{
    char b[BGDBG_MAX_TEXTO + 8];
    if (x == NULL) {
        bgdbg_campo(nome, "texto", "(nulo)");
        return;
    }
    snprintf(b, sizeof b, "\"%.*s\"", BGDBG_MAX_TEXTO, x);
    bgdbg_campo(nome, "texto", b);
}

/** Vetores, registros, ponteiros: o depurador mostra que existem. */
static inline void bgdbg_var_outro(const char *nome, ...)
{
    bgdbg_campo(nome, "outro", "(...)");
}

/* O tipo decide a função; o valor é avaliado uma única vez. */
#define BGDBG_VAR(nome, x) _Generic((x),                                      \
    _Bool: bgdbg_var_logico,                                                  \
    char: bgdbg_var_caractere,                                                \
    signed char: bgdbg_var_inteiro,     short: bgdbg_var_inteiro,             \
    int: bgdbg_var_inteiro,             long: bgdbg_var_inteiro,              \
    long long: bgdbg_var_inteiro,                                             \
    unsigned char: bgdbg_var_natural,   unsigned short: bgdbg_var_natural,    \
    unsigned int: bgdbg_var_natural,    unsigned long: bgdbg_var_natural,     \
    unsigned long long: bgdbg_var_natural,                                    \
    float: bgdbg_var_real,              double: bgdbg_var_real,               \
    long double: bgdbg_var_real_longo,                                        \
    char *: bgdbg_var_texto,            const char *: bgdbg_var_texto,        \
    default: bgdbg_var_outro)((nome), (x))

/** Fecha o quadro, manda para a página e espera o comando dela. */
static inline void bgdbg_fecha(void)
{
    struct bgdbg_sessao *s = bgdbg_sessao();
    char comando = 0;
    bgdbg_junta("e");
    bgdbg_termina_linha();
    if (write(BGDBG_DESCRITOR, s->quadro, (size_t) s->usado) != (ssize_t) s->usado) {
        s->desligado = 1; /* ninguém do outro lado: segue sem depurador */
        return;
    }
    if (read(BGDBG_DESCRITOR, &comando, 1) != 1) {
        s->desligado = 1;
        return;
    }
    if (comando == 'x') s->desligado = 1;
    else if (comando == 'q') exit(BGDBG_SAIDA_PARADO);
}

#endif /* BASICGOL_DEPURADOR_H */
