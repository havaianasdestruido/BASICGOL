# Biblioteca padrão BASICGOL (Fase 5)

A biblioteca padrão reúne utilidades pequenas para os programas BASICGOL e para C
comum. O núcleo é **C11 portátil**, divide-se por responsabilidade e usa `stdio`,
`string`, `stdlib` e `math` da plataforma em vez de duplicá-los. Os headers são
independentes; `<basicgol/biblioteca.h>` agrega os cinco módulos. `<basicgol.h>` também
os inclui e preserva os nomes de linguagem (`COPIA_TEXTO`, `LEIA_INTEIRO`, `ABS` etc.).

## 1. Módulos e arquivos

| Módulo | Header público | Implementação | Escopo |
|---|---|---|---|
| Console | `basicgol/consola.h` | `src/biblioteca/consola.c` | leitura de linhas, leitura numérica por linha, escrita literal e diagnóstico |
| Matemática | `basicgol/matematica.h` | `src/biblioteca/matematica.c` | divisão/resto verificados e primitivas de `ABS`, `MINIMO`, `MAXIMO` e `ARREDONDA` |
| Texto | `basicgol/texto.h` | `src/biblioteca/texto.c` | operações limitadas sobre strings C e conversões numéricas |
| Caractere | `basicgol/caractere.h` | `src/biblioteca/caractere.c` | classificação e mudança de caixa ASCII |
| Utilidades | `basicgol/utilidades.h` | `src/biblioteca/utilidades.c` | semente e sorteio com `rand()` |
| Agregado | `basicgol/biblioteca.h` | — | inclui os cinco headers portáteis e informa a versão 1.0.0 |

Inclua somente o módulo necessário ou use o agregado:

```c
#include <basicgol/biblioteca.h>
```

A interface de palavras-chave `#include <basicgol.h>` reexporta esses módulos para
compatibilidade. Ao chamar helpers, ligue a biblioteca compilada; não é header-only.

## 2. Convenções da API

- **Nomes:** funções C usam o prefixo `bg_` e palavras em português; macros de
  linguagem continuam em maiúsculas (`TEXTO_PARA_INTEIRO`, `ESCREVAL`).
- **Sucesso e erro recuperável:** funções que podem falhar devolvem `bool`: `true`
  significa sucesso e `false`, argumento inválido, entrada inválida, truncamento,
  falha de conversão ou erro de E/S. Não há código global de erro nem encerramento
  silencioso.
- **Saídas:** em falha, ponteiros de saída numéricos permanecem inalterados. Uma
  função que escreve em string deixa `destino[0] == '\0'` em falha sempre que
  destino e capacidade permitem escrever esse terminador.
- **Memória:** o chamador aloca e libera os buffers. A biblioteca não retém ponteiros
  nem transfere propriedade. `bg_junta_texto` usa temporário interno para suportar
  sobreposição; esse armazenamento é liberado antes de retornar.
- **Strings:** são strings C terminadas em NUL; todos os limites e posições contam
  **bytes**, não caracteres Unicode. A capacidade inclui o byte NUL. O chamador deve
  passar ponteiros para objetos válidos e, quando aplicável, buffers realmente tão
  grandes quanto a capacidade informada.
- **Fatal, por compatibilidade da linguagem:** `bg_erro` imprime o diagnóstico e
  encerra com status 1. `DIVIDE`, `RESTO` e `ABS` usam esse caminho quando a operação
  não é definida ou o resultado não cabe (por exemplo, divisor zero; em representações
  usuais, `INT_MIN / -1` e `ABS(INT_MIN)`). Para novas funções, prefira retorno `bool`.
- **Locale e concorrência:** os helpers de caractere são sempre ASCII. Conversão de
  real acompanha a locale C corrente (`strtod`); BASICGOL não chama `setlocale`.
  `rand()` é o gerador global do C e não fornece garantia de segurança entre threads.

## 3. API por módulo

### 3.1 Console — `basicgol/consola.h`

