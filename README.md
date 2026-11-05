# BASICGOL

> BASICGOL — a legibilidade do BASIC e do Portugol, entregue como uma camada de macros C.

**Status: Fase 1 concluída (especificação) · Fase 2 concluída (implementação + testes).**
O cabeçalho [`include/basicgol.h`](include/basicgol.h) e o transpilador
[`tools/basicgolc.c`](tools/basicgolc.c) existem, e a suíte em [`tests/`](tests/) roda
**203 verificações** (cada caso de programa é executado duas vezes: caminho M0 e caminho
M1). Versão da linguagem: **1.0.0**.

---

## O que é

BASICGOL (dialeto **BG**) é uma linguagem de programação *didática* cuja sintaxe usa
palavras-chave em português (`SE`, `ENTAO`, `ENQUANTO`, `ESCREVAL`, `REGISTRO`…) e cujo
modelo de execução é **exatamente o de C**: não há máquina virtual, coletor de lixo,
interpretador nem compilador próprio no caminho crítico.

Isso é possível porque a linguagem foi projetada **de dentro para fora**: em vez de
inventar uma sintaxe bonita e depois descobrir que o pré-processador C não consegue
produzi-la, cada construtor de BG foi escolhido por ser redutível, em tempo de
pré-processamento, a tokens de C válidos. O que não é redutível a macros está marcado
como **M1** (exige um passo de transpilação) ou simplesmente **não existe**.

```c
#include <basicgol.h>

FUNCAO REAL media(REAL a, REAL b) { RETORNE (a + b) / 2.0; }

INICIO
    REAL n1 = 0.0;
    REAL n2 = 0.0;

    ESCREVA("Nota 1: ");
    LEIA(n1);
    ESCREVA("Nota 2: ");
    LEIA(n2);

    SE media(n1, n2) >= 7.0 ENTAO
        ESCREVAL("Aprovado");
    SENAO_SE media(n1, n2) >= 5.0 ENTAO
        ESCREVAL("Recuperacao");
    SENAO
        ESCREVAL("Reprovado");
    FIM_SE
FIM
```

## Como isso será executado (dois caminhos, uma linguagem)

| Caminho | O que exige | Cobre |
|---|---|---|
| **M0 — macros** | só um compilador C11 | todo o subconjunto "núcleo" da linguagem |
| **M1 — transpilador** | um reescritor de tokens (`basicgolc`, Fase 2) | o núcleo + açúcar que exige reordenar tokens (`PARA i DE 1 ATE 10 FACA`) |

A fonte é a mesma nos dois caminhos; a semântica é a mesma; o resultado do
pré-processamento (M0) e o C emitido (M1) são equivalentes.

O fonte é um **`.c` comum** — a linguagem é uma camada de macros sobre C, então não há
extensão própria (D-14). Comando do caminho M0:

```sh
cc -std=c11 -Wall -Wextra programa.c -o programa -lm -I include
```

Caminho M1 (transpilador):

```sh
make                                            # constrói build/basicgolc
build/basicgolc programa.c -o programa.gerado.c # traduz para C "puro"
cc -std=c11 programa.gerado.c -o programa -lm -I include
```

## Repositório

| Caminho | O que é |
|---|---|
| [`include/basicgol.h`](include/basicgol.h) | a linguagem inteira: macros, tipos, biblioteca de execução (`bg_*`), header-only |
| [`tools/basicgolc.c`](tools/basicgolc.c) | transpilador M1: insere chaves, desaçucara `PARA i DE …`, aceita acentos, emite W-01…W-07 |
| [`tests/`](tests/) | suíte de testes: 85 casos em disco, 11 suítes, caminho M0 **e** M1 em cada caso |
| [`tests/COBERTURA.md`](tests/COBERTURA.md) | matriz de rastreabilidade gerada: cada nome, aviso, critério e restrição → o teste que o fixa |
| [`docs/`](docs/) | a especificação normativa (o código é a implementação dela) |
| [`Makefile`](Makefile) | `make`, `make test`, `make test-v`, `make exemplos`, `make cobertura` |

