/*
 * basicgol.h — linguagem BASICGOL, camada de macros sobre C11.
 *
 * BASICGOL 1.1.0  ·  Apache-2.0  ·  especificação: docs/01-especificacao.md
 *
 * Este cabeçalho define a linguagem e mantém a API curta compatível. A
 * biblioteca de execução é modular, em include/basicgol/ e src/biblioteca/;
 * programas que usam helpers devem ligar build/libbasicgol.a (ou os fontes).
 *
 *   make
 *   cc -std=c11 -Wall -Wextra -Wformat=2 -I include programa.c \
 *      build/libbasicgol.a -lm -o programa
 *
 * Configuração (definir ANTES do #include):
 *   BG_SEM_APELIDOS   — não define os nomes curtos (SE, ENQUANTO, ...),
 *                       só as formas prefixadas (BG_SE, BG_ENQUANTO, ...).
 *                       Use ao incluir este cabeçalho em projeto C existente.
 *   BG_USAR_EXTENSOES — liga a seção 2b (SOMA_SEGURA & cia.), a única parte
 *                       que usa extensão de compilador. Desligada, o
 *                       cabeçalho inteiro é C11 estrito. Ver D-22 em
 *                       docs/04-restricoes-e-decisoes.md.
 *
 * Fora da seção 2b, nada aqui depende de extensão de compilador.
 * Requer C11 (_Generic).
 */
#ifndef BASICGOL_H
#define BASICGOL_H

/* ------------------------------------------------------------------ versão */

#define BG_VERSAO_MAIOR    1
#define BG_VERSAO_MENOR    1
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
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

/* =====================================================================
 * 1. Biblioteca padrão modular (prefixo bg_)
 * ===================================================================== */

#include <basicgol/biblioteca.h>

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
#define BG_NATURAL_CURTO        unsigned short
#define BG_NATURAL_LONGO        unsigned long
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

/* --- tamanho, faixa e precisão (valores reais da plataforma) --- */
/* Não são "números da linguagem": são os de <limits.h>/<float.h> para os
 * tipos que a linguagem usa. `TAMANHO_DE`/`BITS_DE` devolvem NATURAL. */
#define BG_TAMANHO_DE(t)        ((size_t) sizeof(t))
#define BG_BITS_DE(t)           ((size_t) (sizeof(t) * CHAR_BIT))
#define BG_MAXIMO_INTEIRO       INT_MAX
#define BG_MINIMO_INTEIRO       INT_MIN
#define BG_MAXIMO_NATURAL       UINT_MAX
#define BG_MAXIMO_REAL          DBL_MAX
#define BG_EPSILON_REAL         DBL_EPSILON

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
 * (M1), então M0 e M1 concordam também no passo negativo. O passo zero é
 * erro de execução (bg_passo_zero), nunca um laço infinito. */
#define BG_PARA_PASSO(i, a, b, p)                                        \
    for ((i) = (a); ((p) > 0) ? ((i) <= (b))                             \
                    : (((p) == 0) ? bg_passo_zero(__FILE__, __LINE__)    \
                                  : ((i) >= (b)));                        \
         (i) += (p))

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

/* Lógica: formas prefixadas e normalização de verdade. Cada argumento é
 * avaliado UMA vez; o resultado é sempre LOGICO (0 ou 1).
 *
 * A normalização é `!= 0`, e não `!!`: as duas são equivalentes em resultado
 * (inclusive para NaN, que é "diferente de zero"), mas `!!` sobre um literal
 * real dispara -Wliteral-conversion no clang — e `LOGICO_DE(0.5)` é justamente
 * o caso de uso documentado. Com `!= 0` não há conversão implícita a bool. */
#define BG_OU_EXCLUSIVO(a, b)       ((bool) (((a) != 0) != ((b) != 0)))
#define BG_LOGICO_DE(x)             ((bool) ((x) != 0))

/* --- divisão e resto verificados --- */
/* DIVIDE/RESTO seguem as conversões usuais de C (o tipo do resultado é o tipo
 * promovido dos operandos), mas recusam divisor zero e quocientes assinados
 * não representáveis (em geral MIN/-1) com ERRO, em vez de comportamento
 * indefinido. `/` e MOD continuam sendo o C cru. */
#define BG_DIVIDE(a, b) _Generic((a) + (b),                                 \
    int: bg_divide_i, long: bg_divide_l, long long: bg_divide_ll,           \
    unsigned: bg_divide_u, unsigned long: bg_divide_ul,                     \
    unsigned long long: bg_divide_ull, float: bg_divide_f,                  \
    double: bg_divide_d, long double: bg_divide_ld,                         \
    default: bg_divide_d)((a), (b), __FILE__, __LINE__)

