# 11 — Matriz de compatibilidade (Fase 6)

> **Como ler esta página.** Cada célula tem um dos quatro estados abaixo, e
> nenhum outro. Não existe "deve funcionar" sem rótulo.
>
> | Estado | Significa |
> |---|---|
> | **executado** | Compilou aqui **e** o binário rodou aqui, com a saída conferida. |
> | **compilado** | Compilou (e ligou, quando o alvo tem ligador); o binário **não** rodou. |
> | **avaliado** | Nenhum comando foi executado. A conclusão vem da documentação do fornecedor e da leitura do código, e está marcada como tal. |
> | **não testado** | Não há informação suficiente nem para avaliar. |
>
> Toda linha "executado" tem a saída correspondente no
> [relatório da fase](12-relatorio-fase-6.md), e o `.tsv` cru fica em
> `build/relatorio/portabilidade.tsv` (regenerado por `make test-portabilidade`).

---

## 1. Compiladores e alvos

| # | Compilador | Versão | Alvo | Como foi obtido | Estado |
|---|---|---|---|---|---|
| 1 | GCC | 12.2.0 (Debian 12) | `x86_64-linux-gnu`, glibc 2.36 | nativo | **executado** |
| 2 | Clang (frontend) | 20.1.2, via `zig cc` 0.15.1 | `x86_64-linux-gnu`, glibc | `zig cc` | **executado** |
| 3 | GCC | 12.2.0 | `i386-linux-musl` (estático) | `zig cc -target x86-linux-musl` | **executado** (roda no kernel x86_64) |
| 4 | GCC | 12.2.0 | `aarch64-linux-gnu` | `zig cc` (compila), sem `qemu` | **compilado** |
| 5 | Clang/MinGW-w64 | 20.1.2 / lld | `x86_64-windows-gnu` | `zig cc -target x86_64-windows-gnu` | **compilado** (sem `wine`) |
| 6 | G++ | 12.2.0 | `x86_64-linux-gnu`, `-std=c++17` | nativo | **executado** (só os cabeçalhos de módulo) |
| 7 | MSVC `cl.exe` | — | Windows | — | **avaliado** (§4) |
| 8 | MinGW-w64 GCC nativo | — | Windows | — | **não testado** (só o alvo 5, via cross) |
| 9 | Clang do LLVM (driver `clang`/`clang-cl`) | — | — | indisponível na máquina | **não testado** (o frontend é o do item 2) |
| 10 | TCC, Intel oneAPI (`icx`), PCC, Digital Mars, Open64 | — | — | indisponíveis | **não testado** |
| 11 | `wine` (execução de binários Windows) | — | — | ausente | **não testado** |

Observação sobre o item 2: o `zig cc` **é** o frontend Clang, mas o driver, o
ligador (`lld`) e os cabeçalhos de libc vêm do zig. Testar por ele prova a
portabilidade **do código** ao frontend Clang; não prova que o driver `clang`
do LLVM, com `-Wl` e `libgcc` do sistema, se comporte igual. Por isso o item 9
continua "não testado".

## 2. Padrões de C

| Padrão | GCC 12 | Clang 20 (zig) | Estado |
|---|---|---|---|
| `-std=c89` / `-ansi` | recusado pelo cabeçalho, com `#error` explicando o motivo | idem | **executado** |
| `-std=c99` | recusado pelo cabeçalho (`_Generic` é C11) | idem | **executado** |
| `-std=c11` | compila e passa a suíte inteira | compila e passa a suíte inteira | **executado** |
| `-std=c17` | compila e passa a suíte inteira | compila e passa a suíte inteira | **executado** |
| `-std=gnu11` | compila limpo | compila limpo | **executado** |
| `-std=c23` / `-std=c2x` | compila limpo | o driver do zig recusa `-std=c23` | **executado** (GCC) / **ignorado** (Clang) |
| `-std=c++17` com `<basicgol.h>` | `g++` para no `#error` "BASICGOL exige C11" | — | **executado** (é o comportamento pretendido, D-15) |
| `-std=c++17` com `<basicgol/*.h>` | compila, liga e roda | — | **executado** |

C11 é o piso. C17 e C23 são o mesmo código; o cabeçalho não usa nada que só
exista em C23.

## 3. Recursos da linguagem × compilador