| Função | Parâmetros | Retorno e efeito |
|---|---|---|
| `bg_leia_linha(entrada, destino, capacidade)` | `entrada`: `FILE *` aberto para leitura; `destino`: buffer; `capacidade`: bytes totais, incluindo NUL | `true` para uma linha completa. Remove LF, CRLF ou CR. Linha vazia é válida. EOF sem bytes, NUL embutido, erro e linha longa devolvem `false`; linha longa é drenada e o destino fica vazio. |
| `bg_leia_texto(destino, capacidade)` | buffer e capacidade; usa `stdin` | `true`/`false` como acima. Compatibilidade com `LEIA_TEXTO`: consome um LF ou CRLF pendente de uma leitura formatada. Para preservar uma linha vazia, use `bg_leia_linha(stdin, ...)`. |
| `bg_leia_inteiro_de(entrada, saida)` | fluxo e ponteiro para `int` | Lê uma linha inteira e converte decimal; `false` em EOF, linha longa, texto inválido ou overflow. Em falha `*saida` não muda. |
| `bg_leia_real_de(entrada, saida)` | fluxo e ponteiro para `double` | Lê uma linha e converte via `strtod`; mesmos critérios de falha, incluindo overflow/underflow sinalizado por `ERANGE`. |
| `bg_leia_inteiro(saida)` | ponteiro para `int`; usa `stdin` | Igual a `bg_leia_inteiro_de`, com compatibilidade de descarte do terminador pendente. |
| `bg_leia_real(saida)` | ponteiro para `double`; usa `stdin` | Igual a `bg_leia_real_de`, com compatibilidade de descarte do terminador pendente. |
| `bg_escreva_texto(saida, texto)` | `FILE *` de saída e texto literal não nulo | `true` se `fputs` concluir; não interpreta `%` como formato e não adiciona quebra de linha. |
| `bg_escreva_linha(saida, texto)` | fluxo de saída e texto literal não nulo | Escreve o texto e LF; `true` se ambas as escritas concluírem. Pode haver saída parcial se o fluxo falhar no meio. |
| `bg_erro(arquivo, linha, mensagem)` | origem, linha e mensagem | Não retorna: escreve `arquivo:linha: erro: mensagem` em `stderr` e encerra com status 1. `NULL` vira `?`/texto vazio. |
| `bg_passo_zero(arquivo, linha)` | origem e linha | Não retorna: emite diagnóstico de passo zero via `bg_erro`. O retorno declarado `int` existe somente para uso em expressão C. |

`bg_leia_linha` consome um terminador por chamada. Em EOF, devolve `false` se não
havia bytes; se havia uma última linha sem terminador, essa linha é sucesso. A função
não fecha nem toma posse do `FILE *`.

### 3.2 Matemática — `basicgol/matematica.h`

As funções `bg_divide_i/l/ll/u/ul/ull/f/d/ld(a, b, arquivo, linha)` recebem dois
operandos do sufixo indicado (`i` = `int`, `l` = `long`, `ll` = `long long`, `u/ul/ull`
sem sinal, `f/d/ld` real). Devolvem o quociente no mesmo tipo. Zero como divisor é
fatal; para inteiros com sinal, também é fatal se o quociente não couber (em geral,
`MIN / -1`). Os parâmetros `arquivo` e `linha` identificam o ponto de uso; os macros
`DIVIDE` e `BG_DIVIDE` os preenchem automaticamente.

As funções `bg_resto_i/l/ll/u/ul/ull(a, b, arquivo, linha)` têm a mesma convenção de
parâmetros e retorno, para resto inteiro. Divisor zero e quociente não representável
(geralmente `MIN % -1`) são fatais. Para capturar esses casos no programa, use
`RESTO`/`BG_RESTO`; `/` e `MOD` continuam sendo operadores C não verificados.

| Função | Parâmetros | Retorno |
|---|---|---|
| `bg_abs_i`, `bg_abs_l`, `bg_abs_ll` | valor `int`, `long` ou `long long` | valor absoluto no mesmo tipo; é fatal quando a magnitude excede o máximo positivo (em geral no mínimo assinado). |
| `bg_abs_f`, `bg_abs_d` | `float` ou `double` | valor absoluto do mesmo tipo, por `fabsf`/`fabs`; NaN segue a semântica da libm. |
| `bg_min_i`, `bg_min_l`, `bg_min_d` | dois operandos do tipo indicado | menor dos dois, no mesmo tipo; o caso `double` usa `fmin`. |
| `bg_max_i`, `bg_max_l`, `bg_max_d` | dois operandos do tipo indicado | maior dos dois, no mesmo tipo; o caso `double` usa `fmax`. |
| `bg_round_l(valor)` | `double` finito cujo arredondamento cabe em `long long` | `long long` mais próximo; empates arredondam para longe de zero (`llround`). Fora do domínio documentado, vale a condição de domínio de `llround`. |

