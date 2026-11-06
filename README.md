# BASICGOL

> BASICGOL — a legibilidade do BASIC e do Portugol, entregue como uma camada de macros C.

**Status: Fases 1–6 concluídas (portabilidade e endurecimento).**
O cabeçalho [`include/basicgol.h`](include/basicgol.h), os módulos em
[`include/basicgol/`](include/basicgol/) e [`src/biblioteca/`](src/biblioteca/) e o
transpilador [`tools/basicgolc.c`](tools/basicgolc.c) são testados em [`tests/`](tests/):
**307 verificações da linguagem**, mais **230 verificações unitárias** e **30 verificações
de integração/falhas fatais** da biblioteca. Versão da linguagem: **1.1.0**; versão da
biblioteca: **1.0.0**.

A suíte roda igual em **dois frontends independentes** (GCC 12.2.0 e Clang 20.1.2), sob
**ASan + UBSan + LeakSanitizer**, e tem uma matriz de portabilidade com **70 verificações
aprovadas** — incluindo 32 bits, AArch64 e Windows por compilação cruzada. O que **não**
foi executado está declarado, item por item, em
[`docs/11-matriz-compatibilidade.md`](docs/11-matriz-compatibilidade.md) e
[`docs/12-relatorio-fase-6.md`](docs/12-relatorio-fase-6.md) §8. Estado de cada
ferramenta: **GCC executado**, **frontend Clang executado (via `zig cc`)**,
**MinGW-w64 e AArch64 compilados**, **MSVC apenas avaliado** — e nada além disso é
chamado de "suportado" em lugar nenhum.

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

## Por que criar uma línguagem nova?

Eu decidi desenvolver esse projeto pois estava frustrado com os defeitos de linguagens em Portugues, como o `Portugol`.

Na minha escola, por um longo período nós tivemos que estudar essa línguagem, e ela é repleta de furos e inconsistências, o que me motivou a desenvolver o `BASICGOL`.

A grande vantagem de usar o `BASICGOL` ao invés de `Portugol`, é o fato de que o `BASICGOL` é um header C11 simples, basta incluir ele no topo do seu arquivo que ele já está pronto para uso.

Meu foco principal é em compatibilidade multi-plataforma e multi-compilador, não quero que `BASICGOL` fique "preso" num estado aonde só compila no MSVC no Windows, quero que também funcione, por exemplo, no GCC sob Linux.

Atualmente, o sistema tem foco principal nos seguintes compiladores. A coluna "estado"
usa o vocabulário de [`docs/11`](docs/11-matriz-compatibilidade.md) §1, e não existe
"suportado" sem execução registrada:

| Compilador | Estado |
|---|---|
| **GCC** (versão livre e de código aberto, Linux/Windows/macOS) | **executado** — GCC 12.2.0, x86-64 Linux, suíte inteira |
| **Clang (LLVM)** (diagnósticos rápidos e detalhados) | **executado** como *frontend* (Clang 20.1.2 via `zig cc`); o driver `clang` do LLVM não foi testado |
| **Microsoft Visual C++ (MSVC)** | **avaliado** — nunca executado aqui; job de CI existe, mas não há registro verde |
| **MinGW-w64** (GCC para Windows, 32/64 bits) | **compilado** — `x86_64-windows-gnu` por compilação cruzada; execução não (sem `wine`) |
| **Intel oneAPI (ICX/ICPX)**, **TCC**, **PCC**, **Digital Mars**, **Open64** | **não testados** — indisponíveis neste ambiente |

Eu tenho planos bem ambiciosos com essa linguagem (_uns até de mais_), os que me vem na cabeça nesse momento são:

