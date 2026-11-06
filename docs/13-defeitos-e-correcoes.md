# 13 — Defeitos encontrados na Fase 6, e o que foi feito

> **O que entra nesta lista.** Só entra aqui o que a Fase 6 **provou** com um
> comando: um programa que compilava e não devia, um aviso que apareceu no
> conjunto adotado, um erro que aparecia só no segundo compilador, um sanitizer
> que acusou. Comportamento documentado como intencional **não** é defeito e
> está na §3, separado, para ninguém "consertar" depois achando que é bug.

> **Prefixos de ID.** `DF-nn` = defeito do **produto** (cabeçalho, biblioteca,
> transpilador); `FT-nn` = defeito da **ferramenta de teste**. Os dois são novos
> nesta fase. `D-nn` continua reservado às **decisões de projeto** de
> [04-restricoes-e-decisoes.md](04-restricoes-e-decisoes.md) — não confunda.

Resumo:

| ID | Defeito | Gravidade | Como apareceu | Arquivo tocado | Fixado por |
|---|---|---|---|---|---|
| DF-01 | `LEIA(LOGICO)` e `LEIA(TEXTO_FIXO(...))` gravavam ponteiro no destino: 8 bytes num objeto de 1 (stack-buffer-overflow) | **alta** — corrupção de memória silenciosa | ASan, ao ler `LEIA(LOGICO)` | `include/basicgol.h` | `n16`, `n21`, `n22`, `06_negativos/*` |
| DF-02 | Transpilador vazava toda alocação (LeakSanitizer acusava em toda execução) | média — inutilizava o sanitizer como sinal | `make test-sanitize`: 129 falhas de uma vez | `tools/basicgolc.c` | `make test-sanitize` |
| DF-03 | `include/basicgol/*.h` não compilavam em C++: `_Noreturn` não existe em C++ | média — quebrava o contrato do `extern "C"` | `g++ -std=c++17 -I include` | `include/basicgol/consola.h` | `tests/portabilidade/cpp_inclusao.cpp` |
| DF-04 | Três conversões que descartavam `const` em `texto.c` (`-Wcast-qual`) | baixa — era seguro, mas era aviso real | varredura de avisos com `-Wcast-qual -Werror` | `src/biblioteca/texto.c` | `make test-portabilidade` §3 |
| DF-05 | Caso de queda intencional desligava `-Wimplicit-fallthrough` por arquivo | baixa — escondia também as quedas acidentais | revisão do `cflags.txt` do caso | `tests/casos/03_controle/c19_.../` | `make test` |
| DF-06 | `make cobertura` gravava a data no arquivo gerado: a checagem de CI falharia todo dia seguinte ao commit | média — o portão de CI era inoperante | `make test-checkout`, comparando antes/depois | `tools/cobertura.py` | `make test-checkout` |
| DF-07 | `build-matriz/` e `build-sanitize/` não estavam no `.gitignore` | baixa — 200+ arquivos entrariam no commit | `git status` | `.gitignore` | `git status` limpo |
| DF-08 | Exemplo declarava procedimento sem parâmetros como `()`, protótipo antigo que o Clang recusa com `-Wstrict-prototypes` | baixa — o código de exemplo não era portátil | `-pedantic -Werror` nos 24 exemplos, por compilador | `docs/exemplos/controle_entrada.c` | `make test-portabilidade` §3 |

---

## DF-01 — `LEIA` com tipo sem formato escrevia um ponteiro no destino

**Gravidade: alta.** É o defeito mais sério da fase.

**Sintoma.** Sob ASan:

```
ERROR: AddressSanitizer: stack-buffer-overflow on address 0x...
WRITE of size 8 at 0x... thread T0
    #0 scanf
    #1 bg_leia_logico
```

**Reprodução.**

```c
#include <basicgol.h>
INICIO
    LOGICO pronto;
    LEIA(pronto);
FIM
```

```sh
cc -std=c11 -fsanitize=address -I include programa.c build/libbasicgol.a -lm -o p && ./p
```

**Causa.** `BG_SFMT` — o `_Generic` que escolhe o formato de `scanf` — terminava
em `default: "%p"`. Para `LOGICO` (que é `bool`, sem associação própria), o
`scanf("%p", &pronto)` escreve **um ponteiro** (8 bytes) num objeto de 1 byte.
Para `LEIA(TEXTO_FIXO(...))` o efeito era o mesmo tipo de coisa: gravava um
endereço dentro do buffer em vez do texto.