As macros didáticas `POTENCIA`, `RAIZ`, `PISO` e `TETO` usam, respectivamente,
`pow`, `sqrt`, `floor` e `ceil` de `<math.h>`; os resultados e domínios são os da
biblioteca matemática C. `ARREDONDA` usa `llround` e devolve `long long`. O programa
que usa essas funções deve ligar `-lm` quando a plataforma mantém a libm separada.

### 3.3 Texto — `basicgol/texto.h`

| Função | Parâmetros | Retorno |
|---|---|---|
| `bg_tamanho_texto(texto)` | string C ou `NULL` | comprimento em bytes (`strlen`); `NULL` equivale a vazio. |
| `bg_copia_texto(destino, capacidade, origem)` | buffer, capacidade incluindo NUL, string de origem | `true` se couber inteira; `NULL` como origem vazia. Em falha limpa destino quando utilizável. Suporta origem/destino sobrepostos. |
| `bg_junta_texto(destino, capacidade, primeiro, segundo)` | buffer, capacidade e duas strings | `true` se a concatenação couber; `NULL` equivale a vazio. Suporta sobreposição entre origens e destino; falha por falta de espaço ou alocação devolve `false` e limpa destino. |
| `bg_acrescenta_texto(destino, capacidade, origem)` | buffer já terminado em NUL, capacidade real e origem | acrescenta a origem inteira; aceita autoacréscimo. `false` se inválido/não couber, limpando destino quando possível. |
| `bg_trecho_texto(destino, capacidade, origem, inicio, quantidade)` | buffer, origem, índice inicial e número máximo de bytes | copia no máximo a quantidade disponível. Início além do fim produz vazio com `true`; falta de espaço devolve `false`. |
| `bg_compara_texto(primeiro, segundo)` | duas strings; `NULL` equivale a `""` | comparação lexicográfica normalizada: `-1`, `0` ou `1`. |
| `bg_texto_vazio(texto)` | string ou `NULL` | `true` se vazio ou nulo. |
| `bg_formata_texto(destino, capacidade, formato, ...)` | buffer, capacidade e formato de `printf` com argumentos correspondentes | usa `vsnprintf`; `true` se a saída inteira couber. Em truncamento, formato nulo ou falha, limpa destino e devolve `false`. O compilador C não verifica a correspondência do formato. |
| `bg_texto_para_inteiro(texto, saida)` | string decimal e `int *` | aceita o número inteiro completo, com espaços ASCII nas bordas; recusa sobra e overflow. `false` não altera `*saida`. |
| `bg_texto_para_natural(texto, saida)` | string decimal não negativa e `unsigned *` | aceita `+` opcional; recusa `-`, sobra e overflow. `false` não altera `*saida`. |
| `bg_texto_para_real(texto, saida)` | string e `double *` | conversão de `strtod` na locale atual; exige consumo integral (espaços ASCII finais são tolerados) e recusa `ERANGE`. `false` não altera `*saida`. |
| `bg_numero_para_inteiro(valor, saida)` | `double` e `int *` | trunca em direção a zero; NaN e valores fora da faixa de `int` falham sem alterar `*saida`. |

As conversões textuais preservam `errno`; o resultado de erro é o booleano, não um
código global. `bg_formata_texto` segue as regras de `printf`: passar argumentos com
tipo incompatível com o formato é erro do chamador. Para escrita literal em console,
use `bg_escreva_texto`, não a função de formatação.

### 3.4 Caractere — `basicgol/caractere.h`

| Função | Parâmetro | Retorno |
|---|---|---|
| `bg_e_digito(c)` | byte em `char` | `true` somente para ASCII `'0'` a `'9'`. |
| `bg_e_letra(c)` | byte em `char` | `true` somente para ASCII `A-Z` ou `a-z`. |
| `bg_caractere_maiusculo(c)` | byte em `char` | converte ASCII `a-z` para `A-Z`; todos os demais bytes não mudam. |
| `bg_caractere_minusculo(c)` | byte em `char` | converte ASCII `A-Z` para `a-z`; todos os demais bytes não mudam. |

Não é uma API Unicode: bytes UTF-8 fora de ASCII passam inalterados. O contrato é
independente de locale e não sofre as restrições de argumento negativo de `<ctype.h>`.

### 3.5 Utilidades e aleatoriedade — `basicgol/utilidades.h`

| Função | Parâmetros | Retorno |
|---|---|---|
| `bg_inicia_sorteio(semente)` | semente `unsigned` | não devolve valor; chama `srand`. Não há semente automática. |
| `bg_sorteia_ate(limite_exclusivo, saida)` | limite positivo e ponteiro `unsigned *` | `true` e um valor em `[0, limite)`; `false` para limite zero, ponteiro nulo ou limite maior que `RAND_MAX + 1`. Falha não altera `*saida`. |
| `bg_sorteia(n)` | limite legado `int` | valor em `[0,n)` para limite suportado; devolve `0` para `n <= 0` ou fora do alcance. Prefira a variante verificável `bg_sorteia_ate`. |