- [ ] Fazer um bootloader exibindo "Olá Mundo!" em `BASICGOL` **puro**
- [ ] Portar exemplo de Arduino pra funcionar 100% em `BASICGOL`
- [ ] Portar funções úteis do C++ pra `BASICGOL` (só as mais úteis mesmo)
- - [ ] Exemplos: `VETOR_DINAMICO` (alternativa simplificada a `std::vector`), `PILHA` (`std::stack`), `FILA` (`std::queue`), `PAR` (`std::pair`), `OPCIONAL` (`std::optional`), `MINIMO`/`MAXIMO` (`std::min`/`std::max`), `TROCA` (`std::swap`), `STRING` (manipulação de texto com tamanho dinâmico), `UNICO_PTR` (propriedade exclusiva de memória inspirada em `std::unique_ptr`) e `ESCOPO_RECURSO` (liberação automática de recursos inspirada em RAII). A implementação deverá usar estruturas, funções e macros C11, com gerenciamento de memória explícito quando necessário e documentação clara das diferenças em relação ao C++
- [ ] Fazer exemplos maiores, demonstrando a capacidade da linguagem, como sistemas inteiros, servidores HTTP, projetos IoT, etc.
- [ ] Reescrever apps feitos para DOS com `BASICGOL`
- [ ] Adicionar camadas de abstração para facilitar programação I/O entre sistemas
- - [ ] Mais espeficiadamente, por exemplo, criar uma camada de abstração de I/O multiplataforma para o `BASICGOL`, permitindo operações como `ABRE_ARQUIVO()`, `LE_ARQUIVO()`, `ESCREVE_ARQUIVO()`, `FECHA_ARQUIVO()`, `LE_PORTA_SERIAL()` e `ESCREVE_PORTA_SERIAL()` sem depender diretamente das APIs específicas de cada sistema operacional. Permitindo que o mesmo código BASICGOL leia dados de uma porta serial no Windows (Win32 API) e no Linux (termios/POSIX), mantendo uma interface comum e delegando os detalhes específicos da plataforma a implementações em C
- [ ] Fazer um app de exemplo que use Lua, demonstrando a capacidade de usar linguagens "embed"-dáveis, como se fosse um CLI pra enviar comandos Lua. (JIT)
- [ ] Usar o motor do Chromium pra fazer um mini-navegador, expondo uma nova função no motor JavaScript dele, uma que permite enviar MessageBoxes nativas de cada sistema diretamente da página Web (provavelmente nunca vou conseguir fazer isso ;--;)

## Como isso será executado (dois caminhos, uma linguagem)

| Caminho | O que exige | Cobre |
|---|---|---|
| **M0 — macros** | compilador C11 + `build/libbasicgol.a` | todo o subconjunto "núcleo" da linguagem |
| **M1 — transpilador** | C11, `basicgolc` e `build/libbasicgol.a` | o núcleo + açúcar que exige reordenar tokens (`PARA i DE 1 ATE 10 FACA`) |

A fonte é a mesma nos dois caminhos; a semântica é a mesma; o resultado do
pré-processamento (M0) e o C emitido (M1) são equivalentes.

O fonte é um **`.c` comum** — a linguagem é uma camada sobre C, sem extensão própria
(D-14). Os helpers da biblioteca são módulos compilados. Comando M0:

```sh
make biblioteca
cc -std=c11 -Wall -Wextra -I include programa.c \
   build/libbasicgol.a -lm -o programa
```

Caminho M1 (transpilador):

```sh
make                                            # transpilador + build/libbasicgol.a
build/basicgolc programa.c -o programa.gerado.c # traduz para C "puro"
cc -std=c11 -I include programa.gerado.c \
   build/libbasicgol.a -lm -o programa
```

## Repositório