O que barrava o problema até aqui era só o `-Wformat`, que reclama de `%p` com
`bool *`. Mas `-Wformat` é **aviso**, e o comando de compilação publicado no
README não usa `-Werror`. Ou seja: quem seguisse a documentação compilava, e o
binário corrompia a pilha.

**Correção.** Remover o `default` de `BG_SFMT`. Em C11, um `_Generic` sem
`default` cujo tipo de controle não casa com nenhuma associação é **violação de
restrição** (6.5.1.1p3) — erro de compilação, com ou sem `-Werror`, nos dois
compiladores:

```
GCC:   error: '_Generic' selector of type '_Bool *' is not compatible with any association
Clang: error: controlling expression type 'bool *' not compatible with any generic
       association type
```

`BG_FMT` (o de **saída**) mantém `default: "%p"`. Imprimir um ponteiro é útil e
não escreve na memória do programa; a assimetria é proposital e está comentada
no cabeçalho.

**Efeito colateral tratado.** `LEIA(TEXTO_FIXO(...))` e `LEIA(TEXTO)` passaram a
ser erro de compilação. Antes eles "compilavam" e faziam a coisa errada. Os
casos `n21_leia_texto` e `n22_leia_texto_fixo` fixam a mensagem, e a referência
([02](02-referencia.md)) diz, na linha do `LEIA`, que texto se lê com
`LEIA_TEXTO`/`LEIA_TEXTO_FIXO`.

---

## DF-02 — Vazamentos no transpilador

**Gravidade: média.** Não afeta o programa gerado; afeta a capacidade de usar
sanitizers como sinal — e é exatamente o tipo de coisa que a Fase 6 existe para
tirar do caminho.

**Sintoma.** `make test-sanitize` com LeakSanitizer: **129 falhas** de uma vez,
todas no `basicgolc`, todas do mesmo formato:

```
Direct leak of 4096 byte(s) in 1 object(s) allocated from:
    #0 realloc
    #1 buf_empurra tools/basicgolc.c:...
```

**Causa.** O transpilador aloca o buffer de saída, o vetor de tokens (e o texto
de cada token), a lista de nomes já avisados e a lista de enumeradores; nada
disso era liberado. Sendo um processo curto, o sistema operacional recupera a
memória no `exit` — mas o LSan (que roda antes do `exit`) acusa, e a suíte
inteira fica vermelha por um motivo que não é semântico.

**Correção.** `buf_libera`, `lista_libera` e `estado_libera` em
`tools/basicgolc.c`, chamadas nos dois caminhos de saída (sucesso e erro). O
critério não foi "sumir com o aviso", foi **fechar o ciclo de vida**: cada
`malloc`/`realloc` do arquivo tem um `free` correspondente, e o comentário no
código diz por quê.

**Verificação.** `make test-sanitize` fecha com as 307 verificações de linguagem
e as 30 de integração, e o `basicgolc` roda limpo sob ASan+UBSan+LSan em
`tests/run_portabilidade.sh`.

---

## DF-03 — Os cabeçalhos de módulo não compilavam em C++

**Gravidade: média.** O cabeçalho promete `extern "C"`; a promessa estava
quebrada.

**Sintoma.**

```
$ g++ -std=c++17 -I include -c tests/portabilidade/cpp_inclusao.cpp
include/basicgol/consola.h:29:1: error: '_Noreturn' does not name a type
```

**Causa.** `bg_erro` era declarada `_Noreturn`, que é palavra-chave de **C11** e
não existe em C++. `extern "C"` resolve ligação, não sintaxe: o C++ ainda
precisa *ler* o cabeçalho.

**Correção.** Macro `BG_NORETURN` em `include/basicgol/consola.h`:

| Situação | Expansão |
|---|---|
| C++ (`__cplusplus`) | `[[noreturn]]` |
| C11+ (`__STDC_VERSION__ >= 201112L`) | `_Noreturn` |
| MSVC ≥ 1928 (16.8) | `_Noreturn` |
| nada disso | vazio (compilador antigo compila; só perde a dica de otimização) |

**Verificação.** `tests/portabilidade/cpp_inclusao.cpp` compila com
`g++ -std=c++17 -Wall -Wextra -Werror`, liga com `build/libbasicgol.a`, chama
`bg_e_digito`, `bg_copia_texto`, `bg_divide_i`, `bg_texto_para_real`,
`bg_sorteia_ate` e `bg_tamanho_texto`, e imprime `cpp: ok`.

