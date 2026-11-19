<h1 align="center"><img src="docs/logo.png" alt="BASICGOL" width="120" /><br />BASICGOL</h1>

<p align="center">
  A legibilidade do BASIC e do Portugol, com a portabilidade de C11.
</p>

<p align="center">
  <a href="https://havaianasdestruido.github.io/BASICGOL/">Documentação</a> ·
  <a href="playground/README.md">Playground</a> ·
  <a href="docs/exemplos/">Exemplos</a> ·
  <a href="CHANGELOG.md">Changelog</a> ·
  <a href="LICENSE">Licença Apache 2.0</a>
</p>

BASICGOL (dialeto **BG**) é uma linguagem didática com palavras-chave em português
(`SE`, `ENTAO`, `ENQUANTO`, `ESCREVAL`, `REGISTRO`…) e modelo de execução **idêntico ao
de C**: não há máquina virtual, coletor de lixo, interpretador nem compilador próprio no
caminho crítico. Um programa BASICGOL é um `.c` comum que inclui `<basicgol.h>`; os
construtores foram escolhidos por serem redutíveis a tokens de C válidos em tempo de
pré-processamento. O que não é redutível a macro está marcado como **M1** (exige o
transpilador `basicgolc`) ou simplesmente não existe.

```c
#include <basicgol.h>

FUNCAO REAL media(REAL a, REAL b) { RETORNE (a + b) / 2.0; }

INICIO
    REAL nota1 = 0.0;
    REAL nota2 = 0.0;

    ESCREVA("Nota 1: ");
    LEIA(nota1);
    ESCREVA("Nota 2: ");
    LEIA(nota2);

    SE media(nota1, nota2) >= 7.0 ENTAO
        ESCREVAL("Aprovado");
    SENAO_SE media(nota1, nota2) >= 5.0 ENTAO
        ESCREVAL("Recuperacao");
    SENAO
        ESCREVAL("Reprovado");
    FIM_SE
FIM
```

## Início rápido

Requisitos: compilador C11 (GCC, Clang ou MSVC), `make` e `ar`.

```sh
make biblioteca                       # build/libbasicgol.a
cc -std=c11 -Wall -Wextra -I include docs/exemplos/ola_mundo.c \
   build/libbasicgol.a -lm -o ola_mundo
./ola_mundo                           # imprime: Ola, mundo!
```

Quem usa a forma açucarada `PARA i DE 1 ATE 10 FACA` precisa do caminho **M1**:

```sh
make all                                       # transpilador + biblioteca
build/basicgolc programa.c -o programa.gerado.c
cc -std=c11 -I include programa.gerado.c build/libbasicgol.a -lm -o programa
```

## Dois caminhos, uma linguagem

| Caminho | O que exige | Cobre |
|---|---|---|
| **M0 — macros** | compilador C11 + `build/libbasicgol.a` | todo o subconjunto núcleo da linguagem |
| **M1 — transpilador** | C11, `basicgolc` e `build/libbasicgol.a` | o núcleo + açúcar que reordena tokens (`PARA i DE 1 ATE 10 FACA`) |

A fonte é a mesma e a semântica é a mesma; a suíte exige que M0 e M1 produzam
**exatamente a mesma saída** (critério AC-32).

## Estado e compatibilidade

Fases 1–6 concluídas. Versão da linguagem **1.1.0**, versão da biblioteca **1.0.0**.
Suíte verde em GCC 12.2.0 e no frontend Clang 20.1.2 (via `zig cc`), sob ASan + UBSan +
LeakSanitizer.

| Compilador | Estado |
|---|---|
| **GCC** | executado — suíte inteira, x86-64 Linux |
| **Clang (LLVM)** | frontend executado via `zig cc`; driver `clang` não testado |
| **MinGW-w64** | executado em CI nativo no Windows (UCRT64, MINGW64, MINGW32) |
| **MSVC** | avaliado; job de CI existe, sem registro verde |
| **AArch64** e outras ferramentas (Intel oneAPI, TCC, PCC, Digital Mars, Open64) | compilado (AArch64) ou não testado |

O vocabulário é o de [`docs/11-matriz-compatibilidade.md`](docs/11-matriz-compatibilidade.md)
§1: nada é chamado de "suportado" sem execução registrada. Instruções por ferramenta
(MSVC, MinGW-w64, `zig cc`, alvos cruzados, sanitizers) estão em
[`docs/10-compiladores.md`](docs/10-compiladores.md).

## Estrutura do repositório

