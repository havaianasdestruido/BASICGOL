# BASICGOL

> BASICGOL — a legibilidade do BASIC e do Portugol, entregue como uma camada de macros C.

**Status: Fase 1 — especificação concluída. Nenhum código de implementação foi escrito ainda.**
Versão da especificação: **0.1** (pré-1.0, sujeita às decisões abertas em
[`docs/04-restricoes-e-decisoes.md`](docs/04-restricoes-e-decisoes.md)).

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

Comando previsto para M0 (o cabeçalho `basicgol.h` é entrega da Fase 2):

```sh
cc -std=c11 -Wall -Wextra -x c programa.bg -o programa -lm
```

## Documentos

| # | Documento | Conteúdo |
|---|---|---|
| 1 | [`docs/01-especificacao.md`](docs/01-especificacao.md) | Especificação da linguagem: objetivos, escopo, léxico, sintaxe, semântica, políticas |
| 2 | [`docs/02-referencia.md`](docs/02-referencia.md) | Referência de palavras-chave, operadores, tipos e biblioteca de execução |
| 3 | [`docs/03-mapeamento-para-c.md`](docs/03-mapeamento-para-c.md) | Tabela sintaxe → C, expansões reais verificadas e o que *não* é mapeável |
| 4 | [`docs/04-restricoes-e-decisoes.md`](docs/04-restricoes-e-decisoes.md) | Restrições de implementação (C-xx), decisões abertas (D-xx) e riscos |
| 5 | [`docs/05-criterios-de-aceitacao.md`](docs/05-criterios-de-aceitacao.md) | Critérios de aceitação da Fase 1 e portões para a Fase 2 |
| — | [`docs/exemplos/`](docs/exemplos/) | Programas de exemplo completos, no dialeto canônico |

## Escopo da Fase 1

Entregue: especificação, referência, mapeamento, restrições/decisões, critérios de
aceitação e exemplos.

Fora de escopo nesta fase: `basicgol.h`, `basicgolc`, suíte de testes, empacotamento,
realce de sintaxe para editores. Os exemplos foram validados contra uma **sonda
descartável** (não versionada) para garantir que cada construtor especificado realmente
expande para C compilável — ver §6 de [`docs/04-restricoes-e-decisoes.md`](docs/04-restricoes-e-decisoes.md).

## Licença

Apache License 2.0 — ver [`LICENSE`](LICENSE).
