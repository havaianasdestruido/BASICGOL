# 12 — Relatório da Fase 6: portabilidade, múltiplos compiladores e endurecimento

> **Regra deste documento.** Tudo o que está marcado como **executado** foi
> rodado na máquina descrita na §2, e o resultado é o que está escrito. O que
> não foi rodado está na §8, com o motivo. Nenhum número aqui é estimado.

---

## 1. Sumário

| # | Comando | O que prova | Resultado |
|---|---|---|---|
| 1 | `make test` | a suíte completa passa no compilador padrão | **307** verificações de linguagem + **230** unitárias + **30** de integração |
| 2 | `make test-matriz` | a mesma suíte passa em **dois frontends independentes** | GCC 12.2.0 e Clang 20.1.2: 307 + 230 + 30 em cada |
| 3 | `make test-sanitize` | não há UB nem vazamento | 307 + 30 sob ASan + UBSan + LSan, `-fno-sanitize-recover=all` |
| 4 | `make test-portabilidade` | cabeçalhos, padrões, avisos, multi-UO, C++, alvos cruzados | **70 aprovadas, 2 ignoradas, 0 falhas** |
| 5 | `make fuzz` | o transpilador não cai com entrada malformada | **2000** mutações, 0 falhas |
| 6 | `make test-checkout` | a documentação funciona do zero | **13** comandos/verificações |
| 7 | `make exemplos` | os 24 exemplos compilam com aviso = erro | 24 `ok` |
| 8 | `make cobertura` | todo construtor documentado tem teste | **117 nomes, 135 casos**, saída determinística |

Saída literal das linhas finais:

```
$ make test
TUDO OK: 307 verificacoes aprovadas
biblioteca: 230 verificacoes aprovadas
TUDO OK: 30 verificacoes da biblioteca aprovadas

$ ./tests/run_checkout_limpo.sh
CHECKOUT LIMPO OK: 13 comandos/verificacoes

$ ./tests/run_portabilidade.sh
PORTABILIDADE OK: 70 verificacoes aprovadas, 2 ignoradas

$ T_CC_ALT=/tmp/bin/clang-zig ./tests/run_matriz.sh
MATRIZ OK: 2 compilador(es): cc /tmp/bin/clang-zig

$ python3 tests/fuzz_basicgolc.py --basicgolc ./basicgolc-asan --iteracoes 2000
fuzz_basicgolc: 2000 mutacoes sem crash, 0 falha(s) [seed=20261106, iteracoes=2000]
```

## 2. Ambiente exato

| Item | Valor |
|---|---|
| Sistema | Linux 6.1.158+ x86_64, Debian GNU/Linux 12 (bookworm) |
| libc | glibc 2.36-9+deb12u14 |
| Compilador padrão | `cc` = `gcc` = **GCC 12.2.0** (Debian 12.2.0-14+deb12u1) |
| C++ | `g++` 12.2.0 |
| Frontend Clang | **Clang 20.1.2** (build `zig-bootstrap`), via **zig 0.15.1** (`zig cc`) |
| Binutils | GNU ld / ar 2.40 |
| Python (transpilador de teste, fuzz, cobertura) | 3.11.2 |
| Alvos cruzados executados | `x86-linux-musl` (i386) |
| Alvos cruzados só compilados | `aarch64-linux-gnu`, `x86_64-windows-gnu` |
| **Ausentes** | `clang`/`clang++` do LLVM, `cl.exe`, `wine`, `qemu-aarch64`, `tcc`, `icx`, `pcc`, MinGW nativo |

Não havia `clang` do LLVM na máquina e não havia como instalá-lo (o host de
release do LLVM não é alcançável daqui). O frontend Clang foi exercitado pelo
`zig cc`. Isso é registrado como **frontend Clang**, não como "clang testado" —
ver [11 §1](11-matriz-compatibilidade.md).

## 3. O que foi executado, em ordem

### 3.1 Suíte da linguagem e da biblioteca (`make test`)

307 verificações de linguagem, cada uma compilando o mesmo `.c` nos dois perfis
(M0 = macros, M1 = transpilado) e conferindo a saída; 230 verificações
unitárias dos módulos; 30 de integração (incluindo os casos de erro fatal que
precisam rodar em processo separado). Nenhuma falha.