#define BG_RESTO(a, b) _Generic((a) + (b),                                  \
    int: bg_resto_i, long: bg_resto_l, long long: bg_resto_ll,              \
    unsigned: bg_resto_u, unsigned long: bg_resto_ul,                       \
    unsigned long long: bg_resto_ull)((a), (b), __FILE__, __LINE__)

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
#define BG_ACRESCENTA_TEXTO(d, t, o)    bg_acrescenta_texto((d), (size_t) (t), (o))
#define BG_TRECHO_TEXTO(d, t, o, i, q)  bg_trecho_texto((d), (size_t) (t), (o), \
                                                       (size_t) (i), (size_t) (q))
#define BG_COMPARA_TEXTO(a, b)          bg_compara_texto((a), (b))
#define BG_TEXTO_VAZIO(s)               bg_texto_vazio(s)
#define BG_FORMATA_TEXTO(d, t, ...)     bg_formata_texto((d), (size_t) (t), __VA_ARGS__)

#define BG_TEXTO_PARA_INTEIRO(s, p)     bg_texto_para_inteiro((s), (p))
#define BG_TEXTO_PARA_NATURAL(s, p)     bg_texto_para_natural((s), (p))
#define BG_TEXTO_PARA_REAL(s, p)        bg_texto_para_real((s), (p))
#define BG_REAL_PARA_INTEIRO(x, p)      bg_numero_para_inteiro((x), (p))

/* --- caractere (somente ASCII; independente de locale) --- */
#define BG_E_DIGITO(c)                  bg_e_digito(c)
#define BG_E_LETRA(c)                   bg_e_letra(c)
#define BG_CARACTERE_MAIUSCULO(c)       bg_caractere_maiusculo(c)
#define BG_CARACTERE_MINUSCULO(c)       bg_caractere_minusculo(c)

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

/* Sem `default:` de propósito (falha de tipo → erro de compilação).
 *
 * A versão anterior terminava em `default: "%p"`, e `%p` faz o scanf ESCREVER
 * um ponteiro no destino: em `LEIA(LOGICO)` isso gravava 8 bytes num objeto de
 * 1 byte (stack-buffer-overflow, visto no ASan) e em `LEIA(TEXTO_FIXO(...))`
 * gravava um endereço dentro do buffer. Quem pegava o problema era só o
 * `-Wformat` — um aviso, que a linha de compilação documentada no README não
 * transforma em erro.
 *
 * C11 6.5.1.1p3: sem `default`, um tipo de controle que não casa com nenhuma
 * associação é violação de restrição. Ou seja, o erro passa a ser de
 * compilação por padrão, com ou sem -Werror, nos dois compiladores testados:
 *   GCC:   '_Generic' selector of type '_Bool *' is not compatible with any association
 *   Clang: controlling expression type 'bool *' not compatible with any generic
 *          association type
 * Para ler texto use LEIA_TEXTO(buf, tam); para ler números/char, LEIA.
 * (BG_FMT, de saída, mantém `default: "%p"`: imprimir ponteiro é útil e não
 * escreve na memória do programa.) */
#define BG_SFMT(x) _Generic(&(x),                                      \
    int *: "%d", unsigned *: "%u", long *: "%ld", unsigned long *: "%lu", \
    long long *: "%lld", unsigned long long *: "%llu",                    \
    float *: "%f", double *: "%lf", long double *: "%Lf",                 \
    char *: " %c", signed char *: " %hhd", unsigned char *: "%hhu",       \
    short *: "%hd", unsigned short *: "%hu")

#define BG_ESCREVA(x)           printf(BG_FMT(x), (x))
#define BG_ESCREVAL(x)          (printf(BG_FMT(x), (x)), fputc('\n', stdout))
#define BG_ESCREVA_LINHA()      fputc('\n', stdout)
#define BG_ESCREVA_TEXTO(s)     bg_escreva_texto(stdout, (s))
#define BG_ESCREVA_FMT(...)     printf(__VA_ARGS__)
#define BG_ESCREVA_ERRO(s)      bg_escreva_texto(stderr, (s))
#define BG_LEIA(x)              ((void) (scanf(BG_SFMT(x), &(x)) == 1))
#define BG_LEIA_OK(x)           (scanf(BG_SFMT(x), &(x)) == 1)
#define BG_LEIA_TEXTO(b, t)     bg_leia_texto((b), (size_t) (t))
/* Leitura numérica por linha: consome a linha inteira, aceita exatamente um
 * número e devolve LOGICO. Não deixa lixo no stdin (ao contrário de LEIA). */
