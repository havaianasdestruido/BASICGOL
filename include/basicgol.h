/*
 * basicgol.h — linguagem BASICGOL, camada de macros sobre C11.
 *
 * BASICGOL 1.0.0  ·  Apache-2.0  ·  especificação: docs/01-especificacao.md
 *
 * Este cabeçalho é a implementação normativa da §3 de
 * docs/03-mapeamento-para-c.md. Ele é header-only: basta incluí-lo e compilar
 * o arquivo .c como C11 (a fonte BG é um .c comum, com este cabeçalho).
 *
 *   cc -std=c11 -Wall -Wextra -Wformat=2 programa.c -o programa -lm
 *
 * Configuração (definir ANTES do #include):
 *   BG_SEM_APELIDOS  — não define os nomes curtos (SE, ENQUANTO, ...),
 *                      só as formas prefixadas (BG_SE, BG_ENQUANTO, ...).
 *                      Use ao incluir este cabeçalho em projeto C existente.
 *
 * Nada aqui depende de extensão de compilador. Requer C11 (_Generic).
 */
#ifndef BASICGOL_H
#define BASICGOL_H

/* ------------------------------------------------------------------ versão */

#define BG_VERSAO_MAIOR    1
#define BG_VERSAO_MENOR    0
#define BG_VERSAO_CORRECAO 0

/* Condição de uso em _Static_assert: _Static_assert(BG_VERSAO_MINIMA(1,0), "...") */
#define BG_VERSAO_MINIMA(mai, men)                                  \
    (BG_VERSAO_MAIOR > (mai) ||                                     \
     (BG_VERSAO_MAIOR == (mai) && BG_VERSAO_MENOR >= (men)))

/* ------------------------------------------------------------- exigir C11 */

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)
#  if defined(_MSC_VER) && (_MSC_VER >= 1928) && defined(_MSVC_LANG)
     /* MSVC moderno em modo C: segue em frente */
#  else
#    error "BASICGOL exige C11 ou superior (_Generic). Compile com -std=c11 ou -std=c17."
#  endif
#endif

/* ------------------------------------------------------------- dependências */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* =====================================================================
 * 1. Biblioteca de execução (prefixo bg_)
 * ===================================================================== */

/* Erro fatal: imprime "arquivo:linha: erro: mensagem" em stderr e encerra
 * com status 1. Única função da biblioteca que interrompe o programa. */
static inline _Noreturn void bg_erro(const char *arquivo, int linha, const char *mensagem)
{
    fflush(stdout);
    fprintf(stderr, "%s:%d: erro: %s\n",
            arquivo ? arquivo : "?", linha, mensagem ? mensagem : "");
    exit(1);
}

/* Lê uma linha para buf.
 * 1) consome UM terminador de linha pendente, se (e somente se) houver;
 * 2) lê a linha com fgets e remove '\n'/'\r' finais;
 * 3) devolve false em EOF/erro; se a linha não coube, drena o resto da linha
 *    (para não dessincronizar a próxima leitura) e devolve false.
 * Contrato C-20 / RB-8 da especificação. */
static inline bool bg_leia_texto(char *buf, size_t tam)
{
    int c;
    size_t n;
    bool completa;

    if (buf == NULL || tam == 0) {
        return false;
    }

    c = getchar();
    if (c == '\r') {
        int d = getchar();
        if (d != '\n' && d != EOF) {
            (void) ungetc(d, stdin);
        }
    } else if (c != '\n' && c != EOF) {
        (void) ungetc(c, stdin);
    }

    if (fgets(buf, (int) tam, stdin) == NULL) {
        buf[0] = '\0';
        return false;
    }

    n = strlen(buf);
    completa = (n > 0 && buf[n - 1] == '\n');
    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) {
        buf[--n] = '\0';
    }

    if (!completa && !feof(stdin)) {
        int d;
        while ((d = getchar()) != EOF && d != '\n') {
            /* drena o resto da linha longa */
        }
        return false;              /* truncamento reportado, nunca silencioso */
    }
    return true;
}

/* Texto ------------------------------------------------------------------ */

static inline size_t bg_tamanho_texto(const char *s)
{
    return (s == NULL) ? (size_t) 0 : strlen(s);
}

static inline bool bg_copia_texto(char *dest, size_t tam, const char *origem)
{
    size_t n = bg_tamanho_texto(origem);

    if (dest == NULL || tam == 0) {
        return false;
    }
    if (n >= tam) {
        dest[0] = '\0';
        return false;
    }
    memcpy(dest, origem, n + 1);
    return true;
}

static inline bool bg_junta_texto(char *dest, size_t tam, const char *a, const char *b)
{
    size_t na = bg_tamanho_texto(a);
    size_t nb = bg_tamanho_texto(b);

    if (dest == NULL || tam == 0) {
        return false;
    }
    if (na + nb >= tam) {
        dest[0] = '\0';
        return false;
    }
    memcpy(dest, a, na);
    memcpy(dest + na, b, nb + 1);
    return true;
}