### 3.2 Sanitizers (`make test-sanitize`)

`build-sanitize/` com `-fsanitize=address,undefined -fno-sanitize-recover=all`,
`-O1 -g`. A suíte inteira: **307 + 30**, sem UB, sem vazamento (LeakSanitizer
incluído). Antes das correções desta fase eram **129 falhas** de vazamento no
transpilador (DF-02) e uma violação de pilha no `LEIA` (DF-01) — as duas estão em
[13](13-defeitos-e-correcoes.md).

### 3.3 Matriz de compiladores (`make test-matriz`)

`tests/run_matriz.sh` roda as três suítes com **cada** compilador encontrado, em
diretório de build próprio (`build-matriz/<compilador>/`), porque o Makefile não
rastreia mudança de flags. Resultado: `cc` (GCC 12.2.0) e `/tmp/bin/clang-zig`
(Clang 20.1.2), cada um com 307 + 230 + 30, **o mesmo número** nos dois.

### 3.4 Portabilidade (`make test-portabilidade`)

O runner tem 11 seções e escreve `build/relatorio/portabilidade.tsv` com uma
linha por verificação. Resumo por seção:

| Seção | O que verifica | Resultado |
|---|---|---|
| 1 | cabeçalho e biblioteca em `-std=c11/c17/gnu11` com `-Wpedantic -Werror`, por compilador | ok (6 por compilador) |
| 2 | recusa explícita de C89 e C99 | ok |
| 3 | os 7 cabeçalhos são autocontidos (cada um compila sozinho) | ok |
| 4 | os programas de portabilidade (`precedencia`, `controle`, `entrada`, `fronteira`) compilam **e passam**, por compilador | ok |
| 5 | M0 e M1 do mesmo `.c` produzem a **mesma saída** | ok |
| 6 | saída dourada do `entrada.c` (contrato de leitura e de falha) | ok |
| 7 | link de 4 unidades de tradução com inclusão múltipla, `BG_SEM_APELIDOS`, `BG_USAR_EXTENSOES` | ok |
| 8 | auditoria de símbolos globais com `nm`: nenhum global extra das UOs de teste | ok |
| 9 | conjunto estrito de avisos sobre cabeçalho, biblioteca e transpilador | ok (11 arquivos) |
| 9b | os 24 exemplos do repositório sob `-pedantic -Werror`, por compilador (M0, ou M1 quando a fonte usa açúcar) | ok (GCC e Clang) |
| 10 | ASan + UBSan + LSan nos programas de portabilidade e no transpilador | ok |
| 11 | C++ e alvos cruzados | ok + 2 ignoradas |

As duas **ignoradas** (não "aprovadas", não "falhas"):
* `-std=c23` no frontend Clang via zig — o driver recusa o padrão;
* execução de binários Windows — não há `wine`. A compilação e a ligação
  **foram** verificadas.

Detalhes de valor que a matriz produziu:

* **`nm` limpo**: as quatro UOs do teste multi-arquivo definem só
  `bg_tu1_resultado`, `bg_tu2_resultado`, `bg_tu3_resultado`. Nenhum símbolo
  auxiliar escapa do cabeçalho, o que sustenta a promessa de que incluir
  `basicgol.h` em vários `.c` não gera colisão no linker.
* **C++**: `g++ -std=c++17` compila os cabeçalhos de módulo, liga e roda
  (`cpp: ok`); `<basicgol.h>` em C++ falha com o `#error` previsto por D-15.
* **i386**: `fronteira.c` passa com as mesmas **176 verificações** no alvo de
  32 bits, e `precedencia` passa. É a evidência de que o mapeamento de tipos não
  depende de 64 bits.
* **AArch64**: biblioteca e `fronteira.c` compilam limpos (sem execução).
* **Windows (MinGW-w64)**: `ola_mundo.exe` e `fronteira.exe` compilam e ligam
  (sem execução).

### 3.5 Fuzz (`make fuzz`)

`tests/fuzz_basicgolc.py` com seed fixa (`20261106`): 10 estratégias de mutação
(truncar, duplicar, apagar, inserir, trocar, CRLF, BOM+CRLF, aninhamento de 300
níveis, remover fechamento, no-op), contrato explícito — **não derrubar, não
travar, não vazar**. Rodado contra o build com ASan+UBSan: **2000 iterações,
0 falhas, 8,9 s**. O contrato do transpilador com entrada inválida é: emitir
diagnóstico e sair com código ≠ 0, nunca `SIGSEGV`.