## Documentos

| # | Documento | Conteúdo |
|---|---|---|
| 1 | [`docs/01-especificacao.md`](docs/01-especificacao.md) | Especificação da linguagem: objetivos, escopo, léxico, sintaxe, semântica, políticas |
| 2 | [`docs/02-referencia.md`](docs/02-referencia.md) | Referência de palavras-chave, operadores, tipos e biblioteca de execução |
| 3 | [`docs/03-mapeamento-para-c.md`](docs/03-mapeamento-para-c.md) | Tabela sintaxe → C, expansões reais verificadas e o que *não* é mapeável |
| 4 | [`docs/04-restricoes-e-decisoes.md`](docs/04-restricoes-e-decisoes.md) | Restrições de implementação (C-xx), decisões abertas (D-xx) e riscos |
| 5 | [`docs/05-criterios-de-aceitacao.md`](docs/05-criterios-de-aceitacao.md) | Critérios de aceitação das duas fases, com o veredito de cada um |
| — | [`docs/exemplos/`](docs/exemplos/) | Programas de exemplo completos, no dialeto canônico |

## Fase 2 — o que foi entregue

* **`basicgol.h`**: implementa as 88 formas curtas da especificação, mais as 94 macros
  `BG_*` (sempre disponíveis) e a biblioteca `bg_*`. Sem dependências além de `libc` e
  `-lm`. Rejeita C99 com mensagem clara.
* **`basicgolc`**: transpilador M1 em C11 puro. Faz três coisas que macro não faz
  (`PARA i DE a ATE b [PASSO p] FACA` → `for`, `REPITA … ATE c;` → `do/while`, e chaves
  explícitas no C emitido), aceita palavras-chave acentuadas na entrada e diagnostica
  W-01…W-07 com linha e coluna.
* **Testes**: 85 casos em disco + checagens de sanidade, CLI e estresse. Cada caso de
  programa roda no caminho M0 e no M1 e precisa produzir exatamente a mesma saída — é a
  prova automatizada da equivalência M0 × M1 (AC-32). A cobertura por item está em
  [`tests/COBERTURA.md`](tests/COBERTURA.md).

### Três coisas que a implementação ensinou (e que a especificação corrigiu)

1. **C-04 estava errada**: `MACRO(f(a, b))` **não** quebra — os parênteses internos
   protegem a vírgula. Só a vírgula de nível superior é problema. Corrigido em
   [01 §11.6](docs/01-especificacao.md), [03 §6](docs/03-mapeamento-para-c.md) e
   [04 C-04](docs/04-restricoes-e-decisoes.md); o caso
   `03_controle/c22_escolha_com_chamada` fixa o comportamento certo nos dois perfis.
2. **`PARA_PASSO` com passo negativo** contava zero iterações no caminho M0 enquanto o M1
   contava certo — divergência de semântica encontrada pela suíte. O cabeçalho passou a
   usar o mesmo teste ternário do transpilador.
3. **`ATE_QUE(cond)` exige `;`** (é um `do/while`): sem o ponto e vírgula o erro do C
   aparece longe da causa. Hoje o transpilador acusa isso e mais dez situações
   (`'SE' exige 'ENTAO' antes de '{'`, `'SENAO SE' nao encadeia`,
   `'IGUAL' e prefixado e exige parenteses`…) antes de o compilador C entrar em cena.

## Escopo

Fora da v1.0, por decisão registrada: `BG_ESTRITO` (D-09), funções de vetor na biblioteca
(D-12), módulos/pacotes (D-18), C++ (D-15), realce de sintaxe para editores.

## Compilando — GCC e Clang

A fonte BASICGOL é um **`.c` comum** que inclui `<basicgol.h>`: sem extensão própria,
sem `-x c`, sem transpilador obrigatório. O caminho M0 precisa apenas de um compilador C11.