/* Conversões: só aceitam o texto INTEIRO (espaços finais tolerados) — RB-7. */

static inline bool bg_texto_para_inteiro(const char *s, int *saida)
{
    char *fim = NULL;
    long v;

    if (s == NULL || saida == NULL) {
        return false;
    }
    v = strtol(s, &fim, 10);
    if (fim == s) {
        return false;
    }
    while (*fim == ' ' || *fim == '\t') {
        fim++;
    }
    if (*fim != '\0') {
        return false;
    }
    *saida = (int) v;
    return true;
}

static inline bool bg_texto_para_real(const char *s, double *saida)
{
    char *fim = NULL;
    double v;

    if (s == NULL || saida == NULL) {
        return false;
    }
    v = strtod(s, &fim);
    if (fim == s) {
        return false;
    }
    while (*fim == ' ' || *fim == '\t') {
        fim++;
    }
    if (*fim != '\0') {
        return false;
    }
    *saida = v;
    return true;
}

/* Matemática com avaliação única (P-11) ----------------------------------- */

static inline int    bg_abs_i(int x)          { return (x < 0) ? -x : x; }
static inline long   bg_abs_l(long x)         { return (x < 0) ? -x : x; }
static inline long long bg_abs_ll(long long x){ return (x < 0) ? -x : x; }
static inline float  bg_abs_f(float x)        { return (x < 0.0f) ? -x : x; }
static inline double bg_abs_d(double x)       { return (x < 0.0) ? -x : x; }

static inline int    bg_min_i(int a, int b)             { return (a < b) ? a : b; }
static inline long   bg_min_l(long a, long b)           { return (a < b) ? a : b; }
static inline double bg_min_d(double a, double b)       { return (a < b) ? a : b; }
static inline int    bg_max_i(int a, int b)             { return (a > b) ? a : b; }
static inline long   bg_max_l(long a, long b)           { return (a > b) ? a : b; }
static inline double bg_max_d(double a, double b)       { return (a > b) ? a : b; }

static inline long long bg_round_l(double x)    { return (long long) llround(x); }
static inline int       bg_sorteia(int n)       { return (n <= 0) ? 0 : (int) (rand() % n); }
static inline void      bg_inicia_sorteio(unsigned s) { srand(s); }

/* =====================================================================
 * 2. Palavras-chave — formas prefixadas (sempre disponíveis)
 * ===================================================================== */

/* --- programa --- */
#define BG_INICIO               int main(void) {
#define BG_FIM                  }

/* --- tipos --- */
#define BG_LOGICO               bool
#define BG_CARACTERE            char
#define BG_INTEIRO_CURTO        short
#define BG_INTEIRO              int
#define BG_INTEIRO_LONGO        long
#define BG_INTEIRO8             int8_t
#define BG_INTEIRO16            int16_t
#define BG_INTEIRO32            int32_t
#define BG_INTEIRO64            int64_t
#define BG_NATURAL              unsigned
#define BG_NATURAL8             uint8_t
#define BG_NATURAL16            uint16_t
#define BG_NATURAL32            uint32_t
#define BG_NATURAL64            uint64_t
#define BG_REAL_CURTO           float
#define BG_REAL                 double
#define BG_REAL_LONGO           long double
#define BG_TEXTO                const char *
#define BG_VAZIO                void
#define BG_CONSTANTE            static const
#define BG_ESTATICO             static
#define BG_EXTERNO              extern

/* --- declarações --- */
#define BG_VETOR(t, n, k)       t n[k]
#define BG_TAMANHO_VETOR(v)     (sizeof(v) / sizeof((v)[0]))
#define BG_TEXTO_FIXO(n, k)     char n[k]
#define BG_REGISTRO(n)          typedef struct n {
#define BG_FIM_REGISTRO(n)      } n
#define BG_ENUMERACAO(n)        typedef enum n {
#define BG_FIM_ENUMERACAO(n)    } n

/* --- funções --- */
#define BG_FUNCAO
#define BG_PROCEDIMENTO         void
#define BG_RETORNE              return

/* --- controle --- */
#define BG_SE                   if (
#define BG_ENTAO                ) {
#define BG_SENAO_SE             } else if (
#define BG_SENAO                } else {
#define BG_FIM_SE               }
#define BG_ENQUANTO             while (
#define BG_FACA                 ) {
#define BG_FIM_ENQUANTO         }
#define BG_REPITA               do {
#define BG_ATE_QUE(c)           } while (!(c))
#define BG_ESCOLHA(e)           switch (e) {
#define BG_CASO                 case
#define BG_CONTRARIO            default
#define BG_FIM_ESCOLHA          }
#define BG_PARE                 break
#define BG_PROSSIGA             continue
#define BG_PARA(i, a, b)            for ((i) = (a); (i) <= (b); ++(i))
/* O passo negativo conta para trás: é o mesmo teste que o transpilador emite
 * (M1), então M0 e M1 concordam também no passo negativo. */
