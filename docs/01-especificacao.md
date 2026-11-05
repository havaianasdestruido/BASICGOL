# BASICGOL — Especificação da Linguagem

| Campo | Valor |
|---|---|
| Nome | BASICGOL |
| Dialeto / abreviatura | **BG** |
| Versão da especificação | `0.1` (Fase 1 — pré-1.0) |
| Modelo de execução | C (nenhuma máquina virtual, nenhum interpretador) |
| Dialeto de origem | C11 (obrigatório), C23 (aceito) |
| Extensão de arquivo | `.c` (UTF-8) |
| Cabeçalho de linguagem | `<basicgol.h>` (entrega da Fase 2) |
| Licença | Apache-2.0 |

Documentos irmãos: [02 — Referência](02-referencia.md), [03 — Mapeamento para C](03-mapeamento-para-c.md),
[04 — Restrições e decisões](04-restricoes-e-decisoes.md), [05 — Critérios de aceitação](05-criterios-de-aceitacao.md).

---

## 1. Identidade e objetivos

### 1.1 Problema

Aprender a programar em português é mais fácil com `SE`/`ENTAO` do que com
`if`/`{`. Mas linguagens didáticas em português (VisuAlg, Portugol Studio) costumam ser
**ilhas**: interpretador próprio, biblioteca própria, nenhuma ponte para o mundo real.
Quem aprende nelas recomeça do zero ao migrar para C.

BASICGOL ataca o problema pelo outro lado: **a linguagem é uma camada de vocabulário
sobre C**, não um ambiente separado. O aluno escreve `SE`, o compilador é o `cc`, e a
biblioteca é a biblioteca C.

### 1.2 Objetivos (mensuráveis)

| ID | Objetivo | Como se verifica |
|---|---|---|
| O-1 | Um programa BG é compilável por um compilador C11 comum, sem ferramenta própria | `cc -std=c11 prog.c` funciona no caminho M0 (a fonte já é `.c`) |
| O-2 | Nenhum construtor especificado depende de transformação impossível ao pré-processador | todo construtor tem linha na tabela de mapeamento com expansão verificada |
| O-3 | Vocabulário em português para 100% do fluxo de controle, tipos, E/S básica e declarações | nenhum `if`/`while`/`for` obrigatório no material didático |
| O-4 | Interoperabilidade total com C: qualquer código C é aceito dentro do programa | exemplo `interop_c.c` mistura BG e C no mesmo arquivo |
| O-5 | Mensagens de erro apontam para a linha do `.c` | compiladores mostram `arquivo:linha` do fonte original |
| O-6 | Um aluno que sabe BG consegue ler C com pouco esforço | tabela de mapeamento é 1:1 e ensinável em uma página |

### 1.3 Não-objetivos

* Ser mais expressiva que C (não há genéricos, classes, exceções, lambdas, módulos).
* Esconder ponteiros e memória (o aluno vai encontrá-los; a especificação os expõe).
* Ser independente de plataforma em nível de SO (portabilidade = a de C11).
* Substituir o ensino de C (BG é uma ponte, não um destino).
* Oferecer sintaxe “livre” (sem parênteses, sem ponto e vírgula, indentação
  significativa): ver [D-04](04-restricoes-e-decisoes.md).

### 1.4 Público

1. **Iniciantes** em português: precisam de `ESCREVAL`, `SE`, `ENQUANTO`, `VETOR`.
2. **Professores**: precisam de um dialeto estável, previsível e mapeável para C.
3. **Programadores C** que querem um dialeto de leitura rápida para protótipos e
   exercícios, sem perder acesso à linguagem hospedeira.

---

## 2. Escopo

### 2.1 Dentro do escopo (v0.1 → v1.0)

* Programa completo com função de entrada (`INICIO`/`FIM`).
* Funções e procedimentos com parâmetros por valor e por endereço (vetores/ponteiros).
* Tipos primitivos, vetores unidimensionais de tamanho fixo, registros, enumerações.
* Estruturas de controle: condicional, condicional encadeado, laços `ENQUANTO`,
  `REPITA`, laço contado, seleção múltipla, desvios `PARE`/`PROSSIGA`/`RETORNE`.
* E/S de console: escrever um valor, ler um valor, ler uma linha, escrever texto.
* Um pequeno conjunto de funções de conveniência (matemática, texto, erros fatais).
* Interoperabilidade irrestrita com C, incluindo `#include`, `struct`, ponteiros e
  funções da biblioteca padrão.

### 2.2 Fora do escopo (v1.0)

Classes/orientação a objetos, exceções, genéricos, sobrecarga de operadores, coleta de
lixo, concorrência nas palavras-chave, módulos/pacotes, reflexão, macros definidas pelo
usuário em sintaxe BG, internacionalização das mensagens do compilador, formatação
automática canônica (só estilo recomendado).

### 2.3 Três níveis de sintaxe (conceito central do projeto)

Todo construtor da linguagem recebe **um** rótulo:

| Rótulo | Significado | Consequência prática |
|---|---|---|
| **N** (núcleo) | Redutível a C por substituição de tokens, na ordem em que aparecem, sem reordenar nada | funciona no caminho **M0** (só macros) |
| **X** (extensão) | Exige *reordenar/inserir* tokens (ex.: `PARA i DE 1 ATE 10`) | só no caminho **M1** (transpilador) |
| **C** (nativo C) | Não é traduzido; é C mesmo, aceito dentro do programa | sempre disponível |

A regra de ouro: **um construtor N nunca depende do compilador “entender” algo; ele é
apenas um nome mais legível para tokens C que o programador já escreveria.** A §5.4
explica por que essa restrição é mais forte do que parece.

---

## 3. Princípios de projeto

| ID | Princípio | Justificativa / consequência |
|---|---|---|
| P-01 | **A fonte é C válido depois da expansão.** | Nada de “o transpilador resolve”. Se a expansão não é C, o construtor é X ou não existe. |
| P-02 | **Palavras-chave em MAIÚSCULAS, ASCII, sem acento.** | C não garante identificadores acentuados; MAIÚSCULAS separam visualmente o vocabulário da linguagem dos identificadores do usuário (P-05). |
| P-03 | **C permanece sempre disponível.** | A camada BG é *aditiva*. Você pode escrever `for (;;)` no meio de um programa BG. Isso é a escotilha de escape para tudo o que a §5.4 proíbe. |
| P-04 | **Um construtor, uma forma canônica.** | Duas sintaxes para a mesma coisa dobram a documentação e os testes. Apelidos existem só para compatibilidade (ver §16). |
| P-05 | **Identificadores do usuário têm minúsculas; o vocabulário da linguagem é MAIÚSCULO.** | Garante que adicionar uma palavra-chave nova em versões futuras não quebre programas existentes. |
| P-06 | **Bloco composto é aberto por palavra-chave e fechado por `FIM_*`.** | Macros conseguem fornecer `{` e `}`; não conseguem fornecer `}` “no fim de um bloco indentado”. `FIM_*` é explícito e verificável. |
| P-07 | **Comando simples termina em `;`; bloco composto não.** | O ponto e vírgula é do C e não pode ser inferido. Ver [D-05](04-restricoes-e-decisoes.md). |
| P-08 | **Tipos são explícitos; não há inferência.** | O pré-processador não infere tipos; `_Generic` escolhe *formatação*, não *tipo de variável*. |
| P-09 | **Expressões são expressões C.** | Precedência, promoções e conversões são as de C — a especificação não as reescreve, apenas as documenta (§9.3). |
| P-10 | **Açúcar nunca muda semântica.** | M1 existe para tornar a leitura melhor, não para mudar o comportamento. Todo construtor X tem expansão C definida por esta especificação. |
| P-11 | **Toda macro com argumentos parentiza os argumentos e avalia cada um uma vez.** | Evita os erros clássicos de macro (`MAX(i++, j)`). Onde não for possível, a operação é uma função `inline` da biblioteca. |
| P-12 | **Diagnóstico honesto > sintaxe bonita.** | Quando a sintaxe desejada produziria erro incompreensível, ela é proibida na especificação e o motivo é documentado (§12.3). |

---

## 4. Modelo de execução e perfis

### 4.1 Caminho M0 (macros)

O arquivo `.c` é compilado **como C**, sem nenhuma flag especial:

```sh
cc -std=c11 -Wall -Wextra programa.c -o programa -lm -I caminho/para/include
```

O cabeçalho define cada palavra-chave como macro (objeto ou função). Não há etapa
intermediária: o texto que o compilador analisa é o resultado do pré-processamento.

### 4.2 Caminho M1 (transpilador `basicgolc`, Fase 2)

```
programa.c  ──basicgolc──►  programa.gerado.c  ──cc──►  executável
```

A fonte e o C gerado são os dois `.c`: o `-o` escolhe onde o C gerado é escrito (nunca
sobre a fonte). O que muda é o conteúdo — o gerado não tem macros de palavra-chave.