GCC:

```sh
gcc -std=c11 -Wall -Wextra -Wformat=2 -Wvla programa.c -o programa -lm -I include
```

Clang (mesmas flags; o cabeçalho usa apenas C11 padrão, sem extensões de compilador):

```sh
clang -std=c11 -Wall -Wextra -Wformat=2 -Wvla programa.c -o programa -lm -I include
```

`-lm` é necessário em plataformas que separam a libm (Linux) para `POTENCIA`, `RAIZ`,
`PISO`, `TETO` e `ARREDONDA`; em MinGW/MSVC a matemática já está no tempo de execução
e a flag pode ser omitida. O CI do repositório (`.github/workflows/testes.yml`) valida
GCC e Clang em `-std=c11` e `-std=c17`.

### Compatibilidade com MSVC (avaliação)

O cabeçalho é **C11 puro**: sem extensões GNU, sem atributos de compilador, sem recursos
de C++ (D-15, C-11). Avaliação estática contra `cl.exe` (não executado neste ambiente,
que não tem MSVC instalado):

| Item | Avaliação |
|---|---|
| Modo C | `/std:c11` ou `/std:c17` exigidos (MSVC ≥ 19.28 / Visual Studio 2019 16.8+ para `_Generic`, `_Static_assert`, `_Noreturn` no modo C). Modos mais antigos fazem o cabeçalho rejeitar a compilação com mensagem clara ("exige C11"). |
| Comando | `cl /std:c11 /EHsc programa.c /Iinclude` (sem `-lm`; `libm` já está incorporada ao tempo de execução do Windows). |
| `_Noreturn` | suportado em `/std:c11` (usado por `bg_erro`). |
| `static inline`, `_Generic`, `<stdint.h>` | suportados desde o modo C11 do MSVC. |
| `long double` / `%Lg` | no MSVC `long double` ≡ `double`; `%Lg` é aceito pelo CRT. |
| `llround`, `scanf`, `fgets`, `ungetc` | CRT padrão, sem problemas. |

Se um projeto tratar `.c` como C++, forçar `/TC` no arquivo-fonte (a extensão já é
`.c`, então não há renomeação — D-14). Para incluir `basicgol.h` em um projeto C
existente sem riscos de colisão de nomes, use `#define BG_SEM_APELIDOS` antes do
`#include`.

## Testes de fumaça (compilar e rodar)

```sh
make smoke        # constrói o transpilador e roda as 203 verificações (caminhos M0 e M1)
make smoke-v      # idem, mostrando cada caso aprovado
```

`make smoke` é sinônimo de `make test`: compila todos os casos de `tests/casos`, os 12
exemplos de `docs/exemplos` nos dois perfis, casos que **devem** falhar na compilação,
os avisos W-01…W-07 do transpilador, a tradução M1 e testes de estresse. Um programa
só passa se os dois caminhos (macros e transpilador) produzirem **exatamente a mesma
saída** — essa é a equivalência M0 × M1 exigida pelo critério AC-32.

## Programa mínimo

`docs/exemplos/ola_mundo.c` — o menor programa BASICGOL possível:

```c
#include <basicgol.h>

INICIO
    ESCREVAL("Ola, mundo!");
FIM
```

```sh
gcc -std=c11 -Wall -Wextra docs/exemplos/ola_mundo.c -o ola_mundo -lm -I include
./ola_mundo           # imprime: Ola, mundo!
```

## Exemplos entregues

`docs/exemplos/` tem 12 programas, cada um com entrada e saída esperadas gravadas
(`.entrada.txt` / `.esperado.txt`), consumidos pelos testes:

| Entrega | Exemplo que a cobre | Construtores principais |
|---|---|---|
| Declarações | `cadastro.c`, `vetores.c`, `media_notas.c` | `REGISTRO`/`FIM_REGISTRO`, vetor de registros, `VETOR`, `TEXTO_FIXO`, `TAMANHO_VETOR`, variáveis tipadas |
| Aritmética | `tabuada.c`, `primos.c`, `media_notas.c` | `PARA`, `PARA_PASSO`, laços aninhados, `MOD`, `ESCREVA_FMT`, `(a + b) / 2.0` |
| Entrada | `contador_vogais.c`, `cadastro.c`, `adivinha.c` | `LEIA_TEXTO` (contrato C-20), `LEIA`, `LEIA_OK`, `SORTEIA`, `INICIA_SORTEIO` |
| Saída | `ola_mundo.c` + todos os demais | `ESCREVA`, `ESCREVAL`, `ESCREVA_FMT`, `ESCREVA_ERRO` |
| Açúcar (M1) | `para_contado.c` | `PARA i DE … ATE … [PASSO …] FACA` / `FIM_PARA` |

## Limitações conhecidas (resumo)

A lista completa e sua justificativa está em [`docs/04-restricoes-e-decisoes.md`](docs/04-restricoes-e-decisoes.md) (restrições C-01…C-23 e decisões D-01…D-18); o sumário executivo está em [`docs/01-especificacao.md` §19](docs/01-especificacao.md). Os principais itens que afetam o uso diário:

1. **Sintaxe que reordena tokens só em M1**: `PARA i DE 1 ATE 10 FACA` exige `basicgolc`; no caminho M0 use `PARA (i, 1, 10) { }` (C-01).
2. **Um valor por chamada de saída**: `ESCREVAL(a, b)` não existe — use `ESCREVA_FMT` ou chamadas separadas (C-03, D-07).
3. **`MOD=` não existe**: `x MOD= 2` vira `x % = 2` e não compila; escreva `x = x MOD 2` ou `x %= 2` (C-01).
4. **`SE` exige `ENTAO`**: `SE x { }` e `SENAO SE` são proibidos; use `SENAO_SE` (C-05, C-06).
5. **Fechamento explícito**: `FIM_SE`, `FIM_ENQUANTO`, `FIM_ESCOLHA`, `FIM_PARA`, `FIM` são obrigatórios; a indentação não fecha blocos (C-14).
6. **Texto imutável**: `TEXTO` é `const char *` — não modifique caracteres; use `TEXTO_FIXO` para buffers. `TAMANHO_VETOR` não confere em ponteiros (C-17, C-18).
7. **Tamanho de vetor**: literal inteiro ou `#define`; `CONSTANTE` (`static const`) **não** é expressão constante em C e faria VLA (C-23).
8. **`LEIA_TEXTO` consome um `\n` pendente, se houver, e lê a linha seguinte** — nunca descarta conteúdo quando nada está pendente (contrato C-20).
9. **Avaliação de argumentos**: `ABS`, `MINIMO`, `MAXIMO`, `ESCREVA`, `ESCREVAL` e `LEIA` avaliam cada argumento exatamente uma vez (P-11, verificado com `ABS(++i)`). A exceção é o **passo de `PARA_PASSO`**, reavaliado a cada iteração — não use expressão com efeito colateral como passo.
10. **Sem rede de segurança em tempo de execução**: sem verificação de limites de vetor, sem detecção de tipo em tempo de execução, sem exceções; estouro de inteiro com sinal é comportamento indefinido de C (PT-E).
11. **C11 obrigatório**: C99 é rejeitado com mensagem clara (falta `_Generic`); C++ não é suportado (D-15).
12. **Diagnóstico no M0 aponta para o cabeçalho**: mensagens de macro citam `BG_*`; para diagnósticos que apontam para a linha do fonte do aluno, use o caminho M1 (`basicgolc`, erros com linha e coluna, §12.5 de `docs/01-especificacao.md`).

## Licença

Apache License 2.0 — ver [`LICENSE`](LICENSE).
