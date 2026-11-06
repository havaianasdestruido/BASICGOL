# 10 — Compilação e compiladores (Fase 6)

> **Escopo.** Este documento é o "como compilar" específico por ferramenta: os
> comandos, as flags que o projeto adota, as que ele **não** adota e por quê, o
> que muda entre GCC, Clang, MSVC, MinGW-w64 e `zig cc`, e como rodar cada
> suíte. O que foi de fato executado, com qual versão e com qual resultado está
> em [12-relatorio-fase-6.md](12-relatorio-fase-6.md); a visão de conjunto está
> em [11-matriz-compatibilidade.md](11-matriz-compatibilidade.md).

Nada aqui é obrigatório para escrever BASICGOL. Tudo aqui é obrigatório para
*saber se a linguagem continua portátil*.

---

## 1. O mínimo

A fonte BASICGOL é um `.c` comum (D-14). Não há extensão de arquivo, não há
`-x c`, não há passo de pré-processamento antes do compilador. Os helpers da
biblioteca vêm de `build/libbasicgol.a` (ou dos fontes de `src/biblioteca/`).

```sh
make                       # transpilador + build/libbasicgol.a
make test                  # suíte da linguagem + biblioteca
```

Programa mínimo, caminho M0 (macros):

```sh
cc -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include programa.c \
   build/libbasicgol.a -lm -o programa
```

Fluxo M1 (transpilador), quando a fonte usa o açúcar `PARA i DE … ATE … FACA`:

```sh
build/basicgolc programa.c -o programa.gerado.c
cc -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include programa.gerado.c \
   build/libbasicgol.a -lm -o programa
```

---

## 2. GCC

| Item | Valor usado na Fase 6 |
|---|---|
| Versão testada | `gcc (Debian 12.2.0-14+deb12u1) 12.2.0` |
| Padrões | `-std=c11`, `-std=c17`, `-std=gnu11`, `-std=c2x` |
| Biblioteca | `-lm` é obrigatória (glibc separa libm) |
| Archive | `ar rcs` (o Makefile faz) |

```sh
make CC=gcc
gcc -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include programa.c \
    build/libbasicgol.a -lm -o programa
```

Avisos que o GCC emite e que **são esperados** (não são defeitos):

* `-Wparentheses` em `a OU b E c` (sem parênteses). É o aviso do C sobre
  misturar `||` e `&&`; a solução é escrever `a OU (b E c)`.
* `-Wimplicit-fallthrough` em `ESCOLHA` com queda intencional. O comentário
  `/* fall through */` **não** funciona aqui (o rótulo `CASO` é macro e o GCC
  perde a anotação na expansão). Use `__attribute__((fallthrough))` — é o que o
  caso `03_controle/c19_escolha_fallthrough` faz.
* `-Wsizeof-pointer-div` em `TAMANHO_VETOR(ponteiro)` (C-18). O aviso é a rede
  de proteção: `sizeof(p) / sizeof(p[0])` compila e não mede o vetor.
* `-Wfloat-equal` **se** for ligado por conta própria: a biblioteca compara
  reais com `0.0` de propósito (`DIVIDE(a, 0.0)` é erro fatal documentado).
  Trocar por `fabs(b) < epsilon` mudaria a semântica; `-Wfloat-equal` não faz
  parte do conjunto adotado (§6).

## 3. Clang

Mesmas flags, mesma biblioteca, mesmo modo C11:

```sh
make CC=clang
clang -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include programa.c \
    build/libbasicgol.a -lm -o programa
```

Avisos específicos do Clang que o GCC não dá:

* `-Wlogical-not-parentheses` em `!x == y` (escreva `(!x) == y`).
* `-Wunreachable-code` (só com a flag ligada) aponta o `total = total + 100;`
  depois de um `PARE` incondicional.

Diferença de **diagnóstico** relevante para quem ensina: no erro de tipo de
`LEIA`, o GCC aponta a linha do cabeçalho (`include/basicgol.h:267`) e o Clang
aponta a linha do aluno (`programa.c:20`). O mesmo programa, o mesmo erro, o
mesmo código de saída — só o "onde" muda.