| Caminho | O que é |
|---|---|
| [`include/basicgol.h`](include/basicgol.h) | palavras-chave, tipos, aliases e compatibilidade com a API `BG_*` |
| [`include/basicgol/`](include/basicgol/) | headers modulares de console, matemática, texto, caractere e utilidades |
| [`src/biblioteca/`](src/biblioteca/) | implementações C11 que formam `build/libbasicgol.a` |
| [`tools/basicgolc.c`](tools/basicgolc.c) | transpilador M1: insere chaves, desaçucara `PARA i DE …`, aceita acentos, emite W-01…W-07 |
| [`tests/`](tests/) | suíte M0/M1 com 135 casos em disco, 230 verificações unitárias da biblioteca e a matriz de portabilidade (`tests/portabilidade/`) |
| [`tests/run_portabilidade.sh`](tests/run_portabilidade.sh) | matriz de compiladores, padrões, avisos, multi-UO, C++, sanitizers e alvos cruzados |
| [`tests/run_matriz.sh`](tests/run_matriz.sh) · [`tests/run_checkout_limpo.sh`](tests/run_checkout_limpo.sh) · [`tests/fuzz_basicgolc.py`](tests/fuzz_basicgolc.py) | suíte por compilador, verificação dos comandos do README, mutação do transpilador |
| [`.github/workflows/testes.yml`](.github/workflows/testes.yml) | CI: matriz de compiladores × padrões, sanitizers, portabilidade, cobertura e avaliação MSVC |
| [`tests/COBERTURA.md`](tests/COBERTURA.md) | matriz de rastreabilidade gerada: nomes, módulos, avisos, critérios e restrições |
| [`docs/`](docs/) | especificação, referência e exemplos executáveis |
| [`Makefile`](Makefile) | `make`, `make test`, `make test-portabilidade`, `make test-sanitize`, `make test-matriz`, `make fuzz`, `make test-checkout`, `make exemplos`, `make cobertura` (`make ajuda` lista tudo) |

## Documentos

| # | Documento | Conteúdo |
|---|---|---|
| 1 | [`docs/01-especificacao.md`](docs/01-especificacao.md) | Especificação da linguagem: objetivos, escopo, léxico, sintaxe, semântica, políticas |
| 2 | [`docs/02-referencia.md`](docs/02-referencia.md) | Referência de palavras-chave, operadores, tipos e biblioteca de execução |
| 3 | [`docs/03-mapeamento-para-c.md`](docs/03-mapeamento-para-c.md) | Tabela sintaxe → C, expansões reais verificadas e o que *não* é mapeável |
| 4 | [`docs/04-restricoes-e-decisoes.md`](docs/04-restricoes-e-decisoes.md) | Restrições de implementação (C-xx), decisões abertas (D-xx) e riscos |
| 5 | [`docs/05-criterios-de-aceitacao.md`](docs/05-criterios-de-aceitacao.md) | Critérios de aceitação das fases, com o veredito de cada um |
| 6 | [`docs/06-tipos-e-expressoes.md`](docs/06-tipos-e-expressoes.md) | **Referência de tipos e expressões**: representação, mapeamento C, conversões, precedência, verdade, estouro, ponto flutuante, extensões opcionais e diferenças em relação a BASIC/Portugol |
| 7 | [`docs/07-controle-de-fluxo.md`](docs/07-controle-de-fluxo.md) | **Controle de fluxo**: sintaxe exata de `SE`, `ENQUANTO`, `REPITA`, `PARA`, `ESCOLHA`, `PARE`, `PROSSIGA`; expansões; casos de borda; ninhos e `SENAO`; limitações |
| 8 | [`docs/08-relatorio-controle-de-fluxo.md`](docs/08-relatorio-controle-de-fluxo.md) | Relatório de aprovação e reprovação da Fase 3 |
| 9 | [`docs/09-biblioteca-padrao.md`](docs/09-biblioteca-padrao.md) | API modular da biblioteca padrão, convenções, testes, build e dependências |
| 10 | [`docs/10-compiladores.md`](docs/10-compiladores.md) | **Como compilar em cada ferramenta**: GCC, Clang, MSVC, MinGW-w64, `zig cc`, sanitizers, C++ e o conjunto de avisos adotado |
| 11 | [`docs/11-matriz-compatibilidade.md`](docs/11-matriz-compatibilidade.md) | **Matriz de compatibilidade**: compilador × alvo × padrão × estado, com o vocabulário de 4 estados (executado / compilado / avaliado / não testado) |
| 12 | [`docs/12-relatorio-fase-6.md`](docs/12-relatorio-fase-6.md) | Relatório da Fase 6: o que foi executado, números, defeitos e lacunas declaradas |
| 13 | [`docs/13-defeitos-e-correcoes.md`](docs/13-defeitos-e-correcoes.md) | Os 8 defeitos encontrados (DF-01…DF-08), com reprodução, causa e correção; e o que **não** é defeito |
| — | [`docs/exemplos/`](docs/exemplos/) | Programas da linguagem e exemplos por módulo da biblioteca |