`basicgolc` é um **reescritor de tokens**, não um compilador: ele mantém um pequeno
autômato (fora de bloco / cabeçalho de `PARA` / corpo) e faz quatro coisas:

1. troca palavras-chave pelos tokens C correspondentes;
2. insere `{` `}` onde as palavras-chave de bloco os exigem;
3. converte `PARA i DE a ATE b [PASSO p] FACA` em um `for` C equivalente (§11.5);
4. emite diagnósticos de estilo/armadilha (W-01…W-07, §12.4) e erros estruturais (§12.5).

Ele **não** faz análise semântica, não valida tipos, não resolve nomes. Se ele não
reconhece algo, repassa o texto sem tocar (o compilador C decide).

### 4.3 Equivalência obrigatória

Para qualquer programa no subconjunto N, o comportamento observável deve ser idêntico:

```
semântica(M0(fonte))  ≡  semântica(C( M1(fonte) ))
```

O critério de aceitação AC-32 (§[05](05-criterios-de-aceitacao.md)) exige essa
comparação em todos os exemplos.

---

## 5. Léxico

### 5.1 Arquivo-fonte

* Codificação **UTF-8** obrigatória. Byte order mark é tolerado e ignorado.
* Fim de linha: `LF`, `CRLF` ou `CR` (todos aceitos).
* Extensão **`.c`** (D-14): o fonte BG é um C comum, com `#include <basicgol.h>` no topo,
  e por isso `cc programa.c` funciona sem `-x c`, `make` e editores o reconhecem de
  imediato. Não existe extensão própria da linguagem.
* Um arquivo `.c` **é** uma unidade de tradução C. Não há sistema de módulos em v1.0:
  usa-se `#include` e `EXTERNO` (ver §13.3). A distinção "fonte BG" × "módulo C puro" é
  de conteúdo, não de nome: o módulo C puro simplesmente não inclui `basicgol.h`.

### 5.2 Comentários

| Forma | Escopo | Observação |
|---|---|---|
| `// texto` | até o fim da linha | forma canônica (igual a C99 e ao Portugol Studio) |
| `/* texto */` | multilinha, não aninha | use para cabeçalhos de arquivo/função |

`#` **não** inicia comentário: `#` pertence ao pré-processador C (diretivas).

### 5.3 Identificadores

```
identificador := (letra | "_") { letra | digito | "_" }
letra         := "A".."Z" | "a".."z"        (somente ASCII)
digito        := "0".."9"
```

Regras normativas:

* **R-1** Identificadores de usuário contêm **pelo menos uma letra minúscula**.
  Sequências 100% maiúsculas são reservadas ao vocabulário da linguagem e às macros de
  configuração (`BG_*`). Isso é o que torna P-05 verdadeiro e a evolução segura.
* **R-2** Prefixos reservados: `bg_` (biblioteca de execução), `BG_` (configuração),
  `k_` (convenção para constantes), `_` no início, e qualquer identificador contendo
  `__`.
* **R-3** Não são permitidos acentos, cedilha nem qualquer caractere fora de ASCII em
  identificadores — nem no caminho M0, nem no M1 (C não garante tal suporte, e a
  portabilidade vale mais do que a estética aqui). Textos acentuados dentro de strings
  são livres: `ESCREVAL("Programação")` é válido.
* **R-4** Nomes de palavras-chave (§5.4) não podem ser usados como identificadores de
  usuário, mesmo que o usuário não use o construtor correspondente.
* **R-5** Nomes de tipo definidos pelo usuário (`REGISTRO`, `ENUMERACAO`) usam
  `PascalCase` (`Aluno`, `FilaDeImpressao`), porque viram nomes de `typedef`.

### 5.4 Palavras-chave, e por que a lista é curta

Palavras-chave são **nomes de macro** definidos por `<basicgol.h>`. Isso impõe uma
restrição dura e define a linguagem:

> **Uma palavra-chave só pode existir se a sua expansão for um conjunto de tokens C
> válido na posição onde ela aparece — sem que o pré-processador precise examinar o
> resto da linha, contar parênteses de outras macros ou reordenar tokens.**

Exemplos do que isso permite e do que isso proíbe:

| Desejo | Viável? | Motivo |
|---|---|---|
| `SE cond ENTAO … FIM_SE` | **sim (N)** | `SE` → `if (`, `ENTAO` → `) {`, `FIM_SE` → `}`; o compilador C vê `if (cond) { … }` |
| `SENAO_SE cond ENTAO` | **sim (N)** | mesma técnica; ver o porquê de não ser `SENAO SE` em [C-05](04-restricoes-e-decisoes.md) |
| `ENQUANTO cond FACA … FIM_ENQUANTO` | **sim (N)** | idem com `while (` / `) {` |
| `VETOR(INTEIRO, v, 10)` | **sim (N)** | macro de função com 3 argumentos |
| `VETOR INTEIRO v DE 10` | **não (X)** | exigiria mover `INTEIRO` e `10` de posição; o pré-processador não reordena tokens |
| `PARA i DE 1 ATE 10 FACA` | **não (X)** | exige montar `for (i = 1; i <= 10; ++i)` — três posições fora de ordem |
| `ESCREVA "oi"` (sem parênteses) | **não (x, nunca)** | exigiria saber onde a expressão termina: o pré-processador é insensível a fim de linha |
| `SE x { … }` | **proibido por decisão** | funcionaria, mas colidiria com a forma canônica `ENTAO` (ver C-06) |
| `SE (x) ENTAO` | **sim, tolerado** | expande para `if ((x)) {` — parênteses redundantes são inofensivos |

A lista completa, com tipo de macro, expansão e risco, está em
[`02-referencia.md`](02-referencia.md) e [`03-mapeamento-para-c.md`](03-mapeamento-para-c.md).

### 5.5 Literais

| Literal | Sintaxe | Tipo BG |
|---|---|---|
| inteiro decimal | `42`, `0`, `1000000` | `INTEIRO` (ou o menor tipo C adequado, conforme C) |
| inteiro hexadecimal | `0x2A` | idem |
| real | `3.14`, `0.5`, `1e-3` | `REAL` |
| caractere | `'A'`, `'\n'`, `'\''` | `CARACTERE` |
| texto | `"Ola"`, `"linha\n"`, `"aspas: \" "` | `TEXTO` (`const char *`) |
| lógico | `VERDADEIRO`, `FALSO` | `LOGICO` |

Escapes aceitos: os de C (`\n \t \r \\ \" \' \0 \xHH \ooo`). Não há literal de texto
multilinha; use concatenação de literais justapostos (`"a" "b"`) ou `ESCREVAL` em
sequência.

---

## 6. Sistema de tipos

### 6.1 Tipos primitivos

| Palavra-chave | Tipo C | Garantia da especificação |
|---|---|---|
| `LOGICO` | `bool` (`<stdbool.h>`) | `VERDADEIRO`/`FALSO`; ocupa o que C decidir |
| `CARACTERE` | `char` | 1 byte; sinal implementação-definido (usar `INTEIRO8`/`NATURAL8` quando importar) |
| `INTEIRO_CURTO` | `short` | ≥ 16 bits |
| `INTEIRO` | `int` | ≥ 16 bits (na prática 32) |
| `INTEIRO_LONGO` | `long` | ≥ 32 bits |
| `INTEIRO8/16/32/64` | `int8_t`…`int64_t` | largura exata, complemento de dois (C11 `<stdint.h>`) |
| `NATURAL` | `unsigned` | ≥ 16 bits, sem sinal |
| `NATURAL_CURTO` | `unsigned short` | ≥ 16 bits, sem sinal |
| `NATURAL_LONGO` | `unsigned long` | ≥ 32 bits, sem sinal |
| `NATURAL8/16/32/64` | `uint8_t`…`uint64_t` | largura exata, sem sinal |
| `REAL_CURTO` | `float` | IEEE-754 simples (recomendado, não garantido) |
| `REAL` | `double` | IEEE-754 dupla (recomendado, não garantido) |
| `REAL_LONGO` | `long double` | ≥ precisão de `double` |
| `TEXTO` | `const char *` | ponteiro para bytes UTF-8, **imutável** pela variável |
| `VAZIO` | `void` | ausência de valor (retorno de procedimento) |

**Regra de portabilidade PT-1**: programas didáticos usam `INTEIRO` e `REAL`. Programas
que serializam dados, calculam máscaras ou dependem de estouro usam os tipos de largura
exata (`INTEIRO32`, `NATURAL64`).

**Regra PT-2**: `TEXTO` é `const char *` — atribuir um literal é válido, **modificar
caracteres não é** (`s[0] = 'x'` é erro de compilação, não aviso). Para texto mutável
use `TEXTO_FIXO(nome, tamanho)` (§7.4).