"Usado" é o que `include/basicgol.h` e `src/` realmente empregam.

| Recurso | Usado? | GCC 12 | Clang 20 | MSVC | Observação |
|---|---|---|---|---|---|
| `_Generic` | sim (núcleo) | executado | executado | avaliado | Sem `_Generic` não há `LEIA`/`ESCREVA` por tipo; é a razão do piso C11. |
| `_Static_assert` | sim (na suíte) | executado | executado | avaliado | O caso `09_tipos_expr/t14_propriedades` usa `_Static_assert` para travar o mapeamento `INTEIRO`→`int`, `NATURAL`→`unsigned` etc.; se o mapeamento mudar, o build falha. O cabeçalho em si não precisa (a checagem de padrão é `#error`). |
| `_Noreturn` / `BG_NORETURN` | sim | executado | executado | avaliado | Em C: `_Noreturn`; em C++: `[[noreturn]]`; MSVC 16.8+: `_Noreturn` (defeito DF-04). |
| `inline` em C | sim (`static inline`) | executado | executado | avaliado | |
| `restrict` | não | — | — | — | Não usado de propósito: não muda nada aqui e restringe o chamador. |
| VLA | não | — | — | **risco** | C-23: tamanho de vetor/texto é literal ou `#define`. MSVC não tem VLA. |
| `__builtin_*_overflow` | opcional (§2b) | executado | executado | **`#error`** | D-22: sem GCC/Clang, `BG_USAR_EXTENSOES` recusa compilar em vez de fingir. |
| `__attribute__((format))` | sim, sob `__GNUC__` | executado | executado | ignorado | MSVC não define `__GNUC__`; os atributos desaparecem. |
| `#pragma clang diagnostic` | sim, sob `__clang__` | ignorado | executado | ignorado | Bloco de 3 linhas que desliga `-Wformat-nonliteral` **só** no `bg_escreva_formato`, que é um `vprintf` de verdade. |
| `long double` de 80 bits | não assumido | — | — | 64 bits | Nenhum código assume precisão estendida; `%Lg` sai com a precisão nativa. |
| Ordem de avaliação / comportamento indefinido | — | ASan+UBSan limpo | ASan+UBSan limpo | não testado | 2000 mutações no transpilador sob ASan+UBSan sem achado. |

## 4. MSVC — avaliação (nada executado)

`cl.exe` não existe nesta máquina e não há Windows. O que está abaixo é
**avaliação**, derivada da página de conformidade da Microsoft e da leitura do
código; a coluna "verificado" é sempre **não**.

| Ponto | Expectativa | Base | Verificado? |
|---|---|---|---|
| `/std:c11`, `/std:c17` | disponíveis a partir do VS 2019 16.8 (`_MSC_VER >= 1928`) | documentação de conformidade C/C++ da Microsoft | não |
| `_Generic`, `_Static_assert`, `_Noreturn`, `<stdnoreturn.h>` | suportados desde 16.8 | idem | não |
| `<stdalign.h>`, `restrict`, `_Pragma` | **desnecessários**: o projeto não os usa | — | — |
| VLA | **não suportado** (recurso opcional; a Microsoft declara `__STDC_NO_VLA__`) | idem | não |
| `%hhd`, `%hhu`, `%llu`, `%Lf`, `snprintf`, `vsnprintf` | suportados pelo UCRT (VS 2015+) | documentação do UCRT | não |
| `printf("%s", NULL)` | **comportamento diferente**: UCRT aborta pelo *invalid parameter handler*; glibc imprime `(null)` | documentação do UCRT | não |
| `-lm` | não existe (CRT já traz a matemática) | — | — |
| `/W4` | equivalente aproximado de `-Wall -Wextra` | — | não |
| `/fsanitize=address` | existe, não executado | — | não |

O workflow `.github/workflows/testes.yml` tem um job `msvc` que faz
`cl /std:c11 /W4` e o caminho M1 completo em `windows-latest`. Ele está marcado
com `continue-on-error` **porque nunca rodou**: enquanto não houver um registro
dele verde, MSVC fica nesta seção, não na §1. É a diferença entre "avaliado" e
"suportado", e o projeto não confunde as duas.

## 5. Pressupostos de plataforma