## Fase 2 — o que foi entregue

* **`basicgol.h`**: implementa as 117 formas curtas e as macros `BG_*`; reexporta os
  módulos da biblioteca. Rejeita C99 com mensagem clara. A Fase 5 moveu os helpers para
  headers e implementações próprias, preservando os aliases didáticos.
* **`basicgolc`**: transpilador M1 em C11 puro. Faz três coisas que macro não faz
  (`PARA i DE a ATE b [PASSO p] FACA` → `for`, `REPITA … ATE c;` → `do/while`, e chaves
  explícitas no C emitido), aceita palavras-chave acentuadas na entrada e diagnostica
  W-01…W-07 com linha e coluna.
* **Testes**: 132 casos em disco, sanidade, CLI, estresse, 230 verificações unitárias
  da biblioteca e verificações de integração. Cada caso roda no caminho M0 e no M1 e
  precisa produzir a mesma saída — prova automatizada da equivalência M0 × M1 (AC-32).
  A matriz está em [`tests/COBERTURA.md`](tests/COBERTURA.md).

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

## Fase 3 — controle de fluxo

Os construtores de controle já estavam no núcleo desde a Fase 2; a Fase 3 os **fixa, testa nos
casos de borda e documenta os limites** ([07](docs/07-controle-de-fluxo.md)):

* **Sintaxe**: `SE`/`SENAO_SE`/`SENAO`/`FIM_SE`, `ENQUANTO`/`FACA`/`FIM_ENQUANTO`,
  `REPITA`/`ATE_QUE(c);`, `PARA`/`PARA_PASSO` (N) e `PARA i DE … ATE … FACA` (açúcar, M1),
  `ESCOLHA`/`CASO`/`CONTRARIO`/`FIM_ESCOLHA`, `PARE`, `PROSSIGA`. Nenhuma palavra nova.
* **Passo zero não trava mais**: `PARA_PASSO` com passo 0 vira erro de execução
  (`PARA_PASSO: passo zero`, código 1) no M0 e no M1; passo literal `0` é recusado já na
  tradução. Para passo não nulo, nada muda.
* **`ATE_QUE(cond)` sem `;`** é recusado pelo transpilador com a linha certa (antes passava
  adiante e o erro aparecia no C).
* **`SENAO SE`** continua proibido (CND-2), e a mensagem agora diz como escrever um `SE`
  aninhado dentro de `SENAO`.
* **Limites documentados**: sem `PARE` rotulado nem `goto`; `PARE` dentro de `ESCOLHA` sai só
  do `ESCOLHA`; `SENAO SE` é aceito pelo M0 e recusado pelo M1; `PARA` (forma N) usa chaves
  de C; limites como `INT_MAX` nunca são alcançados.
* **Testes e exemplos**: casos `03_controle/c24…c40` (zero iterações, limites, passo zero,
  ninhos com `PARE`/`PROSSIGA`, saída de dois laços, `REPITA`/`ATE_QUE` com `PROSSIGA`, `ESCOLHA`
  dentro de laço, entrada com sentinela); negativos `06_negativos/n17…n20` e
  `12_negativos_m1/t12…t14`; exemplos `controle_condicoes`, `controle_lacos`,
  `controle_acumuladores`, `controle_entrada` e `controle_para_acucar` (M1).

## Fase 4 — tipos e semântica de expressões

* **Tipos com contrato explícito**: inteiros com e sem sinal (nativos e de largura
  exata), `REAL_CURTO`/`REAL`/`REAL_LONGO`, `CARACTERE`, `LOGICO`, `TEXTO`, `TEXTO_FIXO`,
  `VAZIO` — cada um com o tipo C, a garantia e o limite documentados em
  [06](docs/06-tipos-e-expressoes.md). `NATURAL_CURTO`/`NATURAL_LONGO` completam a
  família; `TAMANHO_DE`, `BITS_DE`, `MAXIMO_INTEIRO`, `MAXIMO_NATURAL`, `MAXIMO_REAL` e
  `EPSILON_REAL` consultam o ambiente de verdade.