**Regra PT-4 (Fase 4)**: a largura dos tipos nativos é a da plataforma. Quando a largura
importa, use os tipos de largura exata (§6.1) e consulte `BITS_DE`/`TAMANHO_DE`; não
escreva números mágicos como `4` para `sizeof(int)`.

`CARACTERE` ocupa exatamente 1 byte e guarda **um byte**, não um caractere Unicode; o
sinal de `char` é definido pela implementação (por isso existem `INTEIRO8` e `NATURAL8`).
`LOGICO` é `bool`: `VERDADEIRO` vale 1, não −1.

A referência completa — faixas, conversões, operadores, valor de verdade, estouro,
ponto flutuante e diferenças em relação a BASIC/Portugol — está em
[06 — Tipos e Expressões](06-tipos-e-expressoes.md).

### 6.2 Qualificadores

| Palavra-chave | Expansão | Uso |
|---|---|---|
| `CONSTANTE` | `static const` | constante de arquivo ou de função; precisa de inicializador |
| `ESTATICO` | `static` | variável persistente entre chamadas (documentar no código) |
| `EXTERNO` | `extern` | declaração de símbolo definido em outra unidade de tradução |

`CONSTANTE` sem inicializador é erro de C; a especificação exige o inicializador:

```c
CONSTANTE INTEIRO k_max_alunos = 60;
CONSTANTE REAL    k_pi = 3.14159265358979;
```

### 6.3 Conversões

Não há sintaxe de conversão própria: usa-se o cast C, escrito como em C
(`(REAL) contador`), porque `(REAL)` já é `(double)` após a expansão da macro. A
especificação **recomenda** casts explícitos nas três situações em que C os exige ou
onde a leitura melhora: divisão inteira que deveria ser real, comparação entre tipos de
largura diferente e passagem de `INTEIRO` para parâmetro `REAL` em contexto de sobrecarga
inexistente.

### 6.4 Semântica herdada de C (declarada, não redefinida)

Estes comportamentos são **de C** e a especificação não os altera; ela os documenta
porque o público iniciante precisa saber que existem:

| Comportamento | Regra |
|---|---|
| Divisão inteira | truncamento em direção a zero (`7 / 2 == 3`, `-7 / 2 == -3`) |
| Resto `MOD` | sinal acompanha o dividendo (`-7 MOD 2 == -1`) |
| Estouro em inteiro com sinal | **comportamento indefinido** (UB); evite, use tipos largos |
| Estouro em natural (`NATURAL*`) | aritmética módulo 2^N, definida |
| Divisão por zero | UB; `DIVIDE`/`RESTO` transformam em erro com linha (Fase 4) |
| `MIN / -1` e `MIN MOD -1` | UB; `DIVIDE`/`RESTO` recusam |
| `ABS(MINIMO_INTEIRO)` | UB (não há positivo representável); compare antes |
| Conversão real→inteiro fora de faixa | UB; use `REAL_PARA_INTEIRO` |
| `NAN` | diferente de tudo, inclusive de si mesmo; use `isnan` |
| Índice fora do vetor | UB; não há verificação de limites |
| Precisão de `REAL` | binária; `0.1 + 0.2 != 0.3`; compare com tolerância |
| Curto-circuito de `E`/`OU` | garantido (é `&&`/`||`) |

**Regra PT-3**: a documentação de exercícios nunca deve depender de UB. Se um exemplo
didático precisa de estouro, ele é explicitamente marcado e usa tipo sem sinal (onde o
estouro é definido, módulo 2ᴺ) ou os auxiliares opcionais `SOMA_SEGURA`/`MULTIPLICA_SEGURA`
(§11 de [06](06-tipos-e-expressoes.md)).

---

## 7. Declarações

### 7.1 Variáveis

```
tipo identificador [ "=" expressao ] ";"
```

```c
INTEIRO contador = 0;
REAL    total;
LOGICO  aprovado = FALSO;
TEXTO   nome = "sem nome";
```

* **V-1** Toda variável local deve ser inicializada na declaração, exceto quando for
  atribuída antes de qualquer leitura. Não é possível impor isso por macro; o
  transpilador M1 emite o aviso **W-02**. A especificação trata a inicialização
  obrigatória como regra de estilo *normativa* (não verificável no caminho M0).
* **V-2** Declaração no meio do bloco é permitida (C99+), e é o estilo recomendado:
  declare perto do uso.
* **V-3** Não há escopo de bloco implícito: `SE`, `ENQUANTO` etc. **criam** um bloco C,
  então uma variável declarada dentro de um `SE` não existe fora dele (§11.1).

### 7.2 Vetores

```
"VETOR" "(" tipo "," identificador "," expressao_constante ")" ";"
```

```c
VETOR(INTEIRO, notas, 4);
VETOR(REAL, medias, 30);
VETOR(TEXTO, nomes, 10);
```

| Aspecto | Regra |
|---|---|
| Índices | `0 .. n-1` |
| Limites | sem verificação em tempo de execução (UB) |
| Tamanho | **literal inteiro** ou macro de `#define`. Atenção: `CONSTANTE` é `static const` e em C **não** é expressão constante, então `VETOR(t, v, k_max)` viraria VLA (ver C-23). VLA é aceito por C11 mas desencorajado |
| Tamanho em elementos | `TAMANHO_VETOR(v)` → `sizeof(v)/sizeof((v)[0])`; **só funciona em vetor de verdade**, não em ponteiro |
| Passagem para função | o tamanho não acompanha o vetor: passe `(vetor, quantidade)` (§8.3) |
| Atribuição | `v2 = v1` **não copia** (é ponteiro) → use laço ou `COPIA_VETOR` (futuro, ver D-12) |
| Matrizes | fora do escopo v1.0 (não há construtor: a macro teria de reordenar `tipo`, `nome` e duas dimensões). Use C nativo: `INTEIRO m[3][3];` (escotilha P-03) |

### 7.3 Registros

```
"REGISTRO" "(" Nome ")" { tipo identificador ";" }* "FIM_REGISTRO" "(" Nome ")" ";"
```

```c
REGISTRO(Aluno)
    INTEIRO matricula;
    TEXTO   nome;
    REAL    nota;
FIM_REGISTRO(Aluno);
```

Expansão: `typedef struct Aluno { … } Aluno;`. Consequências normativas:

* O nome aparece **duas vezes** (na abertura e no fechamento) porque a macro não pode
  repetir um argumento no final do bloco. Repetir é obrigatório e o compilador acusa
  divergência (§12.3).
* Campos são acessados com ponto: `aluno.nota`.
* Não há herança, métodos, construtores nem campos privados: é `struct` C.
* Estruturas aninhadas são permitidas (declare o registro interno antes).
* Registros são passados **por valor** como em C (cópia); para evitar cópias em funções
  que recebem muitos registros, use ponteiro (C nativo) e documente.
* Nome do registro deve ser `PascalCase` (R-5) e o `typedef` resultante é usado sem a
  palavra `struct`: `Aluno a;`

### 7.4 Textos mutáveis

```
"TEXTO_FIXO" "(" identificador "," expressao_constante ")" ";"
```

```c
TEXTO_FIXO(nome, 40);      /* char nome[40]; */
```

Usado quando o texto precisa ser modificado ou preenchido por leitura (§10.4). Limite
`tam-1` caracteres úteis (o último byte é o terminador). Sem verificação de estouro.

### 7.5 Enumerações

```c
ENUMERACAO(Conceito)
    CONCEITO_A = 10,
    CONCEITO_B,
    CONCEITO_C
FIM_ENUMERACAO(Conceito);
```

Expansão: `typedef enum Conceito { … } Conceito;`. Valores são `INTEIRO`; `ESCOLHA`
sobre enumeração é o uso canônico. Nomes de enumeradores são 100% maiúsculos (são
constantes), o que é permitido porque **não** são macros — mas a verificação de R-1 do
transpilador M1 trata enumeradores como exceção documentada (W-05).

---

## 8. Estrutura de um programa

### 8.1 Forma geral

```
[ #include <basicgol.h> ]
[ #include <...> | #define ... ]*

[ declarações globais ]*
[ FUNCAO/PROCEDIMENTO ]*
INICIO
    comandos
FIM
[ FUNCAO/PROCEDIMENTO ]*
```

`INICIO` expande para `int main(void) {` e `FIM` para `}`. Portanto:

* **um** `INICIO` por programa executável;
* `FIM` fecha exatamente esse bloco;
* `INICIO` não recebe argumentos (não há `argc/argv` no vocabulário BG; use
  `int main(int argc, char **argv)` em C nativo se precisar);
* um arquivo de biblioteca (sem `INICIO`) é permitido e é apenas C.

### 8.2 Funções e procedimentos

```c
FUNCAO INTEIRO soma(INTEIRO a, INTEIRO b) {
    RETORNE a + b;
}

PROCEDIMENTO saudacao(TEXTO nome) {
    ESCREVA("Ola, ");
    ESCREVA_TEXTO(nome);
    ESCREVA_LINHA();
}
```

