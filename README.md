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

Comando do caminho M0 (só precisa de um compilador C11):

```sh
cc -std=c11 -Wall -Wextra -x c programa.bg -o programa -lm -I include
```

Caminho M1 (transpilador):

```sh
make                                       # constrói build/basicgolc
build/basicgolc programa.bg -o programa.c  # translate para C
cc -std=c11 programa.c -o programa -lm -I include
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

## Licença

Apache License 2.0 — ver [`LICENSE`](LICENSE).