E o outro lado do contrato, também verificado: `#include <basicgol.h>` em C++
**tem** de falhar, com a mensagem da decisão D-15 (`BASICGOL exige C11 ou
superior`). O runner confere o texto, não só o código de saída — se alguém
"consertar" o cabeçalho para aceitar C++, esse teste falha.

---

## DF-04 — Conversões que descartavam `const` em `texto.c`

**Gravidade: baixa.** O código estava correto; o aviso era legítimo.

**Sintoma.** No conjunto adotado (`-Wcast-qual -Werror`), três erros:

```
src/biblioteca/texto.c:198:35: error: cast discards 'const' qualifier from pointer target type [-Werror=cast-qual]
```

**Causa.** `fim = (char *) bg_pula_espaco_ascii_final(fim);` — a função devolve
`const char *` e o resultado voltava para a variável `char *fim` do `strtol`.

**Correção.** Uma variável nova, honesta:

```c
const char *sobra = bg_pula_espaco_ascii_final(fim);
if (*sobra != '\0' || ...) { return false; }
```

Sem cast, sem aviso, e o tipo passa a dizer a verdade (a sobra é só lida). Feito
nos três conversores (`inteiro`, `natural`, `real`).

Foi **correção**, não supressão: as opções "ligar `-Wno-cast-qual`" e "adicionar
`-Wno-cast-qual` só neste arquivo" foram descartadas por contrariarem a regra do
projeto (§2 do [README](../README.md)).

---

## DF-05 — A supressão que escondia quedas acidentais

**Gravidade: baixa.** Era dívida de teste, não bug de produto.

**Sintoma.** `tests/casos/03_controle/c19_escolha_fallthrough/cflags.txt`
continha `-Wno-implicit-fallthrough`. O caso queria demonstrar que a queda
intencional de um `CASO` para o seguinte funciona — e para isso **desligava o
aviso para o arquivo inteiro**. Qualquer queda acidental no mesmo arquivo
passaria despercebida, e o caso deixava de provar o que dizia provar.

**Causa.** O jeito canônico (`/* fall through */`) **não funciona** aqui: `CASO`
é uma macro, o GCC perde o comentário na expansão e segue avisando. A primeira
tentativa da Fase 6 foi, portanto, "desligar o aviso". A segunda foi testar a
alternativa certa.

**Correção.** `__attribute__((fallthrough))` no ponto exato, com `((void) 0)`
como alternativa para compiladores sem o atributo:

```c
#if defined(__GNUC__) && (__GNUC__ >= 7)
#  define cai_para_proximo __attribute__((fallthrough))
#else
#  define cai_para_proximo ((void) 0)
#endif
...
        CASO 1:
            ESCREVA("um ");
            cai_para_proximo;
```

Verificado nos dois compiladores com `-Wall -Wextra -Werror`: compila limpo e
imprime `dois tres` tanto no caminho M0 quanto no M1. O `cflags.txt` foi
apagado.

> Nota de projeto: isto é anotação **de C**, no arquivo de teste, não no
> cabeçalho. A linguagem não ganhou construtor novo (a Fase 6 não muda a
> gramática). Se um dia fizer sentido, `CAI_PARA` é candidato natural a
> construtor do perfil N — hoje não existe, e a documentação não finge que
> existe.

---

## DF-06 — `make cobertura` não era determinístico

**Gravidade: média** — não afeta o produto, mas desliga um portão de CI.

**Sintoma.** O workflow de CI compara o arquivo gerado com o versionado:

```sh
make cobertura && git diff --exit-code tests/COBERTURA.md
```

Isso falharia **no dia seguinte a qualquer commit**, porque o gerador escrevia
`> Gerado em {date.today()}` no cabeçalho. Um portão que sempre falha é um
portão que todo mundo aprende a ignorar.

**Correção.** A saída padrão não tem data (é determinística); a versão datada
passou a ser opt-in:

```sh
make cobertura                     # determinística, é a que o CI compara
make cobertura COBERTURA_ARGS=--data  # relatório datado, para anexar
```

**Verificação.** `tests/run_checkout_limpo.sh` copia `tests/COBERTURA.md`,
roda `make cobertura` e compara com `cmp` — a checagem do CI, sem depender de
`git`. É uma das 13 verificações de `make test-checkout`.

---

## DF-08 — `PROCEDIMENTO x()` é protótipo antigo

**Gravidade: baixa.** Não afeta a linguagem; afeta o código de exemplo — que é o
que o aluno copia.

**Sintoma.** No frontend Clang, com o conjunto adotado:

```
docs/exemplos/controle_entrada.c:11:26: error: a function declaration without a
prototype is deprecated in all versions of C [-Werror,-Wstrict-prototypes]
   11 | PROCEDIMENTO mostra_media() {
```