| Regra | Detalhe |
|---|---|
| `FUNCAO` | palavra-chave **vazia** (`#define FUNCAO`); o tipo de retorno vem em seguida. Use quando há valor de retorno e `RETORNE`. |
| `PROCEDIMENTO` | expande para `void`; dispensa `RETORNE`. |
| Corpo | usa **chaves C** (`{ }`). Ver [C-06](04-restricoes-e-decisoes.md) para o motivo. |
| Protótipo | igual a C: `FUNCAO REAL media(REAL, REAL);` antes do uso, se a definição vier depois. |
| Recursão | permitida (é C). |
| Visibilidade | funções são globais; use `ESTATICO` para torná-las privadas do arquivo. |
| Sobrecarga | não existe. Um nome, uma assinatura. |
| `main` | só via `INICIO`. Definir `main` à mão junto com `INICIO` é erro de duplicação. |

### 8.3 Parâmetros

| Forma | Semântica |
|---|---|
| `INTEIRO n` | por valor (cópia) |
| `REAL x` | por valor |
| `TEXTO s` | cópia do ponteiro; o texto apontado é imutável |
| `INTEIRO v[]` | por referência (C ajusta para `INTEIRO *`); o tamanho **não** é transportado |
| `INTEIRO *p` | C nativo, por referência explícita |

Estilo recomendado para vetores:

```c
PROCEDIMENTO imprime_vetor(INTEIRO v[], INTEIRO n) {
    INTEIRO i;
    PARA (i, 0, n - 1) {
        ESCREVA(v[i]);
        ESCREVA(" ");
    }
    ESCREVA_LINHA();
}
```

Chamada: `imprime_vetor(notas, TAMANHO_VETOR(notas));` — o idioma canônico da
linguagem para "vetor + tamanho".

---

## 9. Expressões e operadores

### 9.1 Operadores

| Categoria | Formas BG | Formas C aceitas |
|---|---|---|
| atribuição | `=` | `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `++`, `--` (ver nota sobre `MOD=`) |
| aritmética | `+ - * / MOD` | `+ - * / %` |
| comparação | `== != < <= > >=`, `IGUAL(a,b)`, `DIFERENTE(a,b)` | idem |
| lógica | `E OU NAO`, `OU_EXCLUSIVO(a,b)`, `LOGICO_DE(x)` | `&& || !` |
| divisão/resto verificados | `DIVIDE(a,b)`, `RESTO(a,b)` | — (transformam zero e `MIN/-1` em erro) |
| acesso | `.` `[ ]` `( )` | idem |
| endereço/indireção | `&` `*` | idem (C nativo) |
| texto | `TEXTOS_IGUAIS(a,b)`, `TEXTOS_DIFERENTES(a,b)`, `COMPARA_TEXTO(a,b)` | idem |

`MOD` e `%` são **a mesma coisa** (`MOD` expande para `%`). A forma por palavra é
canônica na documentação didática; `%` permanece válido porque a fonte é C.

**`MOD=` não existe.** `MOD` é um identificador e `=` é outro token: `x MOD= 2`
expandiria para `x % = 2` e não compila (é a restrição C-01 aplicada a operadores
compostos — o pré-processador não pode criar o token único `%=`). Escreva
`x = x MOD 2` (canônico) ou `x %= 2` (C nativo).

### 9.2 Precedência

A precedência é **exatamente a de C** — não há tabela própria porque não há gramática
própria de expressões (P-09). Resumo operacional (do mais forte ao mais fraco):

```
( )  [ ]  .  ->            chamada, índice, membro
++ -- (pós)  ! ~ NAO       unários pós-fixos
* & + - (unários)  (cast)  unários
* / MOD
+ -
< <= > >=
== !=
E (&&)
OU (||)
= += -= *= /= %=
,                          vírgula (evitar)
```

**Recomendação R-6**: use parênteses sempre que misturar `E`/`OU`, ou comparação com
aritmética. `SE a E b OU c ENTAO` é legal e perigoso; `SE (a E b) OU c ENTAO` é claro.

### 9.3 Cuidados de expressão

| Armadilha | Regra da linguagem |
|---|---|
| `=` versus `==` | `=` atribui, `==` compara — como em C. Escrever `SE x = 5 ENTAO` é **erro ou bug**; M1 avisa (W-01). Use `IGUAL(x, 5)` quando quiser eliminar a dúvida. |
| Comparar textos com `==` | compara **ponteiros**, não conteúdo. Use `TEXTOS_IGUAIS`. |
| `NAO` com `E`/`OU` | `NAO a E b` = `(!a) && b` (precedência de `!`). Use parênteses. |
| Dupla avaliação | as macros com argumentos avaliam cada argumento **uma vez** (P-11), inclusive `ABS`, `MINIMO`, `MAXIMO` (implementadas como funções `inline`). |
| Efeito colateral em argumento de `ESCREVA` | `ESCREVA(i++)` avalia uma vez e imprime antes do incremento — evite por legibilidade. |
| Verdade em C | qualquer valor diferente de zero é verdadeiro; a especificação recomenda condições `LOGICO` ou `LOGICO_DE(x)`. M1 avisa (W-03). |
| `NaN` em comparação | todas as comparações com `NaN` são falsas, inclusive `NaN == NaN`; use `isnan(x)` |
| `^` não é potência | em C, `^` é XOR bit a bit; use `POTENCIA`. `2 ^ 3` vale 1 |
| Divisor zero em reais | `/` produz infinito/NaN; `DIVIDE` recusa o divisor zero também em real |
| Texto `NULL` | `TEXTOS_IGUAIS`/`strlen` não aceitam `NULL`; `COMPARA_TEXTO`/`TEXTO_VAZIO`/`TAMANHO_TEXTO` tratam como vazio |

---

## 10. Entrada e saída

Todo E/S de BG é E/S de console via `<stdio.h>`. Não há formatação implícita de vários
valores (§10.5).

### 10.1 Escrever

| Construtor | Efeito |
|---|---|
| `ESCREVA(x);` | escreve **um** valor, sem quebra de linha |
| `ESCREVAL(x);` | escreve **um** valor e quebra a linha |
| `ESCREVA_LINHA();` | apenas quebra a linha |
| `ESCREVA_TEXTO(s);` | escreve texto com `%s`, sem consultar tipo (mais previsível e rápido) |
| `ESCREVA_FMT(fmt, ...);` | `printf` direto — escotilha para formatação avançada (`%05.2f`, `%x`) |
| `ESCREVA_ERRO(s);` | escreve em `stderr` (diagnóstico ao usuário) |

`ESCREVA`/`ESCREVAL` aceitam um valor de tipo escalar conhecido (`LOGICO`, `CARACTERE`,
inteiros, reais, `TEXTO`, enumerações) e escolhem o formato em tempo de compilação via
`_Generic` (C11). Consequências normativas:

* **o argumento é uma expressão de tipo escalar**, não uma lista: `ESCREVAL(a, b)` é
  inválido (o `,` separaria argumentos de macro) → use duas chamadas;
* ponteiros genéricos caem no formato `%p` (útil só para depuração);
* **não há controle de casas decimais** em `ESCREVAL` (o formato é `%g`): para
  `9.50` use `ESCREVA_FMT("%.2f", nota)`.

### 10.2 Ler

| Construtor | Efeito |
|---|---|
| `LEIA(x);` | lê **um** valor escalar para `x`; falha de leitura é ignorada |
| `LEIA_OK(x);` | igual, mas devolve `LOGICO` (para validação) |
| `LEIA_TEXTO(buf, tam);` | lê uma linha inteira para `TEXTO_FIXO`; antes disso consome **um** `\n` pendente, se houver |

Regra de ouro (derivada de um defeito real encontrado na sonda de viabilidade):
**uma leitura numérica com `scanf` deixa o `\n` pendente no `stdin`**, e o `fgets`
seguinte leria uma linha vazia — o programa “pula” a leitura de texto. Por contrato,
`LEIA_TEXTO` **consome um único terminador de linha pendente** (se houver) e só então lê
uma linha inteira. Isso é responsabilidade da função `bg_leia_texto` da biblioteca de
execução, não do programador (ver C-20 em [04](04-restricoes-e-decisoes.md)).

Consequência importante para quem implementa: o contrato é condicional — *consome se
houver, sem descartar conteúdo*. Uma implementação ingênua que “sempre descarta até o fim
da linha” engole a primeira linha do programa quando não há nada pendente, e é
considerada **incorreta** (o exemplo `contador_vogais.c` é o teste desse caso).

### 10.3 Saída de exemplo

```c
ESCREVA("Nome: ");
ESCREVA_TEXTO(nome);
ESCREVA("  nota: ");
ESCREVA_FMT("%.2f", nota);
ESCREVA_LINHA();
ESCREVAL(TAMANHO_VETOR(notas));   /* inteiro sem sinal → escolhido por _Generic */
```

### 10.4 Texto, buffers e o modelo de memória

* `TEXTO` é uma **referência imutável** (`const char *`) — barato de passar, impossível
  de corromper por acidente.
* `TEXTO_FIXO(nome, tam)` é um **buffer** de `char` — é o que `LEIA_TEXTO` preenche e o
  que `JUNTA_TEXTO` escreve.
* Não há alocação automática, `String`, nem coleta de lixo. `JUNTA_TEXTO(destino, tam,
  a, b)` devolve `LOGICO` indicando se coube; truncamento nunca é silencioso por
  contrato (a função deve devolver `FALSO`).
* Programas que precisam de texto de tamanho dinâmico usam `malloc` (C nativo) e
  documentam a liberação.

### 10.5 Limite conhecido (honesto)

`ESCREVAL(a, b, c)` — impressão de vários valores com formatação automática — **não
existe** e não existirá no caminho M0: exigiria contar e classificar argumentos
variádicos no pré-processador (possível com técnicas tipo P99, mas desproporcional ao
ganho e péssimo para mensagens de erro). Ver D-07. A alternativa idiomática é uma
chamada por valor, ou `ESCREVA_FMT`.

---

## 11. Controle de fluxo

Todas as formas abaixo são **N** (núcleo), salvo indicação. Nenhuma delas exige
parênteses na condição, mas parênteses são tolerados (`SE (x > 0) ENTAO`).

### 11.1 Condicional

```
SE cond ENTAO
    ...