* **Operadores em português**: `E`, `OU`, `NAO`, `MOD`, mais `OU_EXCLUSIVO(a,b)` e
  `LOGICO_DE(x)`. Precedência é a de C, documentada com exemplos; `^` continua sendo XOR
  (potência é `POTENCIA`).
* **Erro em vez de comportamento indefinido**: `DIVIDE(a,b)` e `RESTO(a,b)` recusam
  divisor zero e quocientes assinados não representáveis (em geral `MIN/-1`) com mensagem
  apontando a linha do aluno. `/` e `MOD` continuam sendo o C cru.
* **Conversões verificadas**: `TEXTO_PARA_INTEIRO`, `TEXTO_PARA_NATURAL`,
  `TEXTO_PARA_REAL` e `REAL_PARA_INTEIRO` agora recusam sobra, estouro de faixa e NaN
  (antes, `"2147483648"` virava `VERDADEIRO` com valor truncado).
* **Texto seguro**: `ACRESCENTA_TEXTO`, `TRECHO_TEXTO`, `COMPARA_TEXTO`, `TEXTO_VAZIO`,
  `FORMATA_TEXTO` e helpers ASCII de caractere, todos com tamanho explícito e truncamento
  reportado por valor de retorno.
* **Entrada numérica sem lixo**: `LEIA_INTEIRO(x)` e `LEIA_REAL(x)` leem uma linha,
  aceitam exatamente um número e nunca deixam o `stdin` dessincronizado.
* **Extensões isoladas**: `BG_USAR_EXTENSOES` liga aritmética verificada via
  `__builtin_*_overflow` (GCC/Clang). Desligada — o padrão — o cabeçalho é C11 estrito.
* **Testes**: 15 casos novos (M0 + M1), 5 negativos de tipo, 2 checagens de sanidade e 2
  exemplos executáveis; a matriz item-por-item continua em
  [`tests/COBERTURA.md`](tests/COBERTURA.md).

## Fase 5 — biblioteca padrão

* A API está separada em cinco módulos C11: console, matemática, texto, caractere e
  utilidades. Os headers ficam em `include/basicgol/`, as implementações em
  `src/biblioteca/`, e `make biblioteca` constrói `build/libbasicgol.a`.
* Os headers são individuais ou agregados por `<basicgol/biblioteca.h>`; `<basicgol.h>`
  reexporta os módulos e conserva os macros BASICGOL existentes. As funções recuperáveis
  devolvem `bool`; strings usam capacidade explícita, saídas ficam vazias/inalteradas em
  falha e a memória continua pertencendo ao chamador.
* Operações matemáticas existentes reutilizam `<math.h>`. `bg_sorteia_ate` requer limite
  e semente explícitos; a documentação limita as garantias de `rand()` (previsível,
  global, não criptográfico e distribuição dependente da implementação).
* O core não adiciona dependências além de C11/libc e libm quando separada; não inclui
  APIs de plataforma. Helpers específicos de plataforma ficam fora do agregado portátil.
* `docs/09-biblioteca-padrao.md` documenta parâmetros, retornos, convenções, build e
  dependências. Cinco programas em `docs/exemplos/biblioteca_*.c` demonstram os módulos;
  `make test-biblioteca` roda testes de sucesso, erro e fronteira para a API.

## Fase 6 — portabilidade e endurecimento

A Fase 6 não acrescenta construtor nenhum: ela verifica se o que está escrito continua
verdadeiro **fora** do compilador em que foi escrito. Relatório completo em
[`docs/12`](docs/12-relatorio-fase-6.md); matriz em [`docs/11`](docs/11-matriz-compatibilidade.md).

* **Dois frontends, o mesmo número**: GCC 12.2.0 e Clang 20.1.2 rodam a suíte inteira
  (307 + 230 + 30) e nenhum dos dois emite aviso no conjunto adotado (`-Werror`).
* **Sanitizers**: `make test-sanitize` fecha limpo com ASan + UBSan +
  `-fno-sanitize-recover=all` e LeakSanitizer. Foi o que encontrou dois dos defeitos da
  fase.
