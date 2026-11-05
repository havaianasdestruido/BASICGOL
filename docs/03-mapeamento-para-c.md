# BASICGOL → C: tabela de mapeamento

Este documento é normativo: define **exatamente** em que cada construtor BASICGOL se
transforma. Qualquer implementação (o cabeçalho `basicgol.h` da Fase 2 ou o transpilador
`basicgolc`) deve produzir o C descrito aqui, e nada além.

Legenda de perfil: **N** = núcleo, funciona com macros (M0) · **X** = extensão, exige
`basicgolc` (M1) · **C** = C nativo, sem tradução.

---

## 1. Como este mapeamento foi verificado

Antes de congelar a especificação, cada construtor foi reduzido a macros em uma **sonda
descartável** (não versionada: a implementação é entrega da Fase 2) e compilada com:

```sh
gcc 12.2 -std=c11 -Wall -Wextra -Wformat=2 -x c programa.bg -I <sonda> -lm
```

Resultado: compilação limpa (zero avisos) e comportamento esperado para todos os
construtores **N** aqui listados. Duas descobertas dessa validação mudaram o
desenho da linguagem e estão registradas em [04](04-restricoes-e-decisoes.md):

* **`SENAO SE` não funciona** → foi criada a palavra-chave `SENAO_SE` (C-05).
* **`LEIA` + `LEIA_TEXTO` deixam `\n` pendente no `stdin`** → `LEIA_TEXTO` passou a ter a
  obrigação contratual de descartar o resto da linha (C-20).

Os trechos de saída de pré-processador mostrados na §4 são reais (com a tabela de
`_Generic` abreviada por `…` para caber na página).

---

## 2. Tabela mestra

### 2.1 Programa, funções e tipos

| # | Sintaxe BASICGOL | C equivalente | Perfil |
|---|---|---|---|
| 1 | `INICIO` | `int main(void) {` | N |
| 2 | `FIM` | `}` | N |
| 3 | `FUNCAO INTEIRO soma(INTEIRO a, INTEIRO b) { … }` | `int soma(int a, int b) { … }` (`FUNCAO` = vazio) | N |
| 4 | `PROCEDIMENTO p(INTEIRO n) { … }` | `void p(int n) { … }` | N |
| 5 | `RETORNE expr;` | `return expr;` | N |
| 6 | `RETORNE;` | `return;` | N |
| 7 | `ESTATICO FUNCAO …` | `static int …` | N |
| 8 | `EXTERNO FUNCAO REAL media(REAL, REAL);` | `extern double media(double, double);` | N |
| 9 | `INTEIRO x = 0;` | `int x = 0;` | N |
| 10 | `REAL r = 1.5;` | `double r = 1.5;` | N |
| 11 | `LOGICO ok = VERDADEIRO;` | `bool ok = true;` | N |
| 12 | `TEXTO s = "oi";` | `const char *s = "oi";` | N |
| 13 | `CARACTERE c = 'A';` | `char c = 'A';` | N |
| 14 | `VAZIO` | `void` | N |
| 15 | `INTEIRO8/16/32/64` / `NATURAL8/16/32/64` | `int8_t`…`int64_t` / `uint8_t`…`uint64_t` | N |
| 16 | `CONSTANTE INTEIRO k_n = 10;` | `static const int k_n = 10;` | N |
| 17 | `EXTERNO INTEIRO total;` | `extern int total;` | N |
| 18 | `(REAL) contador` | `(double) contador` | C |
| 19 | `INTEIRO v[]` (em parâmetro) | `int *v` | N/C |

### 2.2 Declarações compostas

| # | Sintaxe BASICGOL | C equivalente | Perfil |
|---|---|---|---|
| 20 | `VETOR(INTEIRO, notas, 4);` | `int notas[4];` | N |
| 21 | `VETOR(TEXTO, nomes, 10);` | `const char *nomes[10];` | N |
| 22 | `TEXTO_FIXO(nome, 40);` | `char nome[40];` | N |
| 23 | `TAMANHO_VETOR(notas)` | `(sizeof(notas) / sizeof((notas)[0]))` | N |
| 24 | `REGISTRO(Aluno) … FIM_REGISTRO(Aluno);` | `typedef struct Aluno { … } Aluno;` | N |
| 25 | `ENUMERACAO(Conceito) A, B FIM_ENUMERACAO(Conceito);` | `typedef enum Conceito { A, B } Conceito;` | N |
| 26 | `aluno.nota` | `aluno.nota` | C |
| 27 | `notas[i]` | `notas[i]` | C |