[ SENAO_SE cond ENTAO
    ... ]*
[ SENAO
    ... ]
FIM_SE
```

```c
SE nota >= 7.0 ENTAO
    ESCREVAL("Aprovado");
SENAO_SE nota >= 5.0 ENTAO
    ESCREVAL("Recuperacao");
SENAO
    ESCREVAL("Reprovado");
FIM_SE
```

Regras normativas:

* **CND-1** `SE` exige `ENTAO`. `SE x { … }` é **proibido** — `SE` já abre o parêntese da
  condição, então `{` depois dele é erro de sintaxe (com mensagem razoável, §12.3).
* **CND-2** Encadeamento é feito com **`SENAO_SE`**, nunca `SENAO SE`
  ([C-05](04-restricoes-e-decisoes.md)).
* **CND-3** `SENAO` (e `SENAO_SE`) só podem aparecer depois de `SE`/`SENAO_SE`.
* **CND-4** `FIM_SE` é obrigatório uma única vez, no fim da cadeia inteira (não por ramo).
* **CND-5** Blocos criam escopo C: variáveis declaradas dentro de um ramo não existem fora
  dele. Para um valor produzido por um `SE`, declare a variável **antes** e atribua
  dentro.
* **CND-6** `;` depois de `FIM_SE` é opcional e inofensivo (vira comando vazio), mas o
  estilo canônico o omite.

### 11.2 Laço enquanto

```
ENQUANTO cond FACA
    ...
FIM_ENQUANTO
```

```c
INTEIRO i = 0;
ENQUANTO i < 10 FACA
    ESCREVAL(i);
    i = i + 1;
FIM_ENQUANTO
```

* `ENQUANTO` testa **antes** de cada iteração (pode executar zero vezes).
* A condição é reavaliada a cada iteração; mantenha-a simples e sem efeitos colaterais.
* Modificar a variável de controle dentro do corpo é permitido e é a forma de sair (além
  de `PARE`).

### 11.3 Laço repita

```
REPITA
    ...
ATE_QUE(cond);
```

* Executa o corpo **ao menos uma vez** e testa **depois**.
* `ATE_QUE(cond)` termina o laço **quando `cond` for verdadeira**
  (`do { … } while (!(cond))`).
* A condição vai **entre parênteses** e o comando termina com `;` (é o `while` do C que
  fecha a construção).
* No caminho M1, a forma com açúcar é aceita: `ATE cond;` — ver `comando_repita_sugar` na
gramática (§17) e D-11.

### 11.4 Laço contado (núcleo, M0)

```
PARA (variavel, inicio, fim) {
    ...
}
```

```c
INTEIRO i;
PARA (i, 1, 10) {
    ESCREVAL(i);
}
```

Semântica exata: `for ((i) = (inicio); (i) <= (fim); ++(i))`.

* **inclusivo nos dois extremos**;
* `inicio` é avaliado **uma vez**; `fim` é reavaliado **a cada iteração** (é a expressão
  do teste do `for`). Regra: `fim` deve ser livre de efeitos colaterais;
* se `fim < inicio` o corpo não executa;
* a variável **deve existir antes** e não pode ser declarada na construção;
* não altere a variável de controle dentro do corpo (comportamento definido, mas quase
  sempre bug) → M1 avisa (W-04);
* passo diferente de 1: `PARA_PASSO(i, 0, 100, 10) { … }`. O sinal do passo decide a
  condição de parada: passo positivo conta para cima (`i <= fim`), negativo conta para
  trás (`i >= fim`). É o mesmo teste nos dois perfis (M0 e M1) — a igualdade é verificada
  pelo caso `03_controle/c23_para_passo_negativo`;
* passo **zero** é erro de execução (Fase 3): o laço para com
  `arquivo:linha: erro: PARA_PASSO: passo zero (o laco nunca avancaria)` e código 1,
  em vez de girar para sempre. Vale mesmo com intervalo vazio (casos
  `03_controle/c26` e `c27`). No M1, passo literal `0` é recusado já na tradução;
  ver [07 §5](07-controle-de-fluxo.md). Sem o passo zero, a expansão e o resultado
  para `p ≠ 0` são exatamente os de antes;
* contagem decrescente: use `ENQUANTO` (recomendado) ou C nativo.

### 11.5 Laço contado (açúcar, M1)

```
PARA i DE 1 ATE 10 FACA
    ...
FIM_PARA

PARA i DE 10 ATE 1 PASSO -1 FACA
    ...
FIM_PARA
```

Expansão M1 obrigatória (com `p` = `PASSO expressao` ou `1` quando omitido):

```c
for (i = (inicio); (p) > 0 ? (i <= (fim)) : ((p) == 0 ? bg_passo_zero(__FILE__, __LINE__) : (i >= (fim))); i += (p))
```

Exemplo: `PARA i DE 10 ATE 1 PASSO -1 FACA` →

```c
for (i = (10); (-1) > 0 ? (i <= (1)) : ((-1) == 0 ? bg_passo_zero(__FILE__, __LINE__) : (i >= (1))); i += (-1))
```

Regras: `i` pré-existente; limites inclusivos; `PASSO` opcional (padrão `+1`) e deve ser
constante não nula (o sinal escolhe o sentido); se o intervalo for vazio (`fim < inicio`
com passo positivo), o corpo não executa; `FIM_PARA` fecha. **Não existe no caminho M0**, e
programas que precisam compilar sem `basicgolc` usam §11.4.

### 11.6 Seleção múltipla

```
ESCOLHA (expr)
    CASO valor:
        ...
        PARE;
    CASO valor:
        ...
    CONTRARIO:
        ...
FIM_ESCOLHA
```

```c
ESCOLHA (opcao)
    CASO 1:
        ESCREVAL("Novo");
        PARE;
    CASO 2:
        ESCREVAL("Abrir");
        PARE;
    CONTRARIO:
        ESCREVAL("Opcao invalida");
FIM_ESCOLHA
```

* `ESCOLHA(expr)` é macro de função: **`expr` não pode ter vírgula no nível superior**
  (`ESCOLHA(a, b)`), porque a vírgula seria lida como separador de argumentos.
  Chamadas aninhadas como `ESCOLHA(f(a, b))` **funcionam**, porque os parênteses internos
  protegem a vírgula — a Fase 2 verificou isso empiricamente e o caso
  `tests/casos/03_controle/c22_escolha_com_chamada` fixa o comportamento.
  Isto **corrige** a redação da Fase 1 (ver C-04 em [04](04-restricoes-e-decisoes.md));
* `CASO` expande para `case`, `CONTRARIO` para `default` (o `:` é do programador);
* `PARE;` é obrigatório no fim de cada caso que não deve "cair" no seguinte. Queda
  intencional é C e é permitida, mas `-Wextra` a transforma em erro
  (`-Wimplicit-fallthrough`): anote com `/* fall through */` e, se o compilador não
  reconhecer o comentário dentro das macros, desligue só esse aviso
  (`-Wno-implicit-fallthrough`) — é o que o caso `03_controle/c19_escolha_fallthrough`
  faz, com o motivo documentado;
* um caso que precisa de variáveis locais usa um bloco C explícito:
  `CASO 3: { INTEIRO t = 1; … PARE; }`;
* `expr` deve ser inteiro ou enumeração (regra de C);
* **não** há `ESCOLHA` sobre texto — use `SE`/`SENAO_SE` com `TEXTOS_IGUAIS`.

### 11.6.1 Comparações: infixo é C, prefixado é português

`IGUAL(a, b)` e `DIFERENTE(a, b)` são macros de **função**, logo só existem na forma
prefixada; o C não permite criar um operador infixo a partir delas. Na prática:

```c
SE x == 3 ENTAO          // infixo, é o C de sempre
SE IGUAL(x, 3) ENTAO     // prefixado, leitura em português
SE x IGUAL 3 ENTAO       // ERRO: 'IGUAL' sem '(' — o nome fica indefinido
```

Para textos, `TEXTOS_IGUAIS(a, b)`/`TEXTOS_DIFERENTES(a, b)` são as formas corretas
(`==` compara ponteiros). O transpulador M1 acusa o uso prefixado sem parênteses antes de
o C reclamar.

### 11.7 Desvios

| Comando | Efeito | Onde |
|---|---|---|
| `PARE;` | sai do laço/`ESCOLHA` mais interno | dentro de `ENQUANTO`, `REPITA`, `PARA`, `ESCOLHA` |
| `PROSSIGA;` | salta para a próxima iteração | dentro de laço |
| `RETORNE expr;` | devolve valor | em `FUNCAO` |
| `RETORNE;` | devolve nada | em `PROCEDIMENTO` |

Não há `ir para` (goto) no vocabulário: se precisar, é C nativo e a especificação exige
um comentário justificando.

---

## 12. Erros, diagnósticos e depuração

### 12.1 Política

BASICGOL **não tem exceções** ([D-06](04-restricoes-e-decisoes.md)). Erros são tratados
por três mecanismos, todos explícitos:

1. **Valor de retorno** — a forma preferida: `LOGICO` para sucesso/falha, ou código.
   ```c
   SE NAO LEIA_OK(idade) ENTAO
       ESCREVAL("Entrada invalida");
   FIM_SE
   ```
2. **Contrato verificável** — `AFIRME(cond, "mensagem");` interrompe o programa com
   arquivo/linha quando `cond` é falsa. Uso: invariantes internas e pré-condições de
   função, não validação de entrada do usuário.
3. **Erro fatal explícito** — `ERRO("mensagem");` imprime em `stderr` e encerra com
   código de falha. Uso: situação irrecuperável (arquivo obrigatório ausente).

### 12.2 Mensagens

* Mensagens de erro em tempo de execução são em **português**, sem acento por padrão
  (`"divisao por zero"`), para não depender da codificação do terminal. O programa pode
  usar acentos em mensagens próprias.
* Formato de `ERRO`: `arquivo.c:linha: erro: mensagem`.
* Códigos de saída: `0` sucesso; `1` erro fatal (`ERRO`/`AFIRME`); `2` erro de uso
  (definido pela aplicação, se houver).

### 12.3 Erros de compilação: o que esperar

O compilador C é o compilador de BG. A qualidade da mensagem depende da expansão; esta
tabela é parte da especificação porque o professor vai encontrá-la.

Todas as mensagens abaixo foram **reproduzidas na Fase 2** (GCC 12.2) e são as que a
suíte afirma, uma por uma, em `tests/casos/06_negativos/`.

| Erro do aluno | Mensagem observada | Como explicar |
|---|---|---|
| esqueceu `FIM_SE`/`FIM_ENQUANTO` | `expected declaration or statement at end of input` | “falta fechar o bloco aberto com `SE`” |
| escreveu `SE x {` | `expected ')' before '{' token` + nota mostrando a origem da macro `SE` | “`SE` já abre parêntese; use `ENTAO`” |
| escreveu `SENAO SE` | `expected declaration or statement at end of input` (o `{` extra do `SENAO` estoura no fim) | “use `SENAO_SE`” |
| esqueceu `ENTAO` | `called object is not a function or function pointer` | “`SE` sempre tem `ENTAO`” |
| usou `VETOR INTEIRO` | `'BG_VETOR' undeclared (first use in this function)` | “`VETOR` é chamada: `VETOR(INTEIRO, v, 10);`” |
| usou `PARA i DE 1 ATE 10` sem M1 | `unknown type name 'BG_PARA'` | “no modo macro use `PARA (i, 1, 10) { }` (ou compile com `basicgolc`)” |
| escreveu `MOD=` | `expected expression before '=' token` | “não existe `MOD=`: `x = x MOD 3;`” |
| usou palavra-chave como variável (`INTEIRO SE`) | `expected identifier or '(' before 'if'` | “o nome é das palavras-chave; escolha outro” |
| dois `INICIO` | `redefinition of 'main'` | “um programa tem um `INICIO`” |
| `FIM` a mais | `expected identifier or '(' before '}' token` | “há um `FIM` sobrando” |
| registrou com nomes divergentes | `unknown type name` / `conflicting types` | “o nome aparece na abertura e no fechamento do `REGISTRO`” |
| `ESCREVAL(a, b)` | `macro "BG_ESCREVAL" passed 2 arguments, but takes just 1` | “uma chamada por valor” |
| atribuiu a `TEXTO` | `assignment of read-only location '*s'` | “`TEXTO` é imutável; use `TEXTO_FIXO`” |
| comparou textos com `==` | compila; resultado errado | usar `TEXTOS_IGUAIS` |

> **Nota de projeto (honesta):** mensagens de macro apontam para o cabeçalho e citam
> `BG_*`, não o que o aluno escreveu. É exatamente para isso que existe o caminho M1: o C
> gerado não tem macros de palavra-chave, então a mensagem aponta para o `programa.c` e
> para a linha certa. O que o M1 resolve sozinho, sem esperar o compilador, está em §12.5.

> **Nota de projeto (honesta):** mensagens de macro são piores que mensagens de C nativo.
> É exatamente para melhorá-las que existe o caminho M1: o C gerado tem palavras-chave
> C, sem macros, e as mensagens passam a apontar para o código que o aluno escreveu, com
> `{`/`}` explícitos.

### 12.5 Erros que o transpilador M1 acusa antes do C

| Situação | Mensagem (`arquivo:linha: erro: …`) |
|---|---|
| `SE`/`ENQUANTO` com `{` ou `;` no cabeçalho | `'SE' exige 'ENTAO' antes de '{' (nao use chaves nem ';' no cabecalho do bloco)` |
| `SENAO SE` | `'SENAO SE' nao encadeia: use SENAO_SE (C-05 da especificacao); para um SE dentro do SENAO, escreva antes dele um comando do bloco` |
| bloco fechado com o `FIM_*` errado | `'FIM' fecha o bloco errado: esperado 'FIM_SE' (aberto na linha 3)` |
| bloco nunca fechado | `'INICIO' aberto aqui nao foi fechado (falta 'FIM')` |
| `PARA … DE` sem `ATE` / sem `FACA` | `faltou 'ATE' no laco 'PARA i DE ... ATE ...'` |
| `PARA i …` sem `DE` | `esperado 'DE' depois da variavel de controle em 'PARA i DE ...'` |
| `ATE` sem `REPITA` | `'ATE' sem 'REPITA': use 'REPITA ... ATE_QUE(cond);' ou abra um REPITA` |
| `REGISTRO(Nome)`/`ENUMERACAO(Nome)` seguido de `{` | `'REGISTRO' ja abre o bloco: nao use '{' (va direto para os campos)` |
| macro de função usada como infixo (`x IGUAL 3`) | `'IGUAL' e prefixado e exige parenteses: use IGUAL(...)` |

Cada linha dessa tabela é um caso em `tests/casos/12_negativos_m1/`, e o código de saída
do `basicgolc` é sempre `1` quando há qualquer erro (nada é escrito no arquivo de saída).

### 12.4 Diagnósticos do transpilador M1 (avisos, não erros)

| ID | Aviso |
|---|---|
| W-01 | atribuição (`=`) usada como condição de `SE`/`ENQUANTO` (provável confusão com `==`) |
| W-02 | variável local declarada sem inicializador |
| W-03 | condição que é um literal numérico em `SE`/`ENQUANTO` (padrão "verdade em C": prefira `LOGICO` ou comparação) |
| W-04 | variável de controle de `PARA`/`PARA_PASSO` modificada dentro do corpo |
| W-05 | identificador 100% maiúsculo declarado pelo usuário (viola R-1) |
| W-06 | `TAMANHO_VETOR` aplicado a parâmetro declarado como `[]`/`*` (C-18) |
| W-07 | `VETOR`/`TEXTO_FIXO` com tamanho que é um identificador `k_*` (viraria VLA, C-23) |

---

## 13. Interoperabilidade com C

### 13.1 Regra geral

**P-03**: a camada BG é aditiva. Em qualquer ponto onde a gramática BG permite um
comando, o C correspondente também é aceito, porque a fonte já é C.

```c
INICIO
    /* BG e C lado a lado */
    INTEIRO i;
    for (i = 0; i < 3; i++) { printf("C puro: %d\n", i); }

    PARA (i, 0, 2) {
        ESCREVAL(i);
    }
FIM
```

### 13.2 Regras de convivência

* **I-1** Qualquer `#include`, `#define`, `typedef`, `struct`, `enum`, `union` de C pode
  ser usado. `#define` do usuário não deve usar nomes 100% maiúsculos que colidam com
  palavras-chave.
* **I-2** Bibliotecas C não devem ser incluídas **antes** de `basicgol.h` apenas por
  estilo; a ordem não afeta corretude, mas o cabeçalho deve vir primeiro para que macros
  de configuração (`BG_*`) tenham efeito.
* **I-3** `#define BG_SEM_APELIDOS` antes do `#include` desliga os nomes curtos
  (`SE`, `ENTAO`, …) e mantém apenas os nomes prefixados (`BG_SE`, `BG_ENTAO`, …). Use
  isso ao incluir `basicgol.h` em um projeto C existente para eliminar qualquer risco de
  colisão (ver D-08 em [04](04-restricoes-e-decisoes.md)).
* **I-4** Ponteiros, `malloc/free`, `struct` anônima, bit-fields: C nativo, sem
  vocabulário BG. A especificação não os proíbe nem os embrulha.
* **I-5** Chamar BG de C: as funções BG são funções C comuns com nomes comuns. Não há
  runtime obrigatório além de `<basicgol.h>` (que é header-only, salvo a biblioteca de
  execução, §7 de [03](03-mapeamento-para-c.md)).
* **I-6** Não use `main` em arquivo com `INICIO`.

### 13.3 Múltiplos arquivos (v1.0)

Sem sistema de módulos. O idioma é:

```c
/* biblioteca.c — sem INICIO */
FUNCAO REAL media(REAL a, REAL b) { RETORNE (a + b) / 2.0; }
```

```c
/* principal.c */
#include <basicgol.h>
EXTERNO FUNCAO REAL media(REAL, REAL);   /* ou: #include "biblioteca.h" gerado à mão */
```

Compilação: `cc -std=c11 principal.c biblioteca.c -o app -lm` (o `.c` é C).

---

## 14. Convenções de nomenclatura e formatação

### 14.1 Nomes

| Elemento | Convenção | Exemplo |
|---|---|---|
| variável | `snake_case` minúsculo | `total_vendas`, `indice` |
| função/procedimento | `verbo_substantivo` | `calcula_media`, `imprime_relatorio` |
| tipo (`REGISTRO`/`ENUMERACAO`) | `PascalCase` | `Aluno`, `TipoPagamento` |
| constante | `k_` + `snake_case` | `k_pi`, `k_max_alunos` |
| enumerador | MAIÚSCULO com `_` (exceção de R-1) | `CONCEITO_A` |
| palavra-chave | MAIÚSCULA | `ENQUANTO` |
| reservado | prefixos `bg_`, `BG_` | `bg_leia_texto` |

### 14.2 Formatação canônica (recomendada, não imposta)

* 4 espaços por nível, sem tabulações.
* Uma instrução por linha; `{` na mesma linha do cabeçalho de função.
* Palavras-chave de bloco em coluna própria, alinhadas com o `SE`/`ENQUANTO` que fecham:
  ```c
  SE x > 0 ENTAO
      ...
  FIM_SE
  ```
* Espaço em volta de operadores binários; nenhum espaço dentro de `(` `)` de chamada.
* Limite de 100 colunas.
* `FIM_*` sempre na mesma coluna do abridor correspondente — é a única indentação que a
  linguagem *recomenda com força*, porque é ela que torna o aninhamento legível sem
  chaves.

### 14.3 Ordem recomendada de um arquivo

1. `#include <basicgol.h>`
2. outras inclusões
3. constantes globais
4. tipos (`REGISTRO`, `ENUMERACAO`)
5. protótipos
6. funções
7. `INICIO … FIM`

---

## 15. Política de portabilidade

| ID | Requisito |
|---|---|
| PT-A | Dialeto C: **C11** mínimo (`_Generic` é obrigatório). C17 e C23 são aceitos. C99 não é suportado (sem `_Generic`, `ESCREVA`/`LEIA` não funcionam). |
| PT-B | Compiladores de referência: GCC ≥ 9, Clang ≥ 10, MSVC ≥ 19.28 (`/std:c11`). Nenhuma extensão GNU é necessária. |
| PT-C | Bibliotecas: apenas a biblioteca padrão de C; `-lm` é necessário para `POTENCIA`, `RAIZ`, `ARREDONDA`, `PISO`, `TETO` em plataformas que separam a libm. |
| PT-D | Código-fonte em UTF-8. Textos acentuados são apenas bytes; o terminal do usuário decide a exibição. Identificadores são ASCII (R-3). |
| PT-E | Nenhuma garantia de tamanho de `INTEIRO`/`REAL` além do que C11 garante; use tipos de largura exata quando importar. |
| PT-F | Nenhuma dependência de sistema operacional nas construções da linguagem. Console é `stdio`; `LIMPA_TELA` e cores **não** existem em v1.0. |
| PT-G | Sem `#pragma` obrigatório, sem atributos de compilador, sem `__attribute__`. |
| PT-H | Avisos recomendados: `-Wall -Wextra -Wformat=2`. Recomenda-se **não** usar `-Wpedantic` no caminho M0 em código que usa `INICIO` (o `main` gerado por macro pode gerar ruído em algumas combinações). |
| PT-I | Windows/MSVC: como a fonte já é `.c`, basta `/I` apontando para `include`; se o arquivo for tratado como C++ por algum projeto, force `/TC`. Nada de renomear arquivos (era o custo da extensão `.bg`, descartada em D-14). |

---

## 16. Compatibilidade e evolução

### 16.1 Versionamento

`BG_VERSAO_MAIOR`, `BG_VERSAO_MENOR`, `BG_VERSAO_CORRECAO` são definidos por
`<basicgol.h>`. A especificação segue versionamento semântico:

* **maior** — remove/renomeia palavra-chave ou muda semântica de construtor existente;
* **menor** — adiciona construtor, palavra-chave, aviso ou função de biblioteca;
* **correção** — corrige documentação/defeito sem mudar comportamento especificado.

Histórico: `1.0.0` (Fases 1–2: linguagem, cabeçalho, transpilador); `1.1.0` (Fase 4:
tipos com contrato explícito, `OU_EXCLUSIVO`/`LOGICO_DE`, `DIVIDE`/`RESTO`, conversões
verificadas, biblioteca de texto segura, `LEIA_INTEIRO`/`LEIA_REAL` e extensões
opcionais). A Fase 4 é aditiva, exceto pelo endurecimento de
`TEXTO_PARA_INTEIRO`/`TEXTO_PARA_REAL`, que passaram a recusar estouro de faixa
(D-27 em [04](04-restricoes-e-decisoes.md)).

### 16.2 Garantias

* **G-1** Todo programa válido em `1.x` continua válido e com o mesmo comportamento em
  qualquer `1.y` com `y > x`.
* **G-2** Nenhuma palavra-chave muda de significado dentro da mesma versão maior.
* **G-3** Identificadores de usuário com minúsculas (R-1) nunca se tornam palavras-chave
  — é isso que torna G-1 sustentável.
* **G-4** Remoção de construtor exige: marcação como obsoleto por pelo menos uma versão
  menor, aviso W-xx específico, e tabela de migração na documentação.
* **G-5** O significado de uma expansão de macro documentada em 03 só pode mudar se a
  mudança preservar o comportamento observável do programa.

### 16.3 Palavras reservadas para o futuro

Não são implementadas, mas **não podem** ser usadas como identificadores de usuário:
`PARA_CADA`, `TENTE`, `CAPTURE`, `FINALMENTE`, `CLASSE`, `MODULO`, `IMPORTE`, `GLOBAL`,
`LOCAL`, `REF`, `CONSTANTE_TEXTO`, `PONTEIRO`.

### 16.4 Guarda de versão

Para bibliotecas que exigem recursos recentes:

```c
#include <basicgol.h>
#if BG_VERSAO_MENOR < 3
#error "este modulo exige BASICGOL 1.3 ou superior"
#endif
```

---

## 17. Gramática (EBNF)

Notação: `{ x }` = zero ou mais; `[ x ]` = opcional; `"x"` = terminal;
`x | y` = alternativa. `[X]` marca construtor só no caminho M1.

```ebnf
arquivo        := { diretiva } { declaracao_global } { funcao } principal { funcao } ;
diretiva       := "#include" ... | "#define" ... ;          (* C nativo *)

principal      := "INICIO" { comando } "FIM" ;

funcao         := [ "ESTATICO" | "EXTERNO" ]
                  ( "FUNCAO" tipo | "PROCEDIMENTO" ) ident
                  "(" [ parametros ] ")" bloco_c ;
parametros     := parametro { "," parametro } ;
parametro      := tipo ident | tipo ident "[" "]" | tipo "*" ident ;

declaracao_global := declaracao_variavel | declaracao_vetor
                   | declaracao_registro | declaracao_enumeracao ;

declaracao_variavel := [ "CONSTANTE" | "ESTATICO" | "EXTERNO" ] tipo ident
                       [ "=" expressao ] ";" ;
declaracao_vetor    := "VETOR" "(" tipo "," ident "," expressao ")" ";" ;
declaracao_texto    := "TEXTO_FIXO" "(" ident "," expressao ")" ";" ;
declaracao_registro := "REGISTRO" "(" nome_tipo ")" { campo }
                       "FIM_REGISTRO" "(" nome_tipo ")" ";" ;
campo               := tipo ident ";" ;
declaracao_enumeracao := "ENUMERACAO" "(" nome_tipo ")"
                         enumerador { "," enumerador }
                         "FIM_ENUMERACAO" "(" nome_tipo ")" ";" ;
enumerador          := MAIUSCULO [ "=" expressao ] ;

comando        := declaracao_variavel | declaracao_vetor | declaracao_texto
                | declaracao_registro | declaracao_enumeracao
                | comando_se | comando_enquanto | comando_repita | comando_repita_sugar
                | comando_para | comando_para_sugar
                | comando_escolha | comando_escreva | comando_leia | comando_erro
                | "RETORNE" [ expressao ] ";" | "PARE" ";" | "PROSSIGA" ";"
                | expressao ";" | bloco_c ;

comando_se     := "SE" expressao "ENTAO" { comando }
                  { "SENAO_SE" expressao "ENTAO" { comando } }
                  [ "SENAO" { comando } ]
                  "FIM_SE" ;

comando_enquanto := "ENQUANTO" expressao "FACA" { comando } "FIM_ENQUANTO" ;

comando_repita := "REPITA" { comando } "ATE_QUE" "(" expressao ")" ";" ;

comando_repita_sugar := "REPITA" { comando } "ATE" expressao ";" ;             (* M1 *)

comando_para   := "PARA" "(" ident "," expressao "," expressao ")" bloco_c
                | "PARA_PASSO" "(" ident "," expressao "," expressao ","
                  expressao ")" bloco_c ;

comando_para_sugar := "PARA" ident "DE" expressao "ATE" expressao
                      [ "PASSO" expressao ] "FACA" { comando } "FIM_PARA" ;  (* M1 *)

comando_escolha := "ESCOLHA" "(" expressao ")"
                   { "CASO" expressao ":" { comando } }
                   [ "CONTRARIO" ":" { comando } ]
                   "FIM_ESCOLHA" ;

comando_escreva := ( "ESCREVA" | "ESCREVAL" | "ESCREVA_TEXTO"
                   | "ESCREVA_ERRO" ) "(" expressao ")" ";"
                 | "ESCREVA_LINHA" "(" ")" ";"
                 | "ESCREVA_FMT" "(" expressao { "," expressao } ")" ";" ;

comando_leia   := "LEIA" "(" lvalue ")" ";"
                | "LEIA_OK" "(" lvalue ")" ";"
                | "LEIA_TEXTO" "(" lvalue "," expressao ")" ";" ;

comando_erro   := "ERRO" "(" expressao ")" ";"
                | "AFIRME" "(" expressao "," expressao ")" ";" ;

lvalue         := ident | lvalue "[" expressao "]" | lvalue "." ident ;  (* tem endereco *)

bloco_c        := "{" { comando_c } "}" ;                   (* C nativo, P-03 *)

expressao      := expressao C válida ;                       (* P-09: gramática de C *)
tipo           := "INTEIRO" | "INTEIRO_CURTO" | "INTEIRO_LONGO"
                | "INTEIRO8" | "INTEIRO16" | "INTEIRO32" | "INTEIRO64"
                | "NATURAL" | "NATURAL8" | "NATURAL16" | "NATURAL32" | "NATURAL64"
                | "REAL" | "REAL_CURTO" | "REAL_LONGO"
                | "LOGICO" | "CARACTERE" | "TEXTO" | "VAZIO"
                | nome_tipo ;
nome_tipo      := PascalCase ;                                (* typedef de REGISTRO/ENUMERACAO *)
ident          := (ASCII, ao menos uma minúscula) ;
MAIUSCULO      := [A-Z_] { [A-Z0-9_] } ;
```

Note que `expressao` **não** tem produção própria: essa é a decisão P-09 e é o que
mantém a especificação curta e o mapeamento 1:1.

---

## 18. Exemplos

Programas completos em [`docs/exemplos/`](exemplos/) (13 programas no perfil M0 e 1 no perfil M1; saídas
verificadas em [`exemplos/README.md`](exemplos/README.md)):

| Arquivo | Demonstra |
|---|---|
| [`ola_mundo.c`](exemplos/ola_mundo.c) | mínimo: `INICIO`, `ESCREVAL`, `FIM` |
| [`media_notas.c`](exemplos/media_notas.c) | `FUNCAO`, `SE`/`SENAO_SE`, `ESCREVA_FMT` |
| [`tabuada.c`](exemplos/tabuada.c) | `PARA`, laços aninhados, `PARA_PASSO` |
| [`primos.c`](exemplos/primos.c) | `ENQUANTO`, `PARE`, `MOD`, funções auxiliares |
| [`vetores.c`](exemplos/vetores.c) | `VETOR`, `TAMANHO_VETOR`, passagem vetor+tamanho |
| [`cadastro.c`](exemplos/cadastro.c) | `REGISTRO`, `VETOR` de registros, busca |
| [`adivinha.c`](exemplos/adivinha.c) | `REPITA`/`ATE_QUE`, `SORTEIA`, `LEIA_OK`, `ESCOLHA` |
| [`contador_vogais.c`](exemplos/contador_vogais.c) | `TEXTO`/`TEXTO_FIXO`, `LEIA_TEXTO`, laço sobre caracteres |
| [`interop_c.c`](exemplos/interop_c.c) | BG e C no mesmo arquivo, `#include`, `struct`, ponteiros |
| [`fatorial_recursivo.c`](exemplos/fatorial_recursivo.c) | recursão, `RETORNE`, `INTEIRO64` |
| [`utilidades.c`](exemplos/utilidades.c) | catálogo: todos os tipos, matemática, texto, `ENUMERACAO`, `AFIRME`, `ESCREVA_ERRO` |
| [`tipos.c`](exemplos/tipos.c) | **Fase 4**: um valor de cada tipo, largura, faixa e precisão |
| [`expressoes.c`](exemplos/expressoes.c) | **Fase 4**: precedência, conversões, verdade, `DIVIDE`/`RESTO`, texto seguro |
| [`para_contado.c`](exemplos/para_contado.c) | forma açucarada do laço contado (**M1**) e sua equivalência em M0 |

Cada exemplo abre com um comentário que declara o perfil exigido (`M0` ou `M0+M1`) e o
comando de compilação.

---

## 19. Garantias e limites (resumo executivo)

**Garantido**

1. Um programa que usa apenas construtores **N** compila com um compilador C11, sem
   ferramenta própria, e produz o C descrito na tabela de mapeamento.
2. Todo construtor documentado tem expansão verificada (sonda, §04-6) e não depende de
   comportamento indefinido do pré-processador.
3. O vocabulário em português cobre fluxo de controle, tipos, declarações, E/S de
   console e erros fatais.
4. C está sempre disponível e se mistura sem cerimônia.
5. Toda conversão que pode falhar tem forma verificada que devolve `LOGICO`, e as únicas
   operações aritméticas com UB inevitável (divisão/resto por zero e `MIN/-1`) têm versão
   verificada (`DIVIDE`/`RESTO`) — [06](06-tipos-e-expressoes.md) §4 e §5.
6. O que a linguagem promete sobre estouro e ponto flutuante está escrito sem eufemismo
   em [06 §10](06-tipos-e-expressoes.md).

**Não garantido (por construção)**

1. Sintaxe que exija reordenar tokens (`PARA i DE 1 ATE 10`) fora do caminho M1.
2. Ponto e vírgula opcional, parênteses opcionais, indentação significativa.
3. Impressão formatada automática de vários valores em uma chamada.
4. Mensagens de compilação comparáveis às de C nativo no caminho M0.
5. Verificação de limites, tipos, inicialização ou estouro em tempo de compilação.
6. Qualquer coisa que dependa de inferência de tipos.

Esses limites não são falhas a corrigir: são o contrato da linguagem, e cada um tem uma
alternativa documentada (§[03](03-mapeamento-para-c.md), §[04](04-restricoes-e-decisoes.md)).