> **Nota honesta sobre a Fase 6.** Na máquina onde o relatório foi produzido
> não havia binário `clang`/`clang++` do LLVM, e não havia como instalá-lo
> (sem rede para `apt`, e os binários de release do LLVM ficam atrás de um host
> bloqueado). O frontend **Clang 20.1.2** foi executado através do `zig cc`
> 0.15.1 (ver §7): é o mesmo frontend e o mesmo gerador de código, com o driver
> do zig e `lld`. Isso **não** é o mesmo que testar o driver `clang` do LLVM, e
> o relatório trata os dois como coisas diferentes.

## 4. MSVC (`cl.exe`) — avaliação, **não executada**

> **Estado:** `cl.exe` **não foi executado** nesta fase. Não há Windows, não há
> MSVC e não há como rodá-lo neste ambiente. O que segue é uma avaliação a
> partir do que a linguagem usa e do que a Microsoft documenta; a coluna
> "verificado?" é sempre **não**. O trabalho de CI (`.github/workflows/testes.yml`,
> job `msvc`) compila com `cl /std:c11 /W4` em `windows-latest` e publica o
> resultado — enquanto esse job não estiver verde e registrado em
> [11](11-matriz-compatibilidade.md), **MSVC não pode ser chamado de suportado**.

```bat
cl /nologo /std:c11 /W4 /Iinclude /Fe:programa.exe ^
   programa.c ^
   src\biblioteca\consola.c src\biblioteca\matematica.c ^
   src\biblioteca\texto.c src\biblioteca\caractere.c ^
   src\biblioteca\utilidades.c
```

| Item | Situação |
|---|---|
| Modo C | `/std:c11` (ou `/std:c17`). C11/C17 exigem VS 2019 16.8+ (`_MSC_VER >= 1928`), que é exatamente o número que o cabeçalho confere antes de recusar C99. |
| `_Generic`, `_Static_assert`, `_Noreturn` | Presentes no MSVC desde 16.8 segundo a documentação da Microsoft; **não verificado aqui**. |
| Archive | Não há `ar`; ligue os cinco `.c` de `src/biblioteca/` direto no `cl`, como acima, ou gere um `.lib` com `lib.exe`. |
| `-lm` | Não existe; o CRT do Windows já traz a matemática. |
| `long double` | Igual a `double` no MSVC (64 bits). `%Lg` continua aceito pelo UCRT, mas a precisão é a de `double`. |
| **VLA** | O MSVC **não implementa VLA** (é recurso opcional do C11, e a Microsoft declarou `__STDC_NO_VLA__`). Isso reforça C-23: tamanho de `VETOR`/`TEXTO_FIXO` tem de ser literal ou `#define`, nunca `CONSTANTE` (`static const`) nem variável. |
| `BG_USAR_EXTENSOES` | **Não compila de propósito**: a seção 2b exige `__builtin_*_overflow` e emite `#error` sem GCC/Clang. É a decisão D-22 (recusar em vez de fingir). |
| `%hhd`, `%hhu`, `%llu`, `%Lf` | Suportados pelo UCRT (VS 2015+). |
| `ESCREVA(TEXTO NULL)` | **Perigo**: no glibc imprime `(null)`; no UCRT, `%s` com ponteiro nulo dispara o *invalid parameter handler* e aborta. Use `ESCREVA_TEXTO(s)` (que recusa NULL e devolve `FALSO`) ou teste antes. |
| Avisos | `/W4` é o equivalente aproximado de `-Wall -Wextra`. `/W4 /WX` para tratar aviso como erro. `/Wall` no MSVC liga avisos de qualidade duvidosa — não use como alvo. |
| Sanitizers | `/fsanitize=address` existe no MSVC, mas **não foi executado** aqui. |
| `__attribute__((fallthrough))` | Não existe; o `((void) 0)` do caso `c19` cobre esse caminho, e `/W4` não avisa queda de `case`. |