### 3.6 Checkout limpo (`make test-checkout`)

Copia só o que o Git versionaria (`git ls-files` + não ignorados), roda `make`,
`make biblioteca`, `make test`, `make test-biblioteca`, `make exemplos`,
`make cobertura`, o comando M0 de uma linha do README, o fluxo M1 de duas
etapas, `make test-portabilidade` e `make fuzz`. **13 verificações, todas ok** —
incluindo a de que `make cobertura` **não altera** `tests/COBERTURA.md`.

## 4. Critérios da fase

Ver [`05 §10`](05-criterios-de-aceitacao.md). AC-52 a AC-60, todos **atendidos**,
cada um com o comando que o verifica.

## 5. Defeitos encontrados

Oito, detalhados em [13-defeitos-e-correcoes.md](13-defeitos-e-correcoes.md):

| ID | Resumo | Gravidade |
|---|---|---|
| DF-01 | `LEIA(LOGICO)`/`LEIA(TEXTO_FIXO)` gravava ponteiro no destino (stack-buffer-overflow) | **alta** |
| DF-02 | transpilador vazava toda alocação (129 falhas sob LSan) | média |
| DF-03 | cabeçalhos de módulo não compilavam em C++ (`_Noreturn`) | média |
| DF-04 | três conversões descartavam `const` (`-Wcast-qual`) | baixa |
| DF-05 | supressão de `-Wimplicit-fallthrough` escondia quedas acidentais | baixa |
| DF-06 | `make cobertura` com data: a checagem de CI falharia todo dia | média |
| DF-07 | diretórios de build fora do `.gitignore` | baixa |
| DF-08 | exemplo com protótipo antigo `()` — o Clang recusa com `-Wstrict-prototypes` | baixa |

O DF-01 é o que justifica a fase: um programa que seguia a documentação
compilava, e o binário corrompia a pilha. Não era visível em `make test` porque
o teste de `LEIA` era feito com `-Werror`, e o `-Wformat` transformava o que
"deveria" ser erro de compilação em aviso — que a linha de compilação publicada
não transforma em erro.

## 6. Cobertura

`tests/COBERTURA.md` (regenerado, determinístico): **117 nomes** do cabeçalho,
**135 casos** de teste. Os casos novos da Fase 6:

* `06_negativos/n21_leia_texto` — `LEIA(TEXTO)` é erro de compilação (DF-01).
* `06_negativos/n22_leia_texto_fixo` — `LEIA(TEXTO_FIXO(...))` idem (DF-01).
* `06_negativos/n23_tamanho_de_ponteiro` — `TAMANHO_VETOR(ponteiro)` é
  detectado por `-Wsizeof-pointer-div` nos dois compiladores (limite C-18 com
  rede de proteção).
* `03_controle/c19_escolha_fallthrough` — reescrito para provar a queda
  intencional **com** o aviso ligado (DF-05).

## 7. O que mudou no repositório

| Arquivo | Mudança |
|---|---|
| `include/basicgol.h` | `BG_SFMT` sem `default` (DF-01), com o comentário explicando a assimetria em relação a `BG_FMT` |
| `include/basicgol/consola.h` | `BG_NORETURN` (DF-03) |
| `src/biblioteca/texto.c` | conversores sem descarte de `const` (DF-04) |
| `tools/basicgolc.c` | liberação de buffer, tokens e estado (DF-02) |
| `tools/cobertura.py` | saída determinística + `--data` (DF-06) |
| `Makefile` | `test-portabilidade[-v]`, `test-sanitize`, `test-matriz`, `fuzz`, `test-checkout`, `limpar` ampliado, `COBERTURA_ARGS` |
| `tests/portabilidade/` | 8 programas novos (precedência, controle, entrada, fronteira, multi-UO ×4, C++) |
| `tests/run_portabilidade.sh`, `tests/run_matriz.sh`, `tests/run_checkout_limpo.sh`, `tests/fuzz_basicgolc.py` | ferramentas novas |
| `.github/workflows/testes.yml` | CI com matriz de compiladores/standards, sanitizers, portabilidade, cobertura e um job MSVC de avaliação |
| `.gitignore` | `build*/`, `*.o`, `*.gerado.c`, `__pycache__/` (DF-07) |
| `docs/exemplos/controle_entrada.c` | `PROCEDIMENTO mostra_media(VAZIO)` em vez de `()` (DF-08) |
| `docs/01`, `02`, `03`, `06` | `LEIA` de texto/lógico documentado como erro de compilação; `BG_SFMT` sem `default`; PT-H (`-pedantic`) corrigido com medição |