#define BG_PARA_PASSO(i, a, b, p)                                        \
    for ((i) = (a); ((p) > 0) ? ((i) <= (b)) : ((i) >= (b)); (i) += (p))

/* --- operadores por palavra --- */
#define BG_E                    &&
#define BG_OU                   ||
#define BG_NAO                  !
#define BG_MOD                  %
#define BG_VERDADEIRO           true
#define BG_FALSO                false
#define BG_IGUAL(a, b)              ((a) == (b))
#define BG_DIFERENTE(a, b)          ((a) != (b))
#define BG_TEXTOS_IGUAIS(a, b)      (strcmp((a), (b)) == 0)
#define BG_TEXTOS_DIFERENTES(a, b)  (strcmp((a), (b)) != 0)

/* --- matemática --- */
#define BG_ABS(x) _Generic((x),                     \
    int: bg_abs_i, long: bg_abs_l, long long: bg_abs_ll, \
    float: bg_abs_f, double: bg_abs_d, default: bg_abs_d)(x)

#define BG_MINIMO(a, b) _Generic((a) + (b),         \
    int: bg_min_i, long: bg_min_l, default: bg_min_d)((a), (b))

#define BG_MAXIMO(a, b) _Generic((a) + (b),         \
    int: bg_max_i, long: bg_max_l, default: bg_max_d)((a), (b))

#define BG_POTENCIA(base, expoente) pow((base), (expoente))
#define BG_RAIZ(x)                  sqrt(x)
#define BG_ARREDONDA(x)             bg_round_l((double) (x))
#define BG_PISO(x)                  floor(x)
#define BG_TETO(x)                  ceil(x)
#define BG_SORTEIA(n)               bg_sorteia(n)
#define BG_INICIA_SORTEIO(s)        bg_inicia_sorteio((unsigned) (s))

/* --- texto --- */
#define BG_TAMANHO_TEXTO(s)             bg_tamanho_texto(s)
#define BG_COPIA_TEXTO(d, t, o)         bg_copia_texto((d), (size_t) (t), (o))
#define BG_JUNTA_TEXTO(d, t, a, b)      bg_junta_texto((d), (size_t) (t), (a), (b))
#define BG_TEXTO_PARA_INTEIRO(s, p)     bg_texto_para_inteiro((s), (p))
#define BG_TEXTO_PARA_REAL(s, p)        bg_texto_para_real((s), (p))

/* --- erros --- */
#define BG_ERRO(msg)            bg_erro(__FILE__, __LINE__, (msg))
#define BG_AFIRME(c, msg)       ((c) ? (void) 0 : bg_erro(__FILE__, __LINE__, (msg)))

/* --- entrada e saída --- */

/* Tabela de formatos por tipo, resolvida em tempo de compilação. */
#define BG_FMT(x) _Generic((x),                                     \
    _Bool: "%d", char: "%c", signed char: "%hhd", unsigned char: "%hhu", \
    short: "%hd", unsigned short: "%hu", int: "%d", unsigned int: "%u", \
    long: "%ld", unsigned long: "%lu", long long: "%lld",               \
    unsigned long long: "%llu", float: "%g", double: "%g",              \
    long double: "%Lg", char *: "%s", const char *: "%s", default: "%p")

#define BG_SFMT(x) _Generic(&(x),                                   \
    int *: "%d", unsigned *: "%u", long *: "%ld", unsigned long *: "%lu", \
    long long *: "%lld", unsigned long long *: "%llu",                    \
    float *: "%f", double *: "%lf", char *: " %c", signed char *: " %hhd", \
    short *: "%hd", default: "%p")

#define BG_ESCREVA(x)           printf(BG_FMT(x), (x))
#define BG_ESCREVAL(x)          (printf(BG_FMT(x), (x)), fputc('\n', stdout))
#define BG_ESCREVA_LINHA()      fputc('\n', stdout)
#define BG_ESCREVA_TEXTO(s)     printf("%s", (s))
#define BG_ESCREVA_FMT(...)     printf(__VA_ARGS__)
#define BG_ESCREVA_ERRO(s)      fprintf(stderr, "%s", (s))
#define BG_LEIA(x)              ((void) (scanf(BG_SFMT(x), &(x)) == 1))
#define BG_LEIA_OK(x)           (scanf(BG_SFMT(x), &(x)) == 1)
#define BG_LEIA_TEXTO(b, t)     bg_leia_texto((b), (size_t) (t))