* **Programas de portabilidade** (`tests/portabilidade/`): precedência, controle de fluxo,
  entrada/saída com saída dourada, fronteira de tipos (176 verificações), link de 4
  unidades de tradução, C++ e alvos cruzados — 70 verificações aprovadas, 2 ignoradas
  (padrão `c23` no driver do zig; execução de binários Windows sem `wine`).
* **Auditoria de símbolos**: `nm` prova que incluir `basicgol.h` em vários `.c` não
  exporta nenhum global extra, então não há colisão no linker.
* **C++**: os headers de módulo em `include/basicgol/` compilam, ligam e rodam em C++17
  (`extern "C"` + `BG_NORETURN`); `<basicgol.h>` continua **só C**, de propósito (D-15).
* **Oito defeitos encontrados e corrigidos** em [`docs/13`](docs/13-defeitos-e-correcoes.md),
  incluindo um de gravidade alta: `LEIA(LOGICO)` gravava um ponteiro de 8 bytes num
  objeto de 1 byte (stack-buffer-overflow), porque o `_Generic` do `LEIA` tinha
  `default: "%p"`. Hoje é **erro de compilação**, com ou sem `-Werror`.
* **Nada de "suportado" sem execução**: MSVC está **avaliado**; AArch64 e Windows estão
  **compilados, não executados**. A tabela de estados é a fonte da verdade.

## Escopo

Fora da v1.0 como recursos da linguagem, por decisão registrada: `BG_ESTRITO` (D-09),
funções de vetor (D-12), módulos/pacotes de linguagem (D-18), C++ (D-15) e realce de
sintaxe para editores. A Fase 5 organiza a biblioteca em módulos C, mas não introduz
`IMPORTE` ou pacotes BASICGOL. Tipos decimais, texto Unicode de tamanho dinâmico e
detecção automática de estouro em `+ - *` continuam fora (D-22, C-24, C-25).

## Compilando — GCC e Clang

A fonte BASICGOL é um **`.c` comum** que inclui `<basicgol.h>`; não há extensão própria
nem `-x c`. Os programas que usam helpers ligam a biblioteca modular compilada.

GCC:

```sh
make biblioteca
gcc -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include programa.c \
    build/libbasicgol.a -lm -o programa
```

Clang (mesmas flags; o core e a biblioteca usam C11 padrão):

```sh
make biblioteca
clang -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include programa.c \
    build/libbasicgol.a -lm -o programa
```

`-lm` é necessário em plataformas que separam a libm (por exemplo, Linux); em
plataformas que incorporam a biblioteca matemática, omita a flag. O CI
(`.github/workflows/testes.yml`) valida GCC e Clang em `-std=c11` e `-std=c17` para a
linguagem; os módulos são compilados como C11.

> **Sobre o Clang neste projeto.** O que foi executado foi o *frontend* Clang (20.1.2),
> através do `zig cc` 0.15.1, porque não havia binário do LLVM disponível. Os comandos
> acima com `clang` são os mesmos e a expectativa é idêntica, mas **não há registro de
> execução com o driver `clang`** — está declarado assim em
> [`docs/11 §1`](docs/11-matriz-compatibilidade.md). Instruções por ferramenta (MSVC,
> MinGW-w64, `zig cc`, alvos cruzados, sanitizers e o conjunto de avisos) estão em
> [`docs/10-compiladores.md`](docs/10-compiladores.md).

### Compatibilidade com MSVC (avaliação)

O código usa C11 padrão, sem extensões GNU ou recursos de C++ (D-15, C-11). A
compatibilidade com `cl.exe` não foi executada neste ambiente; para compilar sem o
archive Unix, passe os fontes dos módulos ao compilador:

| Item | Avaliação |
|---|---|
| Modo C | `/std:c11` ou `/std:c17` (MSVC ≥ 19.28 / Visual Studio 2019 16.8+ para `_Generic`, `_Static_assert` e `_Noreturn`). |
| Comando indicativo | `cl /std:c11 /Iinclude programa.c src\biblioteca\consola.c src\biblioteca\matematica.c src\biblioteca\texto.c src\biblioteca\caractere.c src\biblioteca\utilidades.c /Fe:programa.exe` (não verificado aqui). |
| Build archive | O Makefile usa `ar`; para MSVC, ligue os fontes diretamente ou crie um `.lib` com as ferramentas da plataforma. |
| Matemática | Windows CRT geralmente não requer `-lm`; verifique a toolchain escolhida. |
| `long double` / `%Lg` | no MSVC `long double` equivale a `double`; `%Lg` é aceito pelo CRT. |

Se um projeto tratar `.c` como C++, forçar `/TC` no arquivo-fonte (a extensão já é
`.c`, então não há renomeação — D-14). Para incluir `basicgol.h` em um projeto C
existente sem riscos de colisão de nomes, use `#define BG_SEM_APELIDOS` antes do
`#include`.

## Testes de fumaça (compilar e rodar)

```sh
make test               # 307 da linguagem + 230 unitárias + 30 de integração
make test-v             # idem, mostrando cada verificação aprovada
make test-sanitize      # a suíte inteira sob ASan + UBSan + LeakSanitizer
make test-portabilidade # matriz de compiladores, avisos, multi-UO, C++, alvos cruzados
make test-matriz        # a suíte inteira com cada compilador encontrado
make fuzz               # mutação determinística do transpilador
make test-checkout      # os comandos deste README, a partir de um checkout limpo
make smoke              # sinônimo de make test (nome antigo, mantido)
```

`make smoke` é sinônimo de `make test`: compila todos os casos de `tests/casos`, os 24
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
make biblioteca
gcc -std=c11 -Wall -Wextra -I include docs/exemplos/ola_mundo.c \
    build/libbasicgol.a -lm -o ola_mundo
