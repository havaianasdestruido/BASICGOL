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
| 10 | TCC, Intel oneAPI (`icx`), PCC, Digital Mars, Open64 | — | — | indisponíveis | **não testado** (ver também a §8, que amplia esta lista com os candidatos coletados) |
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
  PCC, execução em AArch64 e em Windows, `wine`, sanitizers no MSVC — e toda a
  §8 (outros ~45 compiladores C conhecidos, do mais ao menos conhecido; nada
  foi executado).

Nenhum item de "avaliado" ou "não testado" autoriza escrever "suportado" em
lugar nenhum do projeto.

---

## 8. Candidatos conhecidos, não obtidos (lista coletada; nada executado)

> **Nada nesta seção foi executado, compilado ou avaliado.** Nenhum destes
> compiladores existe na máquina onde esta matriz foi produzida; pelo
> vocabulário do cabeçalho da página, todos são **não testado**. As colunas de
> licença e de conformidade foram copiadas da [lista de compiladores da
> Wikipédia](https://en.wikipedia.org/wiki/List_of_compilers) (seção C; a
> própria página se declara "incomplete list" desde março de 2009) e de
> comentários do Reddit, e servem apenas como **triagem** — não como promessa
> de conformidade e não como leitura de documentação de fornecedor. Onde a
> fonte traz `?`, a célula traz `?`. A Wikipédia e o Reddit não são fontes do
> fornecedor: nenhum item aqui gradua para "avaliado" sem trabalho de verdade
> (ler a doc do fornecedor e o código), e para "executado"/"compilado" só com
> comando rodado e saída conferida — nesse caso a linha migra para a §1.
>
> **Ordem:** do mais conhecido para o menos conhecido. O critério é subjetivo
> (visibilidade pública, histórico, presença em discussões); não é prioridade
> de teste, não é ranking de qualidade e não altera o estado de nenhum item.
>
> GCC, Clang, MSVC, MinGW-w64 e G++ já estão nas §1 e §4 e **não** são
> repetidos aqui. TCC, `icx`, PCC, Digital Mars e Open64 já constam em §1,
> item 10, e aparecem também abaixo para manter a ordenação completa.
> `tests/run_matriz.sh` só descobre `cc`, `gcc`, `clang*`, `tcc` e o que
> `$T_CC_ALT` indicar no `PATH`: nenhum item desta tabela entra na suíte
> enquanto não for instalado.