### 2.3 Controle de fluxo

| # | Sintaxe BASICGOL | C equivalente | Perfil |
|---|---|---|---|
| 28 | `SE c ENTAO` | `if ( c ) {` | N |
| 29 | `SENAO_SE c ENTAO` | `} else if ( c ) {` | N |
| 30 | `SENAO` | `} else {` | N |
| 31 | `FIM_SE` | `}` | N |
| 32 | `ENQUANTO c FACA` | `while ( c ) {` | N |
| 33 | `FIM_ENQUANTO` | `}` | N |
| 34 | `REPITA` | `do {` | N |
| 35 | `ATE_QUE(c);` | `} while (!(c));` | N |
| 36 | `PARA (i, 1, 10) { … }` | `for ((i) = (1); (i) <= (10); ++(i)) { … }` | N |
| 37 | `PARA_PASSO (i, 0, 100, 10) { … }` | `for ((i) = (0); (i) <= (100); (i) += (10)) { … }` | N |
| 38 | `PARA i DE 1 ATE 10 FACA … FIM_PARA` | `for (i = (1); (1) > 0 ? (i <= (10)) : (i >= (10)); i += (1)) { … }` | **X** |
| 39 | `ESCOLHA(e)` | `switch (e) {` | N |
| 40 | `CASO 2:` | `case 2:` | N |
| 41 | `CONTRARIO:` | `default:` | N |
| 42 | `FIM_ESCOLHA` | `}` | N |
| 43 | `PARE;` | `break;` | N |
| 44 | `PROSSIGA;` | `continue;` | N |
| 45 | `se/caso` com bloco próprio: `CASO 3: { … }` | idêntico | C |

### 2.4 Expressões

| # | Sintaxe BASICGOL | C equivalente | Perfil |
|---|---|---|---|
| 46 | `a E b` | `a && b` | N |
| 47 | `a OU b` | `a \|\| b` | N |
| 48 | `NAO a` | `!a` | N |
| 49 | `a MOD b` | `a % b` | N |
| 50 | `VERDADEIRO` / `FALSO` | `true` / `false` | N |
| 51 | `IGUAL(a, b)` | `((a) == (b))` | N |
| 52 | `DIFERENTE(a, b)` | `((a) != (b))` | N |
| 53 | `TEXTOS_IGUAIS(a, b)` | `(strcmp((a), (b)) == 0)` | N |
| 54 | `TEXTOS_DIFERENTES(a, b)` | `(strcmp((a), (b)) != 0)` | N |
| 55 | `ABS(x)` | `_Generic((x), int: bg_abs_i, long: bg_abs_l, float: bg_abs_f, double: bg_abs_d, default: bg_abs_d)(x)` | N |
| 56 | `MINIMO(a, b)` / `MAXIMO(a, b)` | despacho `_Generic` para `bg_min_*`/`bg_max_*` | N |
| 57 | `POTENCIA(b, e)` | `pow((b), (e))` | N |
| 58 | `RAIZ(x)` | `sqrt(x)` | N |
| 59 | `ARREDONDA(x)` | `bg_round_l((double)(x))` | N |
| 60 | `PISO(x)` / `TETO(x)` | `floor(x)` / `ceil(x)` | N |
| 61 | `TAMANHO_TEXTO(s)` | `bg_tamanho_texto(s)` | N |
| 62 | `SORTEIA(n)` | `bg_sorteia(n)` | N |
| 63 | `INICIA_SORTEIO(s)` | `bg_inicia_sorteio((unsigned)(s))` | N |

### 2.5 Entrada e saída

| # | Sintaxe BASICGOL | C equivalente | Perfil |
|---|---|---|---|
| 64 | `ESCREVA(x);` | `printf(_Generic((x), …), (x))` (formato por tipo) | N |
| 65 | `ESCREVAL(x);` | `(printf(_Generic((x), …), (x)), fputc('\n', stdout))` | N |
| 66 | `ESCREVA_LINHA();` | `fputc('\n', stdout)` | N |
| 67 | `ESCREVA_TEXTO(s);` | `printf("%s", (s))` | N |
| 68 | `ESCREVA_FMT(fmt, …);` | `printf(__VA_ARGS__)` | N |
| 69 | `ESCREVA_ERRO(s);` | `fprintf(stderr, "%s", (s))` | N |
| 70 | `LEIA(x);` | `((void)(scanf(_Generic(&(x), …), &(x)) == 1))` | N |
| 71 | `LEIA_OK(x);` | `(scanf(_Generic(&(x), …), &(x)) == 1)` | N |
| 72 | `LEIA_TEXTO(buf, tam);` | `bg_leia_texto((buf), (size_t)(tam))` | N |