Estes são os pontos em que o C deixa a decisão para a implementação. Cada um
está listado com o que o BASICGOL faz a respeito.

| Pressuposto | Verdade em x86-64 Linux (testado) | O que o BASICGOL faz |
|---|---|---|
| `INTEIRO` = `int` de 32 bits | sim — `fronteira.c` exercita `INT_MAX`, `INT_MIN`, estouro de soma e divisão por zero (176 verificações) | `_Static_assert` da suíte (`t14_propriedades`) trava o mapeamento de tipos; `MAXIMO_INTEIRO`/`MINIMO_INTEIRO` vêm de `<limits.h>`, nunca de números escritos à mão. |
| `CARACTERE` = `char`, sinal não especificado | assinado em x86, **não assinado** em AArch64 Linux | `src/biblioteca/caractere.c` classifica por **faixa ASCII explícita** (`c >= '0' && c <= '9'`), sem `<ctype.h>` e sem `(unsigned char)`: as duas convenções de sinal dão o mesmo resultado porque as faixas são positivas e disjuntas, então `0xC3` é "não é dígito" nos dois casos. `ESCREVA(CARACTERE)` usa `%c`. |
| `sizeof(void *)` == `sizeof(INTEIRO * )` | sim | Não é assumido; `%p` é usado só para depuração. |
| `long double` | 80 bits em x86-64 Linux, 64 em MSVC/ARM | Não assumido; `REAL_LONGO` existe, mas nenhum teste depende de precisão estendida. |
| Fim de linha | LF; fonte com CRLF também é aceita | O transpilador normaliza: a saída M1 de uma fonte CRLF é **idêntica** à de uma fonte LF (verificado); BOM UTF-8 é preservado sem erro. |
| Texto vs. binário | Linux não diferencia | `basicgolc` abre entrada e saída em modo binário (`"rb"`/`"wb"`), então no Windows a saída não ganha `\r` automático. |
| `CHAR_BIT` | 8 | Assumido: o transpilador opera byte a byte e `BG_BITS_DE` deriva de `CHAR_BIT`. Não há máquina com `CHAR_BIT != 8` disponível para testar, então isto é **assumido**, não verificado. |

AArch64 ficou em **compilado**: sem `qemu-aarch64` não há como executar, e o
ponto interessante (a diferença de sinal do `char`) não foi exercitado em
execução. Isso é uma lacuna declarada, não uma aprovação.

## 6. Bibliotecas de C

| Biblioteca | Onde aparece | Situação |
|---|---|---|
| `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`, `<errno.h>` | biblioteca e transpilador | C89; nenhuma extensão. |
| `<math.h>` | `matematica.c` (`llround`, `sqrt`, `pow`, `fmod`) | `llround` é C99; presente em glibc, musl, UCRT e MinGW. |
| `<stdint.h>`, `<stdbool.h>`, `<limits.h>`, `<float.h>` | cabeçalho | C99/C11 padrão. |
| `<locale.h>` | **não usado** | `ESCREVA(REAL)` formata com o `%g` do locale corrente — em locale com vírgula decimal, a saída muda. A suíte roda em `C`/`POSIX`. Documentado como decisão D-17 / suposição A-5. |
| `<unistd.h>`, `<windows.h>` | **não usados** | Nenhum código dependente de plataforma; é por isso que o transpilador compila para Windows sem `#ifdef` de plataforma. |

## 7. Resumo do que a matriz sustenta

* **Sustentado por execução:** GCC 12 e frontend Clang 20, em C11/C17/gnu11,
  x86-64 Linux; 32 bits (i386/musl) em execução para `precedencia` e
  `fronteira`; C++17 para os cabeçalhos de módulo; recusa correta de C89, C99 e
  de `<basicgol.h>` em C++.
* **Sustentado por compilação:** AArch64 Linux e Windows x86-64 (MinGW-w64).
* **Avaliado, não executado:** MSVC (VS 2019 16.8+), com os riscos listados na
  §4.
* **Não testado:** clang/clang-cl do LLVM como driver, MinGW nativo, TCC, icx,
  PCC, execution em AArch64 e em Windows, `wine`, sanitizers no MSVC.

Nenhum item de "avaliado" ou "não testado" autoriza escrever "suportado" em
lugar nenhum do projeto.