Nada disso muda a **gramática** nem o comportamento pretendido da linguagem: a
Fase 6 não acrescenta construtor. As duas únicas mudanças com efeito para quem
escreve BASICGOL são DF-01 (erro onde antes era aviso + corrupção de memória) e
DF-03 (cabeçalhos de módulo utilizáveis de C++).

## 8. O que **não** foi testado (lacunas declaradas)

| Lacuna | Por quê | Onde está registrado |
|---|---|---|
| MSVC (`cl.exe`) | não há Windows nem MSVC; job de CI existe, mas nunca rodou | [11 §4](11-matriz-compatibilidade.md) — estado "avaliado" |
| Driver `clang`/`clang-cl` do LLVM | binário ausente e inalcançável; o frontend foi testado via zig | [11 §1](11-matriz-compatibilidade.md), item 9 |
| Execução em AArch64 | sem `qemu-aarch64` | [11 §1](11-matriz-compatibilidade.md), item 4 — "compilado" |
| Execução em Windows | sem `wine` | [11 §1](11-matriz-compatibilidade.md), item 5 — "compilado" |
| `-std=c23` no frontend Clang | o driver do zig recusa o padrão | ignorada registrada no `.tsv` |
| TCC, `icx`, PCC, Digital Mars | indisponíveis | [11 §1](11-matriz-compatibilidade.md), item 10 |
| `char` sem sinal (AArch64) em execução | depende de execução em AArch64 | [11 §5](11-matriz-compatibilidade.md) |
| Locale com vírgula decimal | a suíte roda em `C`/`POSIX` | [D-17](../docs/04-restricoes-e-decisoes.md) e A-5, [11 §6](11-matriz-compatibilidade.md) |
| `CHAR_BIT != 8` | não há plataforma assim disponível | [11 §5](11-matriz-compatibilidade.md) |
| Sanitizers no MSVC | `cl` não roda aqui | [11 §4](11-matriz-compatibilidade.md) |

Nenhuma dessas lacunas foi preenchida por inferência: as células correspondentes
dizem "avaliado", "compilado" ou "não testado", nunca "ok".

## 9. Como reproduzir

```sh
make limpar
make                        # transpilador + biblioteca
make test                   # 307 + 230 + 30
make test-sanitize          # idem sob ASan+UBSan+LSan
make test-portabilidade     # 70 aprovadas, 2 ignoradas
make test-checkout          # 13 verificações, do zero
make fuzz                   # 300 mutações (2000 com --iteracoes 2000)
make cobertura              # deve sair sem diff em tests/COBERTURA.md
```

Para repetir a matriz em dois compiladores, é preciso ter um segundo compilador
visível como `clang` ou `T_CC_ALT`:

```sh
T_CC_ALT=$(command -v clang || echo /caminho/para/clang) make test-matriz
```

O `zig`, quando presente no `PATH`, habilita as verificações de alvo cruzado;
sem ele, elas aparecem como **ignoradas**.

## 10. Conclusão

A Fase 6 fecha com a suíte verde em dois frontends independentes, sem aviso no
conjunto adotado, sem UB e sem vazamento sob sanitizers, com 70 verificações de
portabilidade aprovadas e os comandos da documentação exercitados a partir de um
checkout limpo. Encontrou e corrigiu oito defeitos, um deles de gravidade alta
(corrupção de pilha em `LEIA`).

O que ela **não** faz é transformar avaliação em fato: MSVC continua "avaliado",
Windows e AArch64 continuam "compilado, não executado", e o driver `clang` do
LLVM continua "não testado". Essa é a diferença entre um relatório e um folheto
de marketing — e é o critério que o projeto adota desde a Fase 1.