Construtores que o relatório lista como **risco para MSVC** (todos por causa de
C, não de BASICGOL): VLA (C-23), `__builtin_*` (D-22, é `#error`), `long double`
de 80 bits assumido (não assumido em nenhum lugar do código), e `printf("%s",
NULL)`.

## 5. MinGW-w64 (GCC no Windows)

Compila e liga com as mesmas flags do GCC; a diferença é o alvo. Nesta fase o
alvo foi exercitado **por cross-compilação** (`x86_64-windows-gnu` via `zig cc`),
sem execução (não há `wine`):

```sh
zig cc -target x86_64-windows-gnu -std=c11 -Wall -Wextra -Wformat=2 -Wvla \
    -I include docs/exemplos/ola_mundo.c src/biblioteca/*.c -o ola.exe
```

Com MinGW instalado, é simplesmente:

```sh
x86_64-w64-mingw32-gcc -std=c11 -Wall -Wextra -I include programa.c \
    build/libbasicgol.a -o programa.exe
```

## 6. Conjunto de avisos adotado

O Makefile compila tudo com:

```
-std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror
```

A suíte de portabilidade verifica um conjunto **maior** (não menor):

```
-std=c11 -Wall -Wextra -Wformat=2 -Wformat-security -Wvla -Wpedantic
-Wcast-qual -Wshadow -Wstrict-prototypes -Wmissing-prototypes
-Wmissing-declarations -Wold-style-definition -Wundef -Wwrite-strings
-Wpointer-arith -Wredundant-decls -Wswitch-enum -Wnull-dereference
-Wstrict-overflow=2 -Wmissing-field-initializers -Werror
```

Regra do projeto: **não se desliga aviso para "limpar" o build**. As duas
exceções que existiam foram tratadas na Fase 6:

1. `-Wcast-qual` em `texto.c` — era código de verdade; as três conversões
   `(char *)` que descartavam `const` foram substituídas por um ponteiro
   `const char *sobra`. Aviso resolvido, não silenciado.
2. `-Wno-implicit-fallthrough` no caso `c19` — era supressão por arquivo, que
   escondia também as quedas acidentais. Trocada por
   `__attribute__((fallthrough))` no ponto exato. Flag removida do caso.

`-Wfloat-equal` e `-Wconversion` **não** fazem parte do conjunto: o primeiro
marcaria a comparação com zero que é o contrato documentado; o segundo marcaria
toda conversão implícita do código do usuário (e a linguagem, por desenho, usa
as conversões de C — ver [06](06-tipos-e-expressoes.md)). Ligá-los é permitido,
mas o resultado é ruído, não defeito.

## 7. `zig cc` e alvos cruzados

`zig cc` é um driver que embute frontend Clang + `lld` + cabeçalhos de várias
libcs. Serve para duas coisas nesta fase:

1. **segundo compilador** (frontend Clang) quando não há um `clang` do LLVM;
2. **alvos cruzados** de verdade: i386, AArch64 e Windows, sem instalar
   toolchain.

```sh
zig cc -target x86_64-linux-gnu  ...   # frontend Clang, glibc, executa
zig cc -target x86-linux-musl    ...   # i386, estático, executa
zig cc -target aarch64-linux-gnu ...   # AArch64, só compila (sem qemu)
zig cc -target x86_64-windows-gnu ...  # MinGW-w64, compila e liga (sem wine)
```

`tests/run_portabilidade.sh` usa `zig` automaticamente **se** ele estiver no
`PATH`; se não estiver, as verificações de alvo cruzado aparecem como
**ignoradas** (nunca como aprovadas).

## 8. Sanitizers

Não são obrigatórios, e não são obrigatórios porque nem todo alvo os tem:
MSVC tem ASan, MinGW normalmente não, i386/musl depende da distribuição.

```sh
make test-sanitize        # suíte inteira em build-sanitize/ com ASan+UBSan+LSan
```

Equivalente manual (útil quando `cc` não é o compilador desejado):

```sh
make limpar
make CC=gcc CFLAGS="-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all" \
     T_CFLAGS="-std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror \
               -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all" test
```