/* =====================================================================
 * 3. Nomes curtos (a sintaxe canônica) — desligáveis por BG_SEM_APELIDOS
 * ===================================================================== */

#ifndef BG_SEM_APELIDOS

#define INICIO              BG_INICIO
#define FIM                 BG_FIM

#define LOGICO              BG_LOGICO
#define CARACTERE           BG_CARACTERE
#define INTEIRO_CURTO       BG_INTEIRO_CURTO
#define INTEIRO             BG_INTEIRO
#define INTEIRO_LONGO       BG_INTEIRO_LONGO
#define INTEIRO8            BG_INTEIRO8
#define INTEIRO16           BG_INTEIRO16
#define INTEIRO32           BG_INTEIRO32
#define INTEIRO64           BG_INTEIRO64
#define NATURAL             BG_NATURAL
#define NATURAL8            BG_NATURAL8
#define NATURAL16           BG_NATURAL16
#define NATURAL32           BG_NATURAL32
#define NATURAL64           BG_NATURAL64
#define REAL_CURTO          BG_REAL_CURTO
#define REAL                BG_REAL
#define REAL_LONGO          BG_REAL_LONGO
#define TEXTO               BG_TEXTO
#define VAZIO               BG_VAZIO
#define CONSTANTE           BG_CONSTANTE
#define ESTATICO            BG_ESTATICO
#define EXTERNO             BG_EXTERNO

#define VETOR               BG_VETOR
#define TAMANHO_VETOR       BG_TAMANHO_VETOR
#define TEXTO_FIXO          BG_TEXTO_FIXO
#define REGISTRO            BG_REGISTRO
#define FIM_REGISTRO        BG_FIM_REGISTRO
#define ENUMERACAO          BG_ENUMERACAO
#define FIM_ENUMERACAO      BG_FIM_ENUMERACAO

#define FUNCAO              BG_FUNCAO
#define PROCEDIMENTO        BG_PROCEDIMENTO
#define RETORNE             BG_RETORNE

#define SE                  BG_SE
#define ENTAO               BG_ENTAO
#define SENAO_SE            BG_SENAO_SE
#define SENAO               BG_SENAO
#define FIM_SE              BG_FIM_SE
#define ENQUANTO            BG_ENQUANTO
#define FACA                BG_FACA
#define FIM_ENQUANTO        BG_FIM_ENQUANTO
#define REPITA              BG_REPITA
#define ATE_QUE             BG_ATE_QUE
#define ESCOLHA             BG_ESCOLHA
#define CASO                BG_CASO
#define CONTRARIO           BG_CONTRARIO
#define FIM_ESCOLHA         BG_FIM_ESCOLHA
#define PARE                BG_PARE
#define PROSSIGA            BG_PROSSIGA
#define PARA                BG_PARA
#define PARA_PASSO          BG_PARA_PASSO

#define E                   BG_E
#define OU                  BG_OU
#define NAO                 BG_NAO
#define MOD                 BG_MOD
#define VERDADEIRO          BG_VERDADEIRO
#define FALSO               BG_FALSO
#define IGUAL               BG_IGUAL
#define DIFERENTE           BG_DIFERENTE
#define TEXTOS_IGUAIS       BG_TEXTOS_IGUAIS
#define TEXTOS_DIFERENTES   BG_TEXTOS_DIFERENTES

#define ABS                 BG_ABS
#define MINIMO              BG_MINIMO
#define MAXIMO              BG_MAXIMO
#define POTENCIA            BG_POTENCIA
#define RAIZ                BG_RAIZ
#define ARREDONDA           BG_ARREDONDA
#define PISO                BG_PISO
#define TETO                BG_TETO
#define SORTEIA             BG_SORTEIA
#define INICIA_SORTEIO      BG_INICIA_SORTEIO

#define TAMANHO_TEXTO       BG_TAMANHO_TEXTO
#define COPIA_TEXTO         BG_COPIA_TEXTO
#define JUNTA_TEXTO         BG_JUNTA_TEXTO
#define TEXTO_PARA_INTEIRO  BG_TEXTO_PARA_INTEIRO
#define TEXTO_PARA_REAL     BG_TEXTO_PARA_REAL

#define ERRO                BG_ERRO
#define AFIRME              BG_AFIRME

#define ESCREVA             BG_ESCREVA
#define ESCREVAL            BG_ESCREVAL
#define ESCREVA_LINHA       BG_ESCREVA_LINHA
#define ESCREVA_TEXTO       BG_ESCREVA_TEXTO
#define ESCREVA_FMT         BG_ESCREVA_FMT
#define ESCREVA_ERRO        BG_ESCREVA_ERRO
#define LEIA                BG_LEIA
#define LEIA_OK             BG_LEIA_OK
#define LEIA_TEXTO          BG_LEIA_TEXTO

#endif /* BG_SEM_APELIDOS */

#endif /* BASICGOL_H */