./ola_mundo           # imprime: Ola, mundo!
```

## Exemplos entregues

`docs/exemplos/` tem 24 programas, cada um com entrada e saída esperadas gravadas
(`.entrada.txt` / `.esperado.txt`), consumidos pelos testes. Os cinco exemplos
`biblioteca_*.c` demonstram diretamente os módulos da Fase 5:

| Entrega | Exemplo que a cobre | Construtores principais |
|---|---|---|
| Declarações | `cadastro.c`, `vetores.c`, `media_notas.c` | `REGISTRO`/`FIM_REGISTRO`, vetor de registros, `VETOR`, `TEXTO_FIXO`, `TAMANHO_VETOR`, variáveis tipadas |
| Aritmética | `tabuada.c`, `primos.c`, `media_notas.c` | `PARA`, `PARA_PASSO`, laços aninhados, `MOD`, `ESCREVA_FMT`, `(a + b) / 2.0` |
| Entrada | `contador_vogais.c`, `cadastro.c`, `adivinha.c` | `LEIA_TEXTO` (contrato C-20), `LEIA`, `LEIA_OK`, `SORTEIA`, `INICIA_SORTEIO` |
| Saída | `ola_mundo.c` + todos os demais | `ESCREVA`, `ESCREVAL`, `ESCREVA_FMT`, `ESCREVA_ERRO` |
| Controle de fluxo | `controle_condicoes.c`, `controle_lacos.c`, `controle_acumuladores.c`, `controle_entrada.c` | `SE` aninhado, `SENAO_SE`, `PARA`/`PARA_PASSO`, `ENQUANTO`, `REPITA`, `PARE`, `PROSSIGA`, laços por entrada com sentinela |
| Açúcar (M1) | `para_contado.c`, `controle_para_acucar.c` | `PARA i DE … ATE … [PASSO …] FACA` / `FIM_PARA`, `ATE cond;` |
| Biblioteca padrão | `biblioteca_consola.c`, `biblioteca_matematica.c`, `biblioteca_texto.c`, `biblioteca_caractere.c`, `biblioteca_utilidades.c` | I/O limitado, matemática, texto, ASCII e sorteio com semente explícita |

## Limitações conhecidas (resumo)

A lista completa e sua justificativa está em [`docs/04-restricoes-e-decisoes.md`](docs/04-restricoes-e-decisoes.md) (restrições C-01…C-23 e decisões D-01…D-28); o sumário executivo está em [`docs/01-especificacao.md` §19](docs/01-especificacao.md). Os principais itens que afetam o uso diário:

1. **Sintaxe que reordena tokens só em M1**: `PARA i DE 1 ATE 10 FACA` exige `basicgolc`; no caminho M0 use `PARA (i, 1, 10) { }` (C-01).
2. **Um valor por chamada de saída**: `ESCREVAL(a, b)` não existe — use `ESCREVA_FMT` ou chamadas separadas (C-03, D-07).
3. **`MOD=` não existe**: `x MOD= 2` vira `x % = 2` e não compila; escreva `x = x MOD 2` ou `x %= 2` (C-01).
4. **`SE` exige `ENTAO`**: `SE x { }` e `SENAO SE` são proibidos; use `SENAO_SE` (C-05, C-06).
5. **Fechamento explícito**: `FIM_SE`, `FIM_ENQUANTO`, `FIM_ESCOLHA`, `FIM_PARA`, `FIM` são obrigatórios; a indentação não fecha blocos (C-14).
6. **Texto imutável**: `TEXTO` é `const char *` — não modifique caracteres; use `TEXTO_FIXO` para buffers. `TAMANHO_VETOR` não confere em ponteiros (C-17, C-18): nos dois compiladores, `-Wsizeof-pointer-div` avisa, e o caso `n23` fixa isso.
7. **Tamanho de vetor**: literal inteiro ou `#define`; `CONSTANTE` (`static const`) **não** é expressão constante em C e faria VLA (C-23).
8. **`LEIA_TEXTO` consome um `\n` pendente, se houver, e lê a linha seguinte** — nunca descarta conteúdo quando nada está pendente (contrato C-20).
9. **Avaliação de argumentos**: `ABS`, `MINIMO`, `MAXIMO`, `ESCREVA`, `ESCREVAL` e `LEIA` avaliam cada argumento exatamente uma vez (P-11, verificado com `ABS(++i)`). A exceção é o **passo de `PARA_PASSO`**, reavaliado a cada iteração — não use expressão com efeito colateral como passo.
10. **Sem rede de segurança em tempo de execução**: sem verificação de limites de vetor, sem detecção de tipo em tempo de execução, sem exceções; estouro de inteiro com sinal é comportamento indefinido de C (PT-E).
11. **C11 obrigatório**: C99 é rejeitado com mensagem clara (falta `_Generic`); C++ não é suportado (D-15). Os *headers de módulo* (`include/basicgol/*.h`) podem ser usados de C++17.
12. **Controle de fluxo** (detalhes em [`docs/07-controle-de-fluxo.md`](docs/07-controle-de-fluxo.md) §8): não há `PARE` rotulado nem `goto` (use uma bandeira); `PARE` dentro de `ESCOLHA` sai só do `ESCOLHA`; `SENAO SE` é recusado pelo M1 mas o M0 o aceita como bloco aninhado; a forma `PARA (i, a, b) { }` usa chaves de C; limites como `INT_MAX` nunca são alcançados e laços sobre `NATURAL` com limite `n - 1` podem não terminar.
13. **`LEIA` só lê escalares numéricos e `CARACTERE`**: `LEIA(LOGICO)`, `LEIA(TEXTO)` e `LEIA(TEXTO_FIXO(...))` são **erro de compilação** (o `_Generic` do formato não tem `default`); para texto use `LEIA_TEXTO`/`LEIA_TEXTO_FIXO`. Antes da Fase 6 isso era apenas aviso — e corrompia a pilha (DF-01 em [`docs/13`](docs/13-defeitos-e-correcoes.md)).
14. **Diagnóstico no M0 aponta para o cabeçalho**: mensagens de macro citam `BG_*`; para diagnósticos que apontam para a linha do fonte do aluno, use o caminho M1 (`basicgolc`, erros com linha e coluna, §12.5 de `docs/01-especificacao.md`).

## Licença

Apache License 2.0 — ver [`LICENSE`](LICENSE).