| Caminho | O que é |
|---|---|
| [`include/basicgol.h`](include/basicgol.h) | palavras-chave, tipos, aliases e macros `BG_*` |
| [`include/basicgol/`](include/basicgol/) | headers modulares: console, matemática, texto, caractere, utilidades |
| [`src/biblioteca/`](src/biblioteca/) | implementações C11 que formam `build/libbasicgol.a` |
| [`tools/basicgolc.c`](tools/basicgolc.c) | transpilador M1 (`PARA … FACA`, `REPITA … ATE`, avisos W-01…W-07) |
| [`docs/`](docs/) | especificação, referência, compiladores, matriz de compatibilidade e 24 exemplos |
| [`tests/`](tests/) | suíte M0/M1, testes da biblioteca, fuzz e matriz de portabilidade |
| [`playground/`](playground/) | playground que compila e roda no navegador, sem servidor |
| [`examples/integrations/`](examples/integrations/README.md) | 35 exemplos de interoperabilidade (C++, C#, Assembly, VB.NET, Emscripten) |
| [`website/`](website/) | site Docusaurus com a referência gerada a partir do código |

## Playground no navegador

[`playground/`](playground/) compila e executa BASICGOL na própria aba: clang e wasm-ld
em WebAssembly, sem servidor e sem enviar código para lugar nenhum.

```sh
sh playground/tools/baixar-toolchain.sh        # baixa clang/lld (~18 MB)
node playground/tools/empacotar-toolchain.js   # monta o pacote da página
python3 -m http.server 8000                    # http://localhost:8000/playground/ativo/index.html
```

Detalhes e licenças de terceiros: [`playground/README.md`](playground/README.md) e
[`playground/ATRIBUICOES.md`](playground/ATRIBUICOES.md).

## Documentação

| Documento | Conteúdo |
|---|---|
| [Site](https://havaianasdestruido.github.io/BASICGOL/) | guias, referência gerada do código e relatórios |
| [`docs/01-especificacao.md`](docs/01-especificacao.md) | especificação da linguagem |
| [`docs/02-referencia.md`](docs/02-referencia.md) | palavras-chave, operadores, tipos e biblioteca |
| [`docs/06-tipos-e-expressoes.md`](docs/06-tipos-e-expressoes.md) | tipos, conversões, precedência e estouro |
| [`docs/07-controle-de-fluxo.md`](docs/07-controle-de-fluxo.md) | `SE`, `ENQUANTO`, `REPITA`, `PARA`, `ESCOLHA`, `PARE`, `PROSSIGA` |
| [`docs/09-biblioteca-padrao.md`](docs/09-biblioteca-padrao.md) | API modular da biblioteca padrão |
| [`docs/limitations.md`](docs/limitations.md) | limitações conhecidas (resumo) |
| [`BUILDING.md`](BUILDING.md) · [`docs/10-compiladores.md`](docs/10-compiladores.md) | instalação e compilação por ferramenta |

O site é gerado por [`tools/gerar_documentacao.py`](tools/gerar_documentacao.py) a partir
do código, dos testes e dos documentos, e publicado no GitHub Pages pelo workflow
[`docs.yml`](.github/workflows/docs.yml).

## Testes

```sh
make test               # 307 da linguagem + 230 unitárias + 30 de integração
make test-v             # idem, mostrando cada verificação aprovada
make test-sanitize      # suíte inteira sob ASan + UBSan + LeakSanitizer
make test-portabilidade # compiladores, avisos, multi-UO, C++, alvos cruzados
make fuzz               # mutação determinística do transpilador
make exemplos           # compila os exemplos de docs/exemplos
make ajuda              # lista todos os alvos
```

## Limitações principais

A lista completa está em [`docs/limitations.md`](docs/limitations.md),
[`docs/04-restricoes-e-decisoes.md`](docs/04-restricoes-e-decisoes.md) e
[`docs/07-controle-de-fluxo.md`](docs/07-controle-de-fluxo.md) §8. As que mais aparecem
no uso diário:

1. **Sintaxe que reordena tokens só em M1**: `PARA i DE 1 ATE 10 FACA` exige `basicgolc`; em M0 use `PARA (i, 1, 10) { }` (C-01).
2. **Um valor por chamada de saída**: `ESCREVAL(a, b)` não existe — use `ESCREVA_FMT` ou chamadas separadas (C-03).
3. **`MOD=` não existe**: escreva `x = x MOD 2` (C-01).
4. **`SE` exige `ENTAO`** e `SENAO SE` é proibido; use `SENAO_SE` (C-05, C-06).
5. **Fechamento explícito**: `FIM_SE`, `FIM_ENQUANTO`, `FIM_ESCOLHA`, `FIM_PARA` e `FIM` são obrigatórios (C-14).
6. **Texto imutável**: `TEXTO` é `const char *`; para buffers use `TEXTO_FIXO` (C-17).
7. **Tamanho de vetor** precisa ser literal inteiro ou `#define` (C-23).
8. **`LEIA` só lê escalares numéricos e `CARACTERE`**; para texto use `LEIA_TEXTO` (DF-01).
9. **Sem rede de segurança em tempo de execução**: sem checagem de limites e sem exceções; estouro de inteiro com sinal é comportamento indefinido de C.
10. **C11 obrigatório**: C99 é rejeitado com mensagem clara e C++ não é suportado (D-15).

## Licença

Apache License 2.0 — ver [`LICENSE`](LICENSE).