### 2.6 Erros

| # | Sintaxe BASICGOL | C equivalente | Perfil |
|---|---|---|---|
| 73 | `ERRO("msg");` | `bg_erro(__FILE__, __LINE__, ("msg"))` | N |
| 74 | `AFIRME(c, "msg");` | `((c) ? (void)0 : bg_erro(__FILE__, __LINE__, ("msg")))` | N |

---

## 3. Definições normativas das macros

Trecho normativo do cabeçalho (a implementação completa, com nomes prefixados,
detecção de compilador e documentação embutida, é entrega da Fase 2). Esta é a forma
**verificada**:

```c
/* ---- programa ---- */
#define INICIO              int main(void) {
#define FIM                 }

/* ---- tipos ---- */
#define LOGICO              bool
#define CARACTERE           char
#define INTEIRO_CURTO       short
#define INTEIRO             int
#define INTEIRO_LONGO       long
#define INTEIRO8            int8_t
#define INTEIRO16           int16_t
#define INTEIRO32           int32_t
#define INTEIRO64           int64_t
#define NATURAL             unsigned
#define NATURAL8            uint8_t
#define NATURAL16           uint16_t
#define NATURAL32           uint32_t
#define NATURAL64           uint64_t
#define REAL_CURTO          float
#define REAL                double
#define REAL_LONGO          long double
#define TEXTO               const char *
#define VAZIO               void
#define CONSTANTE           static const
#define ESTATICO            static
#define EXTERNO             extern

/* ---- declarações ---- */
#define VETOR(t, n, k)      t n[k]
#define TAMANHO_VETOR(v)    (sizeof(v) / sizeof((v)[0]))
#define TEXTO_FIXO(n, k)    char n[k]
#define REGISTRO(n)         typedef struct n {
#define FIM_REGISTRO(n)     } n
#define ENUMERACAO(n)       typedef enum n {
#define FIM_ENUMERACAO(n)   } n

/* ---- funções ---- */
#define FUNCAO
#define PROCEDIMENTO        void
#define RETORNE             return

/* ---- controle ---- */
#define SE                  if (
#define ENTAO               ) {
#define SENAO_SE            } else if (
#define SENAO               } else {
#define FIM_SE              }
#define ENQUANTO            while (
#define FACA                ) {
#define FIM_ENQUANTO        }
#define REPITA              do {
#define ATE_QUE(c)          } while (!(c))
#define ESCOLHA(e)          switch (e) {
#define CASO                case
#define CONTRARIO           default
#define FIM_ESCOLHA         }
#define PARE                break
#define PROSSIGA            continue
#define PARA(i, a, b)           for ((i) = (a); (i) <= (b); ++(i))
#define PARA_PASSO(i, a, b, p)  for ((i) = (a); (i) <= (b); (i) += (p))

/* ---- operadores por palavra ---- */
#define E                   &&
#define OU                  ||
#define NAO                 !
#define MOD                 %
#define VERDADEIRO          true
#define FALSO               false
#define IGUAL(a, b)             ((a) == (b))
#define DIFERENTE(a, b)         ((a) != (b))
#define TEXTOS_IGUAIS(a, b)     (strcmp((a), (b)) == 0)
#define TEXTOS_DIFERENTES(a, b) (strcmp((a), (b)) != 0)

/* ---- E/S ---- */
#define BG_FMT(x) _Generic((x), \
    _Bool: "%d", char: "%c", signed char: "%hhd", unsigned char: "%hhu", \
    short: "%hd", unsigned short: "%hu", int: "%d", unsigned int: "%u", \
    long: "%ld", unsigned long: "%lu", long long: "%lld", \
    unsigned long long: "%llu", float: "%g", double: "%g", \
    long double: "%Lg", char *: "%s", const char *: "%s", default: "%p")
#define ESCREVA(x)          printf(BG_FMT(x), (x))
#define ESCREVAL(x)         (printf(BG_FMT(x), (x)), fputc('\n', stdout))
#define ESCREVA_LINHA()     fputc('\n', stdout)
#define ESCREVA_TEXTO(s)    printf("%s", (s))
#define ESCREVA_FMT(...)    printf(__VA_ARGS__)
#define ESCREVA_ERRO(s)     fprintf(stderr, "%s", (s))

#define BG_SFMT(x) _Generic(&(x), \
    int *: "%d", unsigned *: "%u", long *: "%ld", unsigned long *: "%lu", \
    long long *: "%lld", unsigned long long *: "%llu", \
    float *: "%f", double *: "%lf", char *: "%c", signed char *: "%hhd", \
    default: "%p")
#define LEIA(x)             ((void)(scanf(BG_SFMT(x), &(x)) == 1))
#define LEIA_OK(x)          (scanf(BG_SFMT(x), &(x)) == 1)
#define LEIA_TEXTO(b, t)    bg_leia_texto((b), (size_t)(t))

/* ---- erros ---- */
#define ERRO(msg)           bg_erro(__FILE__, __LINE__, (msg))
#define AFIRME(c, msg)      ((c) ? (void)0 : bg_erro(__FILE__, __LINE__, (msg)))
```