Três detalhes que a Fase 6 descobriu na prática:

* **`CFLAGS` e `T_CFLAGS` têm de casar.** O Makefile não rastreia mudança de
  flags; se a biblioteca é instrumentada e o programa de teste não, o link
  falha com `undefined reference to __asan_init` (ou, pior, passa misturado).
  Por isso `make test-sanitize` usa `BUILD=build-sanitize`, um diretório
  separado.
* **`-fno-sanitize-recover=all`** transforma UB em falha imediata; sem isso o
  UBSan imprime e segue, e a suíte "passa" com UB no meio.
* **LeakSanitizer é parte do ASan no Linux.** Antes da Fase 6, o transpilador
  vazava tudo o que alocava e a suíte inteira falhava sob LSan — 129 "falhas"
  que não eram de semântica. Corrigido (defeito DF-02 em
  [13](13-defeitos-e-correcoes.md)); hoje `make test-sanitize` fecha verde.

## 9. C++ (uso opcional da biblioteca)

O cabeçalho da **linguagem** (`basicgol.h`) é só C (D-15): usa `_Generic` e
falha em C++ com a mensagem `BASICGOL exige C11 ou superior`. Os cabeçalhos dos
**módulos** (`include/basicgol/*.h`) declaram `extern "C"` e são usáveis de C++
desde que `_Noreturn` virou `BG_NORETURN` (defeito DF-04):

```cpp
#include <basicgol/biblioteca.h>   // ok em C++17
int main() { return bg_e_digito('7') ? 0 : 1; }
```

```sh
c++ -std=c++17 -Wall -Wextra -Werror -I include meu.cpp build/libbasicgol.a -lm -o meu
```

## 10. Rodando as suítes

| Comando | O que roda | Precisa de |
|---|---|---|
| `make test` | 307 verificações da linguagem (M0 + M1) + 230 unitárias + 30 de integração | um compilador C11 |
| `make test-v` | idem, listando cada verificação aprovada | — |
| `make test-biblioteca` | só os módulos da biblioteca | — |
| `make test-portabilidade` | matriz de compiladores, avisos, multi-UO, C++, sanitizers, alvos cruzados | `make` feito; opcionalmente `zig`, `g++`, `nm` |
| `make test-sanitize` | a suíte inteira sob ASan + UBSan + LSan | compiler com sanitizers |
| `make test-matriz` | a suíte inteira com **cada** compilador encontrado | idem |
| `make fuzz` | 300 mutações determinísticas no transpilador | `python3` |
| `make test-checkout` | os comandos deste documento a partir de um checkout limpo | `git`, `make` |
| `make cobertura` | regenera `tests/COBERTURA.md` (saída determinística) | `python3` |

Cada uma delas imprime no fim uma linha `... OK: N verificações` ou a lista de
falhas. A matriz de portabilidade também é escrita em
`build/relatorio/portabilidade.tsv` (formato TSV: compilador, resultado,
verificação), para colar em relatório ou comparar entre máquinas.

## 11. Armadilhas de build (todas observadas de verdade)

1. **O Makefile não rastreia compilador nem flags.** `make CC=clang` depois de
   `make CC=gcc` **não** recompila nada. Use `make limpar` ou um `BUILD=`
   diferente (é o que `test-sanitize` e `test-matriz` fazem).
2. **`make` sem `biblioteca`** deixa `build/libbasicgol.a` fora; os programas
   que usam helpers falham no link, não na compilação. `make` já constrói os
   dois.
3. **`-lm` no fim da linha**, depois dos objetos/archive, ou o linker não acha
   `pow`, `sqrt`, `llround` em plataformas que separam libm.
4. **`build/basicgolc` é obrigatório no caminho M1**; a suíte avisa com código
   2 (e não "falha de teste") quando ele não existe.
5. **`make cobertura` é determinística** — a Fase 6 removeu a data do arquivo
   gerado; sem isso a checagem de CI (`git diff --exit-code`) falhava todo dia
   seguinte ao commit. `COBERTURA_ARGS=--data` gera a versão datada.