No GCC, silêncio: `-Wstrict-prototypes` não é ligado por `-pedantic` lá.

**Causa.** Em C, `f()` significa "parâmetros **não especificados**" e `f(void)`
significa "sem parâmetros". O primeiro é protótipo antigo, obsoleto no C23. O
código portátil é o que **os dois** compiladores aceitam.

**Correção.** `PROCEDIMENTO mostra_media(VAZIO)`, com a nota no próprio exemplo.
A regra vale para `FUNCAO` também: sem parâmetros, escreva `(VAZIO)`.

**Verificação.** `tests/run_portabilidade.sh` §3 compila os **24** exemplos com
`-pedantic -Wall -Wextra -Wformat=2 -Wvla -Werror`, com cada compilador
(mandando para o caminho M1 os que usam açúcar), e exige zero avisos. Antes da
correção o resultado era 23 de 24 no Clang.

---

## DF-07 — Diretórios de build fora do `.gitignore`

**Gravidade: baixa.** `build-matriz/` e `build-sanitize/` (introduzidos nesta
fase) apareciam como `??` no `git status` e teriam entrado no commit com centenas
de objetos. `.gitignore` passou a cobrir `build/`, `build-*/`, `*.o`,
`*.gerado.c` e `__pycache__/`.

---

## 3. O que **não** é defeito (e não deve ser "consertado")

| Comportamento | Por que é intencional |
|---|---|
| `LEIA(LOGICO)` erra na compilação | Ler lógico é ambíguo (0/1, FALSO/VERDADEIRO, V/N). A linguagem prefere recusar a adivinhar. Fixado por `n16`, `n21`, `n22`. |
| `PARA i DE 1 ATE MAXIMO_INTEIRO` estoura no último `++i` | Consequência da tradução honesta `i <= fim` + `i++`. Documentado em [01](01-especificacao.md) como limite, com teste que demonstra o comportamento real. |
| `TAMANHO_VETOR(ponteiro)` compila e devolve 2 (ou 1) | É `sizeof` de C, não uma função da linguagem. O que existe é a rede: `-Wsizeof-pointer-div`, verificado nos dois compiladores e fixado por `n23`. |
| `DIVIDE(a, 0.0)` aborta | Divisão por zero real é erro fatal documentado; comparar com `0.0` é o contrato. Ligar `-Wfloat-equal` produziria aviso sobre código correto. |
| `ESCREVA(TEXTO NULL)` imprime `(null)` no glibc | Comportamento da libc. `ESCREVA_TEXTO` recusa `NULL` e devolve `FALSO`. No UCRT pode abortar — está na matriz ([11 §4](11-matriz-compatibilidade.md)). |
| `BG_USAR_EXTENSOES` recusa compilar em MSVC | D-22: melhor `#error` com explicação do que extensão emulada pela metade. |
| `AFIRME` não serve como expressão | `AFIRME(c, msg)` é `do { ... } while (0)`, `void`. Não é operador, é instrução. |
| Avisos de `-Wparentheses`/`-Wlogical-not-parentheses` em `a OU b E c` | É o C avisando sobre mistura de `&&`/`||`; a linguagem mantém a precedência de C de propósito. |

---

## 4. Defeitos nas **ferramentas de teste** (não no produto)

Ficam registrados porque custaram tempo e porque o padrão se repete: o teste
estava errado, não o código.

| # | Sintoma | Causa | Correção |
|---|---|---|---|
| FT-01 | O mesmo compilador contava como dois na matriz | `cc` e `gcc` são o mesmo binário com nomes diferentes | dedupe por `readlink -f` |
| FT-02 | A sondagem de `-std=c23` "passava" sem compilar | o probe mandava a saída de pré-processamento para o `/dev/null` e o erro se perdia | `-fsyntax-only` |
| FT-03 | O `basicgolc` "falhava" na checagem de sanitizer | o teste tratava qualquer `stderr` como falha, e o transpilador emite avisos W-02 legítimos | compilar com `-w` e procurar `sanitizer`/`runtime error` no texto |
| FT-04 | `precedencia` "falhava" no i386 | comparava a saída por string; no alvo cruzado a saída não é confiável | comparar o código de saída do programa |
| FT-05 | `run_matriz.sh` morria antes de rodar qualquer teste quando o diretório de relatório não existia | o redirecionamento do log acontecia antes do `mkdir -p` | `mkdir -p "$BASE"` antes de qualquer redirecionamento |