Observações normativas sobre este bloco:

* `SENAO` expande para `} else {` e **portanto** encadeia mal; por isso existe
  `SENAO_SE` (§C-05).
* `SE` expande para `if (` e **portanto** não aceita `{` em seguida; por isso `ENTAO`
  existe (§C-06).
* `INICIO`/`FIM` são macros objeto que produzem a definição de `main`; não use `main` em
  arquivo com `INICIO`.
* `FUNCAO` é uma macro **vazia** de propósito: `FUNCAO` + tipo é mais legível que só o
  tipo, e a expansão é literalmente o tipo.
* `ATE_QUE(c)` consome o `;` do programador: `ATE_QUE(c);` → `} while (!(c));`.
* Nomes prefixados (`BG_SE`, `BG_ENTAO`, …) têm **a mesma** expansão, e são obrigatórios
  quando `BG_SEM_APELIDOS` está definido antes do `#include`.

---

## 4. Saída real do pré-processador

### 4.1 Condicional (`gcc -E -P`)

Fonte:

```c
INICIO
    INTEIRO nota = 8;
    SE nota >= 7 ENTAO
        ESCREVAL("Aprovado");
    SENAO
        ESCREVAL("Reprovado");
    FIM_SE
FIM
```

Saída (tabela `_Generic` abreviada por `…`):

```c
int main(void) {
    int nota = 8;
    if ( nota >= 7 ) {
        (printf(_Generic(("Aprovado"), …), ("Aprovado")), fputc('\n', stdout));
    } else {
        (printf(_Generic(("Reprovado"), …), ("Reprovado")), fputc('\n', stdout));
    }
}
```

### 4.2 Registro, vetor, laço contado e cadeia `SE`/`SENAO_SE`

Fonte:

```c
REGISTRO(Aluno)
    INTEIRO matricula;
    REAL nota;
FIM_REGISTRO(Aluno);

INICIO
    VETOR(INTEIRO, v, 3);
    Aluno a;
    INTEIRO i;
    PARA (i, 0, 2) {
        v[i] = i * 2;
    }
    SE a.nota >= 7 E a.matricula > 0 ENTAO
        ESCREVAL(a.matricula);
    SENAO_SE a.nota >= 5 ENTAO
        ESCREVAL(0);
    SENAO
        ESCREVAL(-1);
    FIM_SE
    RETORNE 0;
FIM
```

Saída:

```c
typedef struct Aluno {
    int matricula;
    double nota;
} Aluno;
int main(void) {
    int v[3];
    Aluno a;
    int i;
    for ((i) = (0); (i) <= (2); ++(i)) {
        v[i] = i * 2;
    }
    if ( a.nota >= 7 && a.matricula > 0 ) {
    } else if ( a.nota >= 5 ) {
    } else {
    }
    return 0;
}
```

Esse é o argumento central da linguagem: **o aluno escreveu BASIC/Portugol e o
compilador viu C.**

---

## 5. Tabela reversa: de BG para C (material de aula)