O gerador de `rand()` é previsível, global ao processo e de qualidade definida pela
implementação; **não** é criptográfico, não é adequado para tokens/segurança e não
promete independência entre threads. A rejeição em `bg_sorteia_ate` remove o viés de
módulo supondo amostras uniformes, mas não pode melhorar a distribuição fornecida pela
implementação de C. Repetir a mesma semente reproduz a sequência somente na mesma
implementação C.

## 4. Nomes da linguagem e compatibilidade

`<basicgol.h>` mantém os helpers didáticos e os aliases `BG_*`/nomes curtos descritos
em [02 — Referência](02-referencia.md). As operações de string e caractere encaminham
para os módulos correspondentes. `ESCREVA_TEXTO` e `ESCREVA_ERRO` escrevem texto
literal e agora devolvem o booleano da escrita. `LEIA_TEXTO`, `LEIA_INTEIRO` e
`LEIA_REAL` continuam usando `stdin`. Os formatos `ESCREVA_FMT` e `LEIA` continuam
sendo os da biblioteca C.

O único módulo de compilador opcional continua sendo `BG_USAR_EXTENSOES` em
`basicgol.h` (GCC/Clang); ele é independente da biblioteca padrão. **Não há helpers
específicos de plataforma nesta versão.** Por isso não há dependência de POSIX,
Windows, `unistd.h` ou `windows.h`, nem headers de plataforma no agregado. Uma futura
extensão de plataforma deve ter header e implementação separados, não incluídos pelo
core portátil.

## 5. Compilação e dependências

### Compilar com o Makefile do projeto

```sh
make                         # build/basicgolc e build/libbasicgol.a
make biblioteca              # somente a biblioteca estática
make test                    # testes da linguagem e da biblioteca
make test-biblioteca         # somente os testes unitários dos módulos
make exemplos                # compila os exemplos
```

### Ligar um programa

```sh
make biblioteca
cc -std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror \
   -I include programa.c build/libbasicgol.a -lm -o programa
./programa
```

A ordem importa em toolchains Unix: objetos/fontes primeiro, archive depois, `-lm` por
último. Se a plataforma já incorpora libm, omita `-lm`. Também é possível compilar as
implementações diretamente, sem criar o archive:

```sh
cc -std=c11 -Wall -Wextra -I include programa.c \
   src/biblioteca/*.c -lm -o programa
```

Para o caminho M1, gere o C e ligue a mesma biblioteca:

```sh
make
build/basicgolc programa.c -o programa.gerado.c
cc -std=c11 -Wall -Wextra -I include programa.gerado.c \
   build/libbasicgol.a -lm -o programa
```

### Inventário mínimo

| Item | Uso |
|---|---|
| Compilador C11 hospedado (`cc`, GCC, Clang ou compatível) | compilação do core e do programa |
| Biblioteca padrão C (libc) | `stdio`, `stdlib`, `string`, conversões, alocação e I/O |
| Biblioteca matemática C (libm, quando separada) | `pow`, `sqrt`, `round`/`llround`, `floor`, `ceil`, `fabs`, `fmin`, `fmax` |
| `make` e `ar` | build e archive estático; não necessários para compilar diretamente os fontes |
| Bash, `mktemp`, `diff` e Python 3 | runner de testes e geração da matriz de cobertura |
| Dependências de terceiros | nenhuma |

## 6. Demonstrações e testes

Os exemplos `docs/exemplos/biblioteca_{consola,matematica,texto,caractere,utilidades}.c`
demonstram um programa pequeno por módulo. Eles são compilados e executados com
entradas/saídas douradas pela suíte regular.

`tests/test_biblioteca.c` exercita a API diretamente: entradas vazias e longas,
terminadores LF/CRLF, EOF, NUL embutido, limites e sobreposição de buffers, conversões
válidas/inválidas/fora de faixa, ASCII, limites matemáticos, sequência aleatória,
entradas inválidas e diagnósticos fatais. `make test-biblioteca` executa os 230 checks
unitários e os casos de integração em processos separados para as rotinas fatais.

Consulte [`tests/README.md`](../tests/README.md) para a suíte completa. A licença é
Apache-2.0, igual à do restante do repositório.