| # | Compilador (autor / organização) | SO (segundo a fonte) | Licença (segundo a fonte) | C89 | C99 | C11 | C17 | Notas e links |
|---|---|---|---|---|---|---|---|---|
| 1 | [Tiny C Compiler (TCC)](http://bellard.org/tcc/) — Fabrice Bellard | Win ✓, Unix ✓ (bare-metal: não) | LGPL | sim | parcial | parcial | ? | Reddit: "o compilador C99 mais rápido conhecido" (Bellard). Já em §1, item 10. |
| 2 | [Intel oneAPI icx](https://www.intel.com/) — Intel | Win ✓, Unix: Linux (outros: não) | Freeware (suporte prioritário opcional) | sim | sim | sim | sim | Conformidade sim×4 segundo a Wikipédia (nota 13 da lista). Já em §1, item 10. |
| 3 | Intel C++ Compiler Classic (icc) — Intel | Win ✓, Unix: Linux, macOS | Freeware (suporte prioritário opcional) | sim | parcial | parcial | ? | Mesma família do item 2; a Wikipédia o lista separadamente. |
| 4 | [Oracle C compiler](http://www.oracle.com/technetwork/server-storage/solarisstudio/downloads/index-jsp-141149.html) (Oracle Solaris Studio) — Oracle | Unix: Solaris, Linux (Win: não) | Proprietário (Freeware) | sim | sim | sim | não | Sugerido por [u/wang_li](https://www.reddit.com/user/wang_li/). O link do Reddit é o do antigo Solaris Studio (provavelmente datado). O comentário u/FUZxxl observa que não há link oficial para o compilador C da Sun — que virou este. |
| 5 | [IBM XL C/C++](https://en.wikipedia.org/wiki/List_of_compilers) — IBM | Unix: AIX, Linux, z/OS, z/VM (Win: não) | Proprietário | sim | sim | sim | sim | Conformidade sim×4 segundo a Wikipédia. |
| 6 | [pcc (Portable C Compiler)](http://pcc.ludd.ltu.se/) — Stephen C. Johnson, Anders Magnusson e outros | Win ✓, Unix ✓, outros ✓ | BSD | sim | parcial | não | ? | Reddit: "pcc, o compilador C99 portátil". O ramo moderno é mantido por Magnusson (Wikipédia). Já em §1, item 10. |
| 7 | [lcc](https://sites.google.com/site/lccretargetablecompiler/) — Chris Fraser, David Hanson | Win ✓, Unix ✓, outros ✓ | Freeware (fonte para uso não comercial) | sim | não | não | ? | Reddit: "lcc, um compilador C99 reorientável". A Wikipédia o lista como reorientável. |
| 8 | [Open Watcom / Watcom C/C++](https://github.com/open-watcom/open-watcom-v2) — Watcom (hoje o projeto open-watcom) | Win (experimental, segundo a Wikipédia), DOS, OS/2 | Sybase Open Watcom Public License | sim | parcial | não | ? | Sugerido por [u/staalmannen](https://www.reddit.com/user/staalmannen/): "os únicos compiladores 16-bit decentes ainda vivos". |
| 9 | [SDCC (Small Device C Compiler)](http://sdcc.sourceforge.net/) — Sandeep Dutta e outros | Win ✓, Unix ✓, bare-metal ✓ | GPL | ? | ? | ? | ? | Sugerido por [u/skeeto](https://www.reddit.com/user/skeeto/): "um compilador ANSI C reorientável". Voltado a embebidos/microcontroladores. |
| 10 | [VBCC](http://sun.hasenbraten.de/vbcc) — Volker Barthelmann | Win ✓, Unix ✓, outros ✓ | Freeware (fonte disponível; modificação não permitida) | sim | parcial | não | ? | Sugerido por [u/dmitrinove](https://www.reddit.com/user/dmitrinove/): "um compilador ISO C portátil e reorientável altamente otimizado". |
| 11 | [AOCC (AMD Optimizing C/C++ Compiler)](https://en.wikipedia.org/wiki/List_of_compilers) — AMD | Unix ✓ (Win: não) | Proprietário | sim | sim | sim | sim | Fork da AMD sobre LLVM/Clang (conhecimento geral; não verificado). |
| 12 | [Digital Mars](https://en.wikipedia.org/wiki/List_of_compilers) — Digital Mars | Win ✓ (Unix: não) | Proprietário | ? | ? | ? | ? | Já em §1, item 10. |
| 13 | [Open64](https://en.wikipedia.org/wiki/List_of_compilers) — AMD, SGI, Google, HP, Intel, Nvidia, PathScale, Tsinghua e outros | Unix ✓, outros ✓ (Win: não) | GPL | ? | ? | ? | ? | Já em §1, item 10. |
| 14 | [PGCC](https://en.wikipedia.org/wiki/List_of_compilers) — The Portland Group | Win ✓, Unix ✓, outros: desconhecido | Proprietário | ? | ? | ? | ? | Linha de otimização para HPC da Portland Group (conhecimento geral; não verificado). |
| 15 | [Pelles C](http://www.smorgasbordet.com/pellesc/) — Pelles Software (a fonte não informa) | Win ✓ — IDE completa: compilador, assembler, macro assembler e mais | não informada na fonte colada; freeware (conhecimento geral, não verificado) | ? | ? | ? | ? | Sugerido por [u/systemsock](https://www.reddit.com/user/systemsock/): "uma pequena IDE para Windows". |
| 16 | [CompCert](https://en.wikipedia.org/wiki/List_of_compilers) — INRIA | Win ✓, Unix ✓ | Freeware (uso não comercial) ou GPL | sim | parcial | não | ? | Compilador C formalmente verificado (conhecimento geral; não verificado). |
| 17 | [Turbo C](https://en.wikipedia.org/wiki/List_of_compilers) — Borland; hoje Embarcadero | Win ✓, outros ✓ (Unix: não) | Proprietário; a v2.01 é livremente disponível segundo a Wikipédia | ? | ? | ? | ? | Histórico; marco da era DOS. |
| 18 | [C++Builder](https://en.wikipedia.org/wiki/List_of_compilers) — Embarcadero | Win ✓, Unix: iOS e Android (segundo a Wikipédia) | Proprietário | sim | sim | parcial | ? | IDE C++ que também aceita C; não é um compilador C puro. |
| 19 | [cc65](https://en.wikipedia.org/wiki/List_of_compilers) — não consta na fonte | Unix ✓, bare-metal ✓ (Win: não) | Zlib License | não | não | não | não | Alvo 6502 (nome do projeto); popular na retrocomputação. |
| 20 | [Edison Design Group (EDG)](https://en.wikipedia.org/wiki/List_of_compilers) — Edison Design Group | Win ✓, Unix ✓, outros ✓ | Proprietário | sim | sim | sim | sim | Front-end C/C++ licenciado a terceiros e usado como base de outros produtos (conhecimento geral; não verificado). |
| 21 | [bcc (Bruce's C Compiler)](http://linux.die.net/man/1/bcc) — Bruce Evans | Unix ✓ (Win: não) | "GNU License" (como consta na Wikipédia) | ? | ? | ? | ? | Sugerido por [u/skeeto](https://www.reddit.com/user/skeeto/): "Compilador ANSI C do Bruce". Não confundir com o `bcc` de BPF/clang-tools (precisão minha, não da fonte). |
| 22 | [Ch](https://en.wikipedia.org/wiki/List_of_compilers) — SoftIntegration, Inc | Win ✓, Unix: macOS, FreeBSD, Linux, Solaris, HP-UX, AIX, QNX; bare-metal ✓ | Freeware | sim | sim | não | ? | Interpretador C/C++ embutível (conhecimento geral; não verificado). |
| 23 | [SEGGER Compiler](https://en.wikipedia.org/wiki/List_of_compilers) — Segger Microcontroller | Win ✓, Unix ✓, outros ✓ | Proprietário | sim | sim | parcial | parcial | Compilador para mercado embutido. |
| 24 | [cproc](https://en.wikipedia.org/wiki/List_of_compilers) — Michael Forney | Win ✓, Unix ✓ (outros: não) | ISC | sim | sim | sim | sim | Rende com o backend QBE (conhecimento geral; não verificado); espírito minimalista. |
| 25 | [scc](http://git.suckless.org/scc/) — projeto suckless | não consta na fonte | não informada na fonte (projetos suckless costumam usar MIT; não verificado) | ? | ? | ? | ? | Reddit: compilador C99 do projeto suckless. |
| 26 | [8cc](https://github.com/rui314/8cc) — Rui Ueyama | Win ✓, Unix ✓ | MIT | sim | sim | sim | não | Reddit: "um compilador C11 pequeno e auto-compilável". Mesmo autor do `chibicc` (conhecimento geral; não verificado). |
| 27 | [Amsterdam Compiler Kit (ack)](https://github.com/davidgiven/ack) — Andrew Tanenbaum, Ceriel Jacobs | Unix ✓, outros ✓ (Win: não) | BSD | ? | ? | ? | ? | Reddit: "o Amsterdam Compiler Kit, originalmente escrito para Minix. C89". |
| 28 | [Small-C](https://en.wikipedia.org/wiki/List_of_compilers) — Ron Caine, James E. Hendrix (*Byte* magazine) | Win ✓, Unix ✓, outros: CP/M, DOS | Domínio público | parcial | não | não | ? | Histórico; marcou a era CP/M. |
| 29 | [Interactive C](https://en.wikipedia.org/wiki/List_of_compilers) — KISS Institute for Practical Robotics | Win ✓, Unix: "Unix", macOS, Linux, IRIX, Solaris, SunOS | Freeware | parcial | não | não | ? | Voltado a robótica educacional. |
| 30 | [Wind River (Diab) Compiler](https://en.wikipedia.org/wiki/List_of_compilers) — Wind River Systems | Win ✓, Unix ✓, outros ✓ | Proprietário | ? | ? | ? | ? | Mercado embutido (conhecimento geral; não verificado). |
| 31 | [Aztec C](https://en.wikipedia.org/wiki/List_of_compilers) — Manx Software Systems | outros: CP/M, CP/M-86, DOS, Classic Mac OS | Proprietário | ? | ? | ? | ? | Clássico da era 8/16 bits e do Classic Mac OS. |
| 32 | [THINK C / Lightspeed C](https://en.wikipedia.org/wiki/List_of_compilers) — THINK Technologies | outros: Classic Mac OS | Proprietário | ? | ? | ? | ? | Clássico do Classic Mac OS. |
| 33 | [Lattice C](https://en.wikipedia.org/wiki/List_of_compilers) — Lifeboat Associates | Unix ✓, outros: DOS, OS/2, Commodore, Amiga, Atari ST, Sinclair QL | Proprietário | ? | ? | ? | ? | Popular em DOS/Amiga/Atari ST. |
| 34 | [MPW C](https://en.wikipedia.org/wiki/List_of_compilers) — Apple | outros: Classic Mac OS | Proprietário | ? | ? | ? | ? | Macintosh Programmer's Workshop (nome por extenso na Wikipédia). |
| 35 | [QuickC](https://en.wikipedia.org/wiki/List_of_compilers) — Microsoft | Win ✓ (Unix: não) | Proprietário | ? | ? | ? | ? | Ancestral leve do Visual C++ para DOS/Windows. |
| 36 | [Acorn C/C++](https://en.wikipedia.org/wiki/List_of_compilers) — Acorn, Codemist | outros: RISC OS | Proprietário | sim | sim | não | sim | Linha copiada como consta na Wikipédia (desfecho incomum: C17 "sim" com C11 "não"; não auditada). |
| 37 | [BDS C](https://en.wikipedia.org/wiki/List_of_compilers) — BD Software | outros: CP/M | Domínio público | ? | ? | ? | ? | Histórico; CP/M. |
| 38 | [Digital Research C](https://en.wikipedia.org/wiki/List_of_compilers) — Digital Research | outros: CP/M, DOS | Proprietário | ? | ? | ? | ? | A própria Wikipédia marca "better source needed" no link da lista. |
| 39 | [Whitesmiths C](https://en.wikipedia.org/wiki/List_of_compilers) — Whitesmiths Ltd | Unix ✓ (Win: não) | Proprietário (fonte disponível para uso não comercial) | não | ? | não | ? | Um dos primeiros compiladores C comerciais (conhecimento geral; não verificado). |
| 40 | [Tandem C](https://en.wikipedia.org/wiki/List_of_compilers) — Tandem Computers | Unix ✓, outros: Guardian, NonStop Kernel, NonStop OS | Proprietário | sim | ? | ? | parcial | Mainframe tolerante a falhas. |
| 41 | [MCP (Unisys)](https://en.wikipedia.org/wiki/List_of_compilers) — Unisys | outros: MCP (Burroughs) | Proprietário | ? | ? | ? | ? | Linha Burroughs/Unisys. |
| 42 | [kencc](http://gsoc.cat-v.org/projects/kencc/) e os [compiladores Plan 9](http://plan9.bell-labs.com/sys/doc/compiler.html) — Bell Labs (Plan 9) | **não roda em Linux, macOS ou Windows** (segundo a fonte) | não informada na fonte colada | ? | ? | ? | ? | Sugerido por [u/FUZxxl](https://www.reddit.com/user/FUZxxl/). A fonte não achou link oficial para o compilador C da Sun (item 4). |
| 43 | [Snyder-C-compiler (Alan Snyder's Portable C Compiler)](https://github.com/PDP-10/Snyder-C-compiler) — Alan Snyder; manutenção atual larsbrinkhoff | Unix ✓ (Win: não) | MIT | ? | ? | ? | ? | Alvo PDP-10; preservação histórica. |
| 44 | [c](https://github.com/andrewchambers/c) — Andrew Chambers | não consta na fonte | não informada na fonte colada | ? | ? | sim (Reddit) | ? | Reddit: "tentativa de um compilador C11 muito elegante e pequeno por Andrew Chambers". |
| 45 | [fcc](https://github.com/Fedjmike/fcc) — Fedjmike | não consta na fonte | não informada na fonte colada | ? | ? | sim (Reddit) | ? | Reddit: "Compilador C11 do Fedjmike". |
| 46 | [nwcc](http://nwcc.sourceforge.net/) — Nils Weller | não consta na fonte | não informada na fonte colada | sim (Reddit) | ? | ? | ? | Reddit: "compilador C89 escrito por Nils Weller. Já capaz de compilar aplicações como, por exemplo, bash e lynx". |
| 47 | [neatcc](http://repo.or.cz/w/neatcc.git) — não consta na fonte | não consta na fonte | não informada na fonte colada | sim, parcial/subconjunto ("sub-C89", Reddit) | não | não | não | Reddit: "compilador sub-C89 com os objetivos simplicidade, usabilidade e confiabilidade". Manual: [neatcc.pdf](http://litcave.rudi.ir/neatcc.pdf). |

**Fontes.** Wikipédia, *List of compilers* — seção C
(<https://en.wikipedia.org/wiki/List_of_compilers>), conforme colada nesta
sessão em 2026-11-06 (a página se declara incompleta desde março de 2009);
comentários do Reddit coletados pelo usuário (u/FUZxxl, u/systemsock,
u/wang_li, u/staalmannen, u/skeeto, u/dmitrinove); páginas oficiais
linkadas nas linhas; notas marcadas "conhecimento geral, não verificado" são
indicações de contexto, não resultados de teste. Onde havia conflito ou
ausência de dado, a célula registra `?` em vez de escolher. Anomalias copiadas
da Wikipédia (p. ex. Acorn C/C++: C11 "não" com C17 "sim") foram mantidas como
constam na fonte, sem auditoria.

**Para promover um item desta tabela:** instalar/obter o compilador, rodar a
suíte (`make test-matriz` serve de referência, embora hoje só descubra
`cc`/`gcc`/`clang*`/`tcc`/`$T_CC_ALT`), registrar o resultado na §1 com
versão, alvo, como foi obtido e estado honesto ("executado"/"compilado"/
"avaliado"), e atualizar a §7. A mera presença nesta tabela não autoriza a
palavra "suportado" em lugar nenhum do projeto.