| Conceito | BASICGOL | C |
|---|---|---|
| bloco | `SE … ENTAO … FIM_SE` | `if (…) { … }` |
| senão | `SENAO` | `else` |
| senão se | `SENAO_SE` | `else if` |
| enquanto | `ENQUANTO c FACA … FIM_ENQUANTO` | `while (c) { … }` |
| repita | `REPITA … ATE_QUE(c);` | `do { … } while (!(c));` |
| para | `PARA (i, a, b) { … }` | `for (i = a; i <= b; ++i) { … }` |
| escolha | `ESCOLHA(e) … FIM_ESCOLHA` | `switch (e) { … }` |
| caso | `CASO v:` | `case v:` |
| padrão | `CONTRARIO:` | `default:` |
| pare | `PARE;` | `break;` |
| continue | `PROSSIGA;` | `continue;` |
| retorne | `RETORNE x;` | `return x;` |
| escreva | `ESCREVA(x);` | `printf(…)` |
| leia | `LEIA(x);` | `scanf(…, &x)` |
| texto | `TEXTO s = "oi";` | `const char *s = "oi";` |
| vetor | `VETOR(INTEIRO, v, 10);` | `int v[10];` |
| registro | `REGISTRO(Aluno) … FIM_REGISTRO(Aluno);` | `typedef struct Aluno { … } Aluno;` |
| e/ou/não | `E` `OU` `NAO` | `&&` `\|\|` `!` |
| módulo | `MOD` | `%` |

---

## 6. O que **não** é mapeável (e o substituto)

| Sintaxe desejada | Por que não dá | Substituto |
|---|---|---|
| `VETOR INTEIRO v DE 10;` | exigiria reordenar tokens (`int v[10]`) | `VETOR(INTEIRO, v, 10);` (N) |
| `PARA i DE 1 ATE 10 FACA` | exigiria montar três partes de `for` fora de ordem | `PARA (i, 1, 10) { }` (N) ou M1 (X) |
| `ESCREVA "oi"` (sem parênteses) | o pré-processador não vê fim de linha nem precedência | `ESCREVA("oi");` — **nunca** haverá |
| `ESCREVAL(a, b, c)` | sem contagem/classificação de variádicos | uma chamada por valor, ou `ESCREVA_FMT` |
| `SE x { … }` | `SE` já emite `(`; colidiria com a forma canônica | `SE x ENTAO … FIM_SE`, ou `if (x) { … }` |
| `SENAO SE cond ENTAO` | `SENAO` emite `} else {`, gerando `{` extra | `SENAO_SE cond ENTAO` |
| `FUNCAO soma(a, b) INTEIRO` | exigiria mover o tipo de volta | `FUNCAO INTEIRO soma(…)` |
| fim de bloco automático pela indentação | macros não veem indentação | `FIM_SE`, `FIM_ENQUANTO`, … |
| `;` opcional | macro não pode inserir `;` com segurança depois de comandos arbitrários | `;` obrigatório (P-07) |
| texto com `+` (`"a" + "b"`) | em C é aritmética de ponteiros | `JUNTA_TEXTO(dest, tam, a, b)` |
| `TEXTO` modificável (`s[0] = 'x'`) | `TEXTO` é `const char *` | `TEXTO_FIXO(nome, tam)` |
| `ESCOLHA(f(a, b))` | vírgula dentro do argumento de macro | `INTEIRO t = f(a, b); ESCOLHA(t)` |
| identificadores acentuados (`média`) | C não garante; quebra portabilidade | `media` |
| `PARA i DE 1 ATE 10` com `i` novo (`INTEIRO i`) | exige declaração + laço (2 construtos) | declare antes |
| `TENTE/CAPTURE` | não há exceções em C | valor de retorno + `AFIRME`/`ERRO` |
| vetor de tamanho dinâmico `VETOR(t, v, n)` com `n` variável | seria VLA: vetores de tamanho variável são limitados e perigosos | `malloc` (C nativo) ou tamanho máximo fixo |
| `VETOR(t, v, k_minha_constante)` com `CONSTANTE` | `static const int` **não** é expressão constante em C → VLA silencioso | use literal (`VETOR(t, v, 30)`) ou `#define` (C-23) |

---

## 7. Biblioteca de execução (API especificada)

A Fase 2 deve fornecer estas funções (todas `static inline` em `basicgol.h`, exceto
`bg_erro`, que precisa de um símbolo por programa — resolvido por `static inline` +
`_Noreturn`). Assinaturas **normativas**:

```c
/* erros */
_Noreturn void bg_erro(const char *arquivo, int linha, const char *mensagem);
/* imprime "arquivo:linha: erro: mensagem" em stderr e encerra com status 1 */

/* entrada de texto (ver C-20)                                              */
bool bg_leia_texto(char *buf, size_t tam);
/* 1. consome UM terminador de linha pendente, se e somente se houver         */
/*    (\n ou \r\n deixado por um scanf anterior); caso contrario, nada e    */
/*    descartado — nao engole a primeira linha do programa.                   */
/* 2. le uma linha inteira com fgets, removendo '\n' e '\r' finais.          */
/* 3. devolve false em EOF/erro/tam==0; nada de truncamento silencioso:       */
/*    linha maior que tam e lida parcialmente e o resto fica no buffer,       */
/*    comportamento documentado e testado (T-13).                             */

/* texto */
size_t bg_tamanho_texto(const char * s);
bool   bg_copia_texto(char *dest, size_t tam, const char *origem);
bool   bg_junta_texto(char *dest, size_t tam, const char *a, const char *b);
/* false = não coube; nesse caso dest[0] = '\0' (truncamento nunca silencioso) */

bool bg_texto_para_inteiro(const char *s, int *saida);
bool bg_texto_para_real(const char *s, double *saida);
/* true somente se a conversao consumiu o texto INTEIRO (espacos finais
   tolerados); sobra de caracteres -> false. Sem isso, "4x2" viraria 4. */

/* matemática com avaliação única */
int    bg_abs_i(int x);      long   bg_abs_l(long x);
float  bg_abs_f(float x);    double bg_abs_d(double x);
int    bg_min_i(int a, int b);      double bg_min_d(double a, double b);
int    bg_max_i(int a, int b);      double bg_max_d(double a, double b);
long long bg_round_l(double x);

/* sorteio */
int  bg_sorteia(int n);              /* 0 .. n-1; n<=0 devolve 0 */
void bg_inicia_sorteio(unsigned s);
```

Requisitos de qualidade dessa biblioteca (entram nos critérios de aceitação da Fase 2):

| ID | Requisito |
|---|---|
| RB-1 | Nenhuma função avalia um argumento macro mais de uma vez |
| RB-2 | Nenhuma função tem efeito colateral escondido (exceto `bg_leia_texto`, documentada) |
| RB-3 | Truncamento de texto é sempre reportado por valor de retorno |
| RB-4 | Nenhuma função aborta o programa, exceto `bg_erro` |
| RB-5 | Todas compilam sem avisos em `-Wall -Wextra -Wformat=2` (GCC e Clang) |
| RB-6 | Nenhuma depende de extensão de compilador |
| RB-7 | Conversão texto→número só aceita o texto inteiro (sem sobra) |
| RB-8 | `bg_leia_texto` só consome um terminador de linha se ele estiver realmente pendente (C-20) |

---

## 8. Regras normativas para implementadores

| ID | Regra |
|---|---|
| RG-1 | Toda macro com argumentos parentiza **cada** uso do argumento. |
| RG-2 | Nenhum argumento de macro aparece mais de uma vez na expansão (P-11); se precisar de duas avaliações, use função `inline`. |
| RG-3 | Macros de comando não têm valor utilizável (são instruções); não as use em expressão. |
| RG-4 | Nenhuma macro de bloco usa `do { … } while (0)`: os `FIM_*` já fecham o bloco com `}`, e um invólucro `do/while(0)` impediria `SENAO` de se ligar ao `if` gerado por `SE`. |
| RG-5 | As expansões devem ser exatamente as da §3; mudanças exigem alteração desta especificação e nova rodada de validação. |
| RG-6 | O cabeçalho deve ser **idempotente** (`#ifndef`), compilável isoladamente e não deve definir nada além do que está documentado. |
| RG-7 | O cabeçalho deve funcionar incluído depois de `<stdio.h>`, `<math.h>` e `<string.h>` (que ele mesmo inclui). |
| RG-8 | Diante de `BG_SEM_APELIDOS`, nenhum nome curto pode ser definido. |
| RG-9 | O mapeamento não pode depender de `sizeof`, de avaliação em tempo de execução nem de qualquer informação que só o transpilador teria. |
| RG-10 | `basicgolc` (M1) deve emitir C **sem macros** (palavras-chave já traduzidas, chaves explícitas), para que as mensagens do compilador apontem para o código do aluno. |