#define BG_LEIA_INTEIRO(x)      bg_leia_inteiro(&(x))
#define BG_LEIA_REAL(x)         bg_leia_real(&(x))

/* =====================================================================
 * 2b. Extensões de compilador — OPCIONAIS e ISOLADAS (Fase 4)
 * =====================================================================
 * Nada nesta seção existe por padrão. Definir BG_USAR_EXTENSOES antes do
 * #include habilita aritmética verificada via __builtin_*_overflow, que é
 * extensão de GCC/Clang. Sem suporte, o pedido falha com #error: preferimos
 * recusar a fingir portabilidade. O resto do cabeçalho não depende disto. */
#ifdef BG_USAR_EXTENSOES
#  if defined(__GNUC__) || defined(__clang__)
#    define BG_TEM_EXTENSOES 1
/* Todas operam em INTEIRO e devolvem LOGICO: true = operação exata. */
static inline bool bg_soma_segura(int a, int b, int *resultado)
{
    return !__builtin_add_overflow(a, b, resultado);
}

static inline bool bg_subtrai_segura(int a, int b, int *resultado)
{
    return !__builtin_sub_overflow(a, b, resultado);
}

static inline bool bg_multiplica_segura(int a, int b, int *resultado)
{
    return !__builtin_mul_overflow(a, b, resultado);
}

#    define BG_SOMA_SEGURA(a, b, p)         bg_soma_segura((a), (b), (p))
#    define BG_SUBTRAI_SEGURA(a, b, p)      bg_subtrai_segura((a), (b), (p))
#    define BG_MULTIPLICA_SEGURA(a, b, p)   bg_multiplica_segura((a), (b), (p))
#  else
#    error "BG_USAR_EXTENSOES exige __builtin_*_overflow (GCC ou Clang)."
#  endif
#endif /* BG_USAR_EXTENSOES */

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
#define NATURAL_CURTO       BG_NATURAL_CURTO
#define NATURAL_LONGO       BG_NATURAL_LONGO
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

#define TAMANHO_DE          BG_TAMANHO_DE
#define BITS_DE             BG_BITS_DE
#define MAXIMO_INTEIRO      BG_MAXIMO_INTEIRO
#define MINIMO_INTEIRO      BG_MINIMO_INTEIRO
#define MAXIMO_NATURAL      BG_MAXIMO_NATURAL
#define MAXIMO_REAL         BG_MAXIMO_REAL
#define EPSILON_REAL        BG_EPSILON_REAL

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
#define OU_EXCLUSIVO        BG_OU_EXCLUSIVO
#define LOGICO_DE           BG_LOGICO_DE
#define DIVIDE              BG_DIVIDE
#define RESTO               BG_RESTO

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
#define TEXTO_PARA_NATURAL  BG_TEXTO_PARA_NATURAL
#define TEXTO_PARA_REAL     BG_TEXTO_PARA_REAL
#define REAL_PARA_INTEIRO   BG_REAL_PARA_INTEIRO
#define ACRESCENTA_TEXTO    BG_ACRESCENTA_TEXTO
#define TRECHO_TEXTO        BG_TRECHO_TEXTO
#define COMPARA_TEXTO       BG_COMPARA_TEXTO
#define TEXTO_VAZIO         BG_TEXTO_VAZIO
#define FORMATA_TEXTO       BG_FORMATA_TEXTO

#define E_DIGITO            BG_E_DIGITO
#define E_LETRA             BG_E_LETRA
#define CARACTERE_MAIUSCULO BG_CARACTERE_MAIUSCULO
#define CARACTERE_MINUSCULO BG_CARACTERE_MINUSCULO

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
#define LEIA_INTEIRO        BG_LEIA_INTEIRO
#define LEIA_REAL           BG_LEIA_REAL

#ifdef BG_TEM_EXTENSOES
#define SOMA_SEGURA         BG_SOMA_SEGURA
#define SUBTRAI_SEGURA      BG_SUBTRAI_SEGURA
#define MULTIPLICA_SEGURA   BG_MULTIPLICA_SEGURA
#endif

#endif /* BG_SEM_APELIDOS */

#endif /* BASICGOL_H */
