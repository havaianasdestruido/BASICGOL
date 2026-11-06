# BASICGOL — Referência de Tipos e Expressões

> Documento **normativo** da Fase 4. A implementação dos helpers foi organizada na Fase 5; consulte [09 — Biblioteca padrão](09-biblioteca-padrao.md) para headers, linkagem e ownership. Este documento responde a três perguntas:
> *que tipo é este?*, *que operação posso fazer com ele?* e *o que acontece quando
> algo dá errado?* — sempre dizendo **o que é C** e **o que é BASICGOL**.
>
> Documentos irmãos: [01 — Especificação](01-especificacao.md) §6/§9,
> [02 — Referência](02-referencia.md), [03 — Mapeamento para C](03-mapeamento-para-c.md),
> [04 — Restrições e decisões](04-restricoes-e-decisoes.md),
> [05 — Critérios de aceitação](05-criterios-de-aceitacao.md).

**Regra da casa:** a linguagem não inventa um modelo de dados. Ela dá **nomes em
português** aos tipos de C e **verifica** os casos em que C deixa o programador no
terreno indefinido. Tudo o que não tem implementação definida e testada **não é
exposto** — está na §12 (limites conhecidos).

---

## 1. Resumo executivo

| Grupo | Tipos BASICGOL | Tipo C | Largura |
|---|---|---|---|
| lógico | `LOGICO` | `bool` (`_Bool`) | ≥ 1 byte |
| caractere | `CARACTERE` | `char` | exatamente 1 byte |
| inteiro com sinal | `INTEIRO_CURTO`, `INTEIRO`, `INTEIRO_LONGO` | `short`, `int`, `long` | a da plataforma |
| inteiro com sinal, largura exata | `INTEIRO8/16/32/64` | `int8_t` … `int64_t` | 8/16/32/64 bits |
| inteiro sem sinal | `NATURAL_CURTO`, `NATURAL`, `NATURAL_LONGO` | `unsigned short`, `unsigned`, `unsigned long` | a da plataforma |
| inteiro sem sinal, largura exata | `NATURAL8/16/32/64` | `uint8_t` … `uint64_t` | 8/16/32/64 bits |
| real | `REAL_CURTO`, `REAL`, `REAL_LONGO` | `float`, `double`, `long double` | a da plataforma |
| texto | `TEXTO` | `const char *` | ponteiro para bytes UTF-8 |
| texto mutável | `TEXTO_FIXO(nome, tam)` | `char nome[tam]` | buffer de `tam` bytes |
| vazio | `VAZIO` | `void` | ausência de valor |

Não existem: inteiro de precisão arbitrária, decimal exato (`decimal`, `currency`),
variante (`variant`), tipo dinâmico, `string` com tamanho gerenciado, vetor de tamanho
variável exposto como construtor, nem união de tipos. Quem precisa de algo assim usa
`malloc` e as estruturas de C — de forma visível no fonte (P-03).

---

## 2. Representações e mapeamento para C

### 2.1 Lógico

```c
LOGICO aprovado = VERDADEIRO;   /* bool aprovado = true; */
LOGICO faltou   = FALSO;        /* bool faltou   = false; */
```

* `LOGICO` **é** `bool` de `<stdbool.h>`. `VERDADEIRO` é `true` (= 1) e `FALSO` é
  `false` (= 0). Não existe `VERDADEIRO == -1` como em GW-BASIC.
* `sizeof(LOGICO) >= 1`; o valor exato é o da plataforma e **não** é contrato.
* Em expressão aritmética, `LOGICO` é promovido a `INTEIRO`: `VERDADEIRO + VERDADEIRO`
  vale `2`. Isso é C, e é a razão pela qual a linguagem recomenda comparar `LOGICO` com
  `LOGICO`, não somar booleanos.
* Qualquer **escalar** pode ser convertido para `LOGICO` (§4.2). Não há conversão
  implícita de texto para lógico por conteúdo: `"falso"` é um ponteiro não nulo, logo
  `VERDADEIRO`. Use `TEXTOS_IGUAIS` para comparar conteúdo.

### 2.2 Caractere

```c
CARACTERE letra = 'A';          /* char letra = 'A'; */
CARACTERE nova  = 'A' + 1;      /* promoção para int, converte de volta */
```

* `CARACTERE` **é** `char`, ocupa exatamente 1 byte e guarda **um** byte, não um
  caractere Unicode. Acentos em UTF-8 ocupam 2 bytes e não cabem em um `CARACTERE`.
* O **sinal** de `char` é definido pela implementação (pode ser `signed char` ou
  `unsigned char`). Por isso:
  * `CARACTERE` serve para letras ASCII, pontuação e `'\\n'`;
  * para armazenar bytes numéricos de 0 a 255 use `NATURAL8`;
  * para valores de −128 a 127 use `INTEIRO8`.
* Aritmética promove para `INTEIRO`: `'A' + 1 == 66`. Para voltar ao tipo caractere é
  preciso cast explícito (`(CARACTERE) (letra + 1)`).
* `CARACTERE` não é texto: `'A'` é um caractere, `"A"` é um `TEXTO`. Atribuir um ao
  outro é erro de compilação (teste `06_negativos/n12`).

### 2.3 Inteiros

| Palavra-chave | Tipo C | Largura garantida | Faixa (na prática) |
|---|---|---|---|
| `INTEIRO_CURTO` | `short` | ≥ 16 bits | −32 768 … 32 767 |
| `INTEIRO` | `int` | ≥ 16 bits | −2 147 483 648 … 2 147 483 647 |
| `INTEIRO_LONGO` | `long` | ≥ 32 bits | 64 bits em Linux/macOS, 32 em Windows |
| `INTEIRO8` | `int8_t` | exatamente 8 | −128 … 127 |
| `INTEIRO16` | `int16_t` | exatamente 16 | −32 768 … 32 767 |
| `INTEIRO32` | `int32_t` | exatamente 32 | −2 147 483 648 … 2 147 483 647 |
| `INTEIRO64` | `int64_t` | exatamente 64 | −9,22 × 10¹⁸ … 9,22 × 10¹⁸ |
| `NATURAL_CURTO` | `unsigned short` | ≥ 16 bits | 0 … 65 535 |
| `NATURAL` | `unsigned` | ≥ 16 bits | 0 … 4 294 967 295 |
| `NATURAL_LONGO` | `unsigned long` | ≥ 32 bits | 0 … 2⁶⁴−1 em Linux/macOS |
| `NATURAL8` … `NATURAL64` | `uint8_t` … `uint64_t` | exata | 0 … 2ᴺ−1 |

A coluna “na prática” **não é promessa** da linguagem: é o que se observa nas
plataformas suportadas. A promessa está na coluna “garantida”, e ela é a de C11. Para
saber o valor real em tempo de compilação use `BITS_DE(tipo)` e as constantes da §3; não
escreva `if (sizeof(int) == 4)` como se fosse portável.

`INTEIRO8`…`NATURAL64` são **obrigatórios** em C11 e têm largura exata — são o tipo
correto para serialização, máscaras de bits, CRC, dinheiro em centavos e qualquer
cálculo em que o estouro precisa ser previsível.

**Sem sinal vs. com sinal.** Misturar os dois em uma expressão converte o valor com sinal
para o tipo sem sinal (conversão módulo 2ᴺ, definida). É por isso que
`-1 < 1u` é **falso** em C — e em BASICGOL. A suíte fixa o comportamento em
`09_tipos_expr/t01` e `09_tipos_expr/t07`; o compilador avisa com `-Wsign-compare`
(que a suíte trata como erro nos próprios testes).

### 2.4 Reais

| Palavra-chave | Tipo C | Garantia | Observação |
|---|---|---|---|
| `REAL_CURTO` | `float` | ≥ 6 algarismos decimais significativos | IEEE-754 simples nas plataformas comuns |
| `REAL` | `double` | ≥ 10 algarismos decimais significativos | IEEE-754 dupla nas plataformas comuns |
| `REAL_LONGO` | `long double` | precisão ≥ `double` | 80 bits (x86), 128 bits (ARM), 64 bits (MSVC) |

* A linguagem **recomenda** IEC 60559 (IEEE-754) e testa o comportamento nos
  ambientes de referência, mas **não promete** IEEE-754: C11 não obriga. O que a
  linguagem promete é o que C promete — mais a documentação honesta da §10.
* `REAL` não é decimal: `0.1` não é representável exatamente, `0.1 + 0.2 != 0.3`.
  Compare com tolerância (`ABS(a - b) < EPSILON_REAL`) ou compare inteiros.
* Não há tipo `decimal`/`moeda`. Para dinheiro, use inteiros (centavos) ou uma
  biblioteca decimal de C, de forma explícita.
* `POTENCIA`, `RAIZ`, `PISO`, `TETO`, `ARREDONDA`, `ABS`, `MINIMO`, `MAXIMO` preservam
  o tipo real do argumento (via `_Generic`/`<math.h>`), e `DIVIDE` mantém a divisão real.

### 2.5 Texto

```c
TEXTO nome = "BASICGOL";        /* const char *nome = "BASICGOL"; */
TEXTO_FIXO(buf, 32);            /* char buf[32];                  */
```

* `TEXTO` é **`const char *`**: um ponteiro para bytes UTF-8 terminados por `'\0'`.
  Atribuir um literal é válido; **modificar** (`nome[0] = 'b'`) é erro de compilação,
  não aviso — o literal pode estar em memória somente-leitura.
* `TAMANHO_TEXTO(s)` conta **bytes**, não caracteres: `"ação"` tem 6 bytes e 4
  caracteres. Não há tipo “texto Unicode” em v1.0.
* Comparar com `==` compara **ponteiros**, não conteúdo. A única comparação correta é
  `TEXTOS_IGUAIS(a, b)` / `TEXTOS_DIFERENTES(a, b)` (ou `COMPARA_TEXTO`, §8).
* Não existe concatenação com `+`: em C isso é aritmética de ponteiros. Use
  `JUNTA_TEXTO`, `ACRESCENTA_TEXTO`, `COPIA_TEXTO`, `TRECHO_TEXTO` ou `FORMATA_TEXTO`.
* `TEXTO` não tem dono: apontar para um buffer local que já morreu é *dangling pointer*,
  exatamente como em C. `TEXTO_FIXO` dentro de uma função deixa de existir no retorno.
* `TEXTOS_IGUAIS`/`TEXTOS_DIFERENTES` exigem ponteiros válidos (não são `NULL`-safe,
  como `strcmp`). Para entradas que podem ser `NULL`, use `COMPARA_TEXTO`/`TEXTO_VAZIO`.

### 2.6 Vazio e declarações compostas

| Construtor | Expansão C | Uso |
|---|---|---|
| `VAZIO` | `void` | retorno de `PROCEDIMENTO`; `(VAZIO) expr` descarta valor |
| `VETOR(t, nome, n)` | `t nome[n]` | vetor de `n` elementos, índices **0 … n−1** |
| `TEXTO_FIXO(nome, n)` | `char nome[n]` | buffer de texto mutável |
| `TAMANHO_VETOR(v)` | `(sizeof(v) / sizeof((v)[0]))` | número de elementos; **só** para vetor real, nunca ponteiro |
| `TAMANHO_DE(t)` | `((size_t) sizeof(t))` | tamanho em bytes do tipo |
| `BITS_DE(t)` | `((size_t) (sizeof(t) * CHAR_BIT))` | tamanho em bits do tipo |

`TAMANHO_VETOR` é exatamente o idioma pedido por C-18: passe vetor **e** tamanho para a
função; dentro dela o vetor é ponteiro e `TAMANHO_VETOR` mentiria (aviso W-06).

### 2.7 Qualificadores

| Palavra | Expansão | Efeito |
|---|---|---|
| `CONSTANTE` | `static const` | valor imutável, com inicializador obrigatório |
| `ESTATICO` | `static` | símbolo privado do arquivo / variável persistente |
| `EXTERNO` | `extern` | declara símbolo definido em outra unidade de tradução |

`CONSTANTE` **não** é expressão constante para tamanho de vetor em C (`static const int`
não serve como `int v[k]`): use literal ou `#define` (C-23).

---

## 3. Constantes de faixa, precisão e tamanho

| Constante | Valor | Tipo |
|---|---|---|
| `MAXIMO_INTEIRO` / `MINIMO_INTEIRO` | `INT_MAX` / `INT_MIN` | `INTEIRO` |
| `MAXIMO_NATURAL` | `UINT_MAX` | `NATURAL` |
| `MAXIMO_REAL` | `DBL_MAX` | `REAL` |
| `EPSILON_REAL` | `DBL_EPSILON` (menor ε com `1.0 + ε != 1.0`) | `REAL` |

São macro-objeto: podem ser usadas em `_Static_assert`, em atribuição e em comparação.
Para os tipos de largura exata use os limites de `<stdint.h>` (`INT32_MAX` etc.) — eles
existem em C e não foram renomeados, para não criar duas verdades (teste
`09_tipos_expr/t14` prova que `MAXIMO_INTEIRO == INT_MAX`).

---

## 4. Conversões

Há **três classes** de conversão. A diferença entre elas é o que acontece quando o valor
não cabe.

### 4.1 Conversões de C (declaradas, não redefinidas)

| Situação | Regra | Segurança |
|---|---|---|
| promoção de inteiro | `CARACTERE`, `LOGICO`, `INTEIRO_CURTO` → `INTEIRO` | sempre exata |
| conversão usual em operador | o tipo comum de C (`int` < `long` < `long long` < `float` < `double` < `long double`) | pode perder precisão em real |
| alargamento com sinal | `INTEIRO8` → `INTEIRO32` → `INTEIRO64` | sempre exata |
| estreitamento inteiro | `INTEIRO64` → `INTEIRO8` | **definido** para tipo sem sinal (módulo 2ᴺ); **definido pela implementação** para tipo com sinal, se o valor não couber |
| inteiro → real | `INTEIRO64` → `REAL` | pode arredondar acima de 2⁵³ |
| real → inteiro (cast) | trunca em direção a zero | **indefinido** se o valor não cabe ou é NaN |
| escalar → `LOGICO` | 0 → `FALSO`; qualquer outro → `VERDADEIRO` | sempre definida |
| `LOGICO` → inteiro | `FALSO` → 0, `VERDADEIRO` → 1 | sempre exata |
| ponteiro ↔ inteiro | só com cast explícito; resultado depende da plataforma | **não recomendado** |

O cast escreve-se como em C, porque a macro do tipo já é o tipo:

```c
INTEIRO n = (INTEIRO) 3.9;      /* 3   */
REAL    r = (REAL) 7 / 2;       /* 3.5 */
NATURAL8 b = (NATURAL8) 300;    /* 44  (300 mod 256) */
```

### 4.2 Conversão para `LOGICO` (valor de verdade)

```c
LOGICO_DE(x)                    /* (bool)((x) != 0) */
```

| `x` | `LOGICO_DE(x)` | Observação |
|---|---|---|
| `0`, `0.0`, `'\0'`, `FALSO`, ponteiro nulo | `FALSO` | |
| qualquer outro inteiro, inclusive negativo | `VERDADEIRO` | `-1` é verdadeiro |
| `NAN` | `VERDADEIRO` | NaN é diferente de zero |
| ponteiro não nulo, inclusive `""` | `VERDADEIRO` | `""` **não** é falso |

`LOGICO_DE` normaliza para 0/1 e é o que `SE`, `ENQUANTO` e `ATE_QUE` efetivamente fazem
com a condição. Escrever `SE x ENTAO` com `x` não lógico é permitido (C), mas a
especificação recomenda `SE LOGICO_DE(x) ENTAO` ou declarar a variável como `LOGICO`.

### 4.3 Conversões verificadas (biblioteca)

Estas **devolvem `LOGICO`** e nunca escrevem no destino quando falham:

| Função | Origem → destino | Regras |
|---|---|---|
| `TEXTO_PARA_INTEIRO(s, &n)` | texto → `INTEIRO` | aceita só o texto inteiro; espaços finais tolerados; recusa sobra, vazio, `ERANGE` e fora de `INT_MIN..INT_MAX` |
| `TEXTO_PARA_NATURAL(s, &u)` | texto → `NATURAL` | idem; recusa `-` (senão `-1` viraria `ULONG_MAX`); recusa > `UINT_MAX` |
| `TEXTO_PARA_REAL(s, &r)` | texto → `REAL` | idem; locale **C** (ponto decimal, não vírgula); recusa estouro e underflow |
| `REAL_PARA_INTEIRO(x, &n)` | real → `INTEIRO` | trunca em direção a zero; recusa NaN, infinito e fora de faixa (o cast seria UB) |

```c
INTEIRO n = 0;
SE TEXTO_PARA_INTEIRO("42", &n) ENTAO
    ESCREVAL(n);                       /* 42 */
SENAO
    ERRO("entrada invalida");
FIM_SE
```

Detalhes contratuais:

* o destino só é alterado em caso de sucesso (como `scanf`);
* as funções de texto usam `strtol`/`strtoul`/`strtod`, mas preservam `errno`; falhas
  são devolvidas por `LOGICO` (não há código global de erro),
* `TEXTO_PARA_REAL("1e999")` devolve `FALSO` (estouro) e `"1e-999"` também (underflow);
* `TEXTO_PARA_REAL` aceita o que o `strtod` aceita: `"inf"`, `"-inf"` e `"nan"` são
  **valores representáveis**, não erros de faixa, e são aceitos (com `NAN`, `x == x` é
  falso). Se isso não serve, valide a faixa depois da conversão;
* não há conversão implícita de texto para número, nem de número para texto por
  atribuição. Atribuir `"42"` a `INTEIRO` é erro de compilação (teste
  `06_negativos/n13`); para obter texto formatado use `FORMATA_TEXTO` ou `ESCREVA_FMT`.
* o destino tem tipo fixo e **a variável precisa ser daquele tipo**: `&n` em
  `TEXTO_PARA_NATURAL` tem de ser um `NATURAL` (não `INTEIRO`, não `NATURAL8`);
  `TEXTO_PARA_REAL` só escreve em `REAL`; `REAL_PARA_INTEIRO` só em `INTEIRO`. Passar o
  ponteiro de outro tipo é erro de compilação (`-Werror=pointer-sign` /
  `-Wincompatible-pointer-types`), e é assim de propósito: converter `-1` para natural
  ou `1e300` para inteiro em silêncio é exatamente o que a Fase 4 removeu. Para outro
  tipo, converta em dois passos explícitos:

```c
NATURAL u = 0;
INTEIRO n = 0;
SE TEXTO_PARA_NATURAL("42", &u) ENTAO      /* u: 42 */
    n = (INTEIRO) u;                       /* estreitamento explicito */
FIM_SE
```

### 4.4 Tipo do resultado de uma conversão verificada

As funções verificadas devolvem `LOGICO`; o valor convertido sai pelo ponteiro. Isso é
deliberado: em C não existe retorno múltiplo, e usar um “valor mágico” para erro
(como `-1`) confundiria com um resultado válido.

---

## 5. Operadores

### 5.1 Tabela mestra

| Categoria | BASICGOL | C | Resultado |
|---|---|---|---|
| atribuição | `=`, `+=`, `-=`, `*=`, `/=`, `%=` | idem | tipo do destino |
| incremento | `++x`, `x++`, `--x`, `x--` | idem | tipo do operando |
| aritmética | `+`, `-`, `*`, `/` | idem | tipo comum dos operandos |
| resto | `a MOD b` ou `a % b` | `a % b` | inteiro; sinal do dividendo |
| divisão/resto verificados | `DIVIDE(a, b)`, `RESTO(a, b)` | — | tipo promovido; erro se divisor 0 ou quociente assinado não representável |
| comparação | `==`, `!=`, `<`, `<=`, `>`, `>=` | idem | `LOGICO` (na prática `int` 0/1) |
| comparação prefixada | `IGUAL(a, b)`, `DIFERENTE(a, b)` | `((a) == (b))`, `((a) != (b))` | `LOGICO` |
| comparação de texto | `TEXTOS_IGUAIS(a, b)`, `TEXTOS_DIFERENTES(a, b)` | `strcmp(a,b) == 0` | `LOGICO` |
| lógica | `a E b`, `a OU b`, `NAO a` | `a && b`, `a \|\| b`, `!a` | `int` 0/1 |
| lógica exclusiva | `OU_EXCLUSIVO(a, b)` | `(bool) (((a) != 0) != ((b) != 0))` | `LOGICO` |
| normalização | `LOGICO_DE(x)` | `(bool) ((x) != 0)` | `LOGICO` |
| acesso | `.`, `[ ]`, `( )` | idem | campo / elemento / retorno |
| ponteiros | `&x`, `*p`, `p->campo` | idem | C nativo |
| bit a bit | `& \| ^ ~ << >>` | idem | C nativo (não há forma em português) |
| condicional | `c ? a : b` | idem | tipo comum de `a` e `b` |
| tamanho | `sizeof`, `TAMANHO_DE(t)`, `BITS_DE(t)` | idem | `size_t` (`NATURAL`) |

`MOD` e `%` são a mesma coisa. `MOD` só existe para inteiros; `%` com real é erro de
compilação (teste `06_negativos/n14`), porque C não tem resto real. Para reais use
`x - PISO(x / y) * y` ou `fmod` de `<math.h>` — de forma explícita.

### 5.2 Atribuição

* `=` **atribui**; `==` **compara** (D-10). `SE x = 5 ENTAO` é o erro clássico: o
  compilador avisa e o transpilador emite W-01.
* Atribuição composta (`+=`, `-=`, `*=`, `/=`, `%=`) segue C: calcula no tipo comum e
  converte para o tipo do destino.
* `MOD=` **não existe**: `%` é um token único e o pré-processador não o fabrica a partir
  de `MOD` + `=` (C-01). Escreva `x = x MOD 2` ou `x %= 2`.
* Atribuir entre tipos diferentes converte, com as regras da §4.1. O compilador avisa
  (`-Wconversion` não está ligado por padrão; `-Wall` pega os casos óbvios).
* Uma atribuição é uma **expressão** com valor (o valor atribuído), mas usá-la dentro de
  outra expressão é desencorajado pela especificação (W-01).

### 5.3 Comparação

| Comparação | Semântica |
|---|---|
| inteiros de mesmo tipo | comparação numérica |
| inteiros de tipos diferentes | conversões usuais de C; misturar sinal e natural pode surpreender (`-1 < 1u` é falso) |
| reais | comparação numérica; `NAN` é diferente de tudo, inclusive de si mesmo; `<`, `<=`, `>`, `>=` com `NAN` são falsos |
| reais e inteiros | o inteiro vira real (pode perder precisão acima de 2⁵³) |
| `LOGICO` | compara 0/1 |
| `TEXTO` com `==` | compara **endereços**; nunca use para conteúdo |
| `TEXTO` com `TEXTOS_IGUAIS` | compara bytes até `'\0'`, sensível a maiúsculas e a acentos |
| `CARACTERE` | compara códigos de byte |
| ordenação de texto | `COMPARA_TEXTO(a, b)` devolve −1, 0 ou +1 |

### 5.4 Aritmética inteira

* `/` entre inteiros é **divisão inteira**, truncada em direção a zero:
  `7 / 2 == 3`, `-7 / 2 == -3`. Em muitos dialetos BASIC `/` produz real; em BASICGOL o
  tipo dos operandos decide (P-09). Para resultado real, converta antes:
  `(REAL) a / b`.
* `MOD` acompanha o sinal do **dividendo**: `-7 MOD 2 == -1`, `7 MOD -2 == 1`.
* Estouro em tipo **com sinal** é comportamento **indefinido** em C — a linguagem não
  promete “dar a volta”. Use `INTEIRO64`, escolha o tipo pela faixa (§2.3) ou use
  `SOMA_SEGURA`/`MULTIPLICA_SEGURA` (§11).
* Estouro em tipo **sem sinal** é definido: aritmética módulo 2ᴺ
  (`(NATURAL8) 255 + 1 == 0`).
* `INT_MIN / -1` e `INT_MIN % -1` causam UB se o quociente não couber no tipo (caso comum
  em representação de complemento de dois); `DIVIDE`/`RESTO` recusam qualquer quociente
  não representável antes da operação.
* Divisão ou resto por zero inteiro é **indefinido** em C. `DIVIDE`/`RESTO` recusam com
  erro; `/` e `MOD` crus continuam sendo UB, como em C.

### 5.5 Aritmética real

* `+ - * /` seguem C/IEC 60559 nas plataformas comuns: `/ 0.0` produz infinito ou NaN,
  e não erro. **A biblioteca escolheu diferente**: `DIVIDE(a, 0.0)` **recusa** o divisor
  zero com erro, para que o mesmo construto tenha a mesma política em inteiro e real.
  Para obter infinito/NaN deliberadamente, use `/` cru e documente.
* `0.1 + 0.2 != 0.3`. Comparação exata de reais quase sempre está errada; use tolerância.
* Não há `^` para potência (em C é XOR). Use `POTENCIA(base, expoente)` (`pow`, exige `-lm`).
* `ARREDONDA(x)` arredonda para o inteiro mais próximo e devolve `INTEIRO64`
  (`long long`, via `llround`); se o valor não couber em `long long`, o resultado não é
  confiável. Use `PISO`/`TETO`
  (que devolvem real) quando a faixa for grande.

### 5.6 Lógica e curto-circuito

* `E` é `&&`, `OU` é `||`, `NAO` é `!`. `E`/`OU` têm **curto-circuito garantido** pelo
  padrão C: o lado direito não é avaliado quando o esquerdo decide.
* `E`/`OU` devolvem `int` 0/1 (não `bool`); `LOGICO_DE` e `OU_EXCLUSIVO` devolvem `LOGICO`
  (`bool`), também 0/1.
* A normalização é `(bool) ((x) != 0)`, e **não** `!!(x)`: o resultado é o mesmo
  (inclusive para `NAN`, que é “diferente de zero”), mas `!!` sobre um literal real
  dispara `-Wliteral-conversion` no clang — e `LOGICO_DE(0.5)` é caso de uso documentado.
  O `!` de C continua à disposição (`NAO x`): com literal real, ele avisa no clang.
* Precedência: `NAO` liga mais forte que `E`, que liga mais forte que `OU`
  (`NAO a E b` = `(!a) && b`). Misturar os três sem parênteses é a recomendação R-6
  justamente porque é onde se erra.
* `OU_EXCLUSIVO(a, b)` normaliza cada operando (`!!`) e compara: é XOR **lógico**, não
  bit a bit. Para XOR bit a bit use `^` (C nativo).

---

## 6. Precedência (idêntica à de C)

Do mais forte para o mais fraco. Não há tabela própria na linguagem: esta é a de C, e é
por isso que ela vale (P-09).

| Nível | Operadores | Associatividade |
|---|---|---|
| 1 | `( )` `[ ]` `.` `->` chamada, índice, membro | esquerda |
| 2 | `++` `--` pós-fixos | esquerda |
| 3 | `NAO` (`!`) `~` `+` `-` unários, `*` indireção, `&` endereço, `sizeof`, casts | direita |
| 4 | `*` `/` `MOD` (`%`) | esquerda |
| 5 | `+` `-` | esquerda |
| 6 | `<<` `>>` | esquerda |
| 7 | `<` `<=` `>` `>=` | esquerda |
| 8 | `==` `!=` | esquerda |
| 9 | `&` (bit a bit) | esquerda |
| 10 | `^` (bit a bit) | esquerda |
| 11 | `\|` (bit a bit) | esquerda |
| 12 | `E` (`&&`) | esquerda |
| 13 | `OU` (`\|\|`) | esquerda |
| 14 | `? :` | direita |
| 15 | `=` `+=` `-=` `*=` `/=` `%=` | direita |
| 16 | `,` | esquerda |

Exemplos verificados pela suíte (`09_tipos_expr/t05`) e pelo exemplo `expressoes.c`:

```c
2 + 3 * 4          /* 14 */      7 MOD 2 + 1        /* 2  */
(2 + 3) * 4        /* 20 */      2 + 3 < 6          /* 1  */
10 - 2 - 3         /* 5  */      (1 E 0) OU 1       /* 1  */
100 / 10 / 2       /* 5  */      NAO 0 E 0          /* 0  */
-7 / 2             /* -3 */      (INTEIRO) 2.9 + 1  /* 3  */
```

---

## 7. Condições e valor de verdade

`SE`, `ENQUANTO` e `ATE_QUE` aceitam qualquer expressão escalar, como C. A condição é
verdadeira quando o valor é diferente de zero (ou ponteiro não nulo). Recomendações
normativas:

1. Declare a variável de condição como `LOGICO` quando ela representa uma decisão.
2. Ao converter de outro tipo, use `LOGICO_DE(x)` — o nome diz o que está acontecendo.
3. Nunca use `SE texto ENTAO` esperando testar conteúdo: ponteiro não nulo é sempre
   verdadeiro. Use `TEXTO_VAZIO(s)` ou `TEXTOS_IGUAIS`.
4. Não compare real com `==`, exceto com 0.0 quando o valor é exatamente atribuído.

O transpilador M1 emite W-03 para condição literal (`SE 1 ENTAO`) e W-01 para
atribuição em condição.

---

## 8. Texto: contrato da biblioteca

As funções de texto são helpers C compilados em `src/biblioteca/texto.c` e os headers
públicos estão em `include/basicgol/texto.h`. Elas recebem o tamanho do destino e
**reportam truncamento por valor de retorno**. Regra de ouro (RB-3): *truncamento nunca é
silencioso*; em caso de falha, `dest[0] == '\0'` quando o destino é utilizável.

| Forma | Devolve | Contrato |
|---|---|---|
| `TAMANHO_TEXTO(s)` | `NATURAL` | `strlen`; `NULL` conta como 0 |
| `COPIA_TEXTO(d, t, s)` | `LOGICO` | copia `s` para `d`; `false` se não coube (limpa `d`) |
| `JUNTA_TEXTO(d, t, a, b)` | `LOGICO` | `a` + `b` em `d`; `false` se não coube (limpa `d`) |
| `ACRESCENTA_TEXTO(d, t, s)` | `LOGICO` | acrescenta ao fim do que já está em `d` (substitui `strncat`) |
| `TRECHO_TEXTO(d, t, s, i, n)` | `LOGICO` | copia até `n` bytes a partir de `i`; `i` além do fim = vazio; `false` se não coube |
| `COMPARA_TEXTO(a, b)` | `INTEIRO` | −1/0/+1; `NULL` conta como `""` |
| `TEXTO_VAZIO(s)` | `LOGICO` | verdadeiro para `""` e `NULL` |
| `FORMATA_TEXTO(d, t, fmt, ...)` | `LOGICO` | `snprintf` verificável; `false` se truncou (limpa `d`) |
| `TEXTOS_IGUAIS(a, b)` | `LOGICO` | `strcmp`; exige ponteiros válidos |
| `TEXTOS_DIFERENTES(a, b)` | `LOGICO` | idem, negado |

Sobreposição e `NULL`:

* origem `NULL` é texto vazio em toda a família (`COPIA_TEXTO`, `JUNTA_TEXTO`,
  `ACRESCENTA_TEXTO`, `TRECHO_TEXTO`, `COMPARA_TEXTO`, `TAMANHO_TEXTO`) — nunca
  desreferenciada. `TEXTOS_IGUAIS`/`TEXTOS_DIFERENTES` são `strcmp` cru e exigem
  ponteiros válidos (é a forma curta, de uma linha).
* **origem e destino podem se sobrepor**: cópia, acréscimo e trecho usam `memmove`;
  `JUNTA_TEXTO` monta a saída em temporário antes de copiar. Assim, por exemplo,
  `TRECHO_TEXTO(buf, sizeof buf, buf, 2, 3)` e `JUNTA_TEXTO(buf, sizeof buf, buf, buf+1)`
  não destroem a origem durante a operação. `JUNTA_TEXTO` também pode falhar se não
  conseguir alocar esse temporário; nesse caso devolve `FALSO` e limpa o destino.
* falha **limpa** o destino (`d[0] = '\0'`): depois de `ACRESCENTA_TEXTO(buf, 6, buf)`
  que não coube, `TAMANHO_TEXTO(buf)` é 0. Nunca sobra conteúdo parcial.

Limites conhecidos:

* `FORMATA_TEXTO` **não** tem o formato conferido pelo compilador: para isso a função
  precisaria de `__attribute__((format))`, que é extensão. Um formato inválido é erro do
  programador, em tempo de execução.
* `FORMATA_TEXTO` usa o formato de `printf`; `%s` exige `char *` válido.
* `TAMANHO_TEXTO` conta bytes; para contar caracteres UTF-8 seria preciso decodificar
  (fora do escopo da v1.0, D-20).
* Não há função de busca/substituição de substring. `strstr` é C e continua disponível.

---

## 9. Entrada numérica e seus erros

| Forma | Como lê | Erro de entrada |
|---|---|---|
| `LEIA(x)` | `scanf` do próximo token | o token inválido **fica** no `stdin`; a próxima leitura pode falhar em cascata |
| `LEIA_OK(x)` | idem, devolvendo `LOGICO` | idem |
| `LEIA_INTEIRO(x)` | lê **uma linha** e converte o texto inteiro | a linha inteira é consumida; a próxima leitura começa na linha seguinte |
| `LEIA_REAL(x)` | idem, com `TEXTO_PARA_REAL` | idem |
| `LEIA_TEXTO(buf, t)` | lê uma linha, consome `\n` pendente | `false` se não coube (drena o resto da linha) |

`LEIA_INTEIRO`/`LEIA_REAL` são a resposta a “entrada numérica inválida”:

```c
INTEIRO idade = 0;
SE LEIA_INTEIRO(idade) ENTAO
    ESCREVAL(idade);
SENAO
    ESCREVAL("digite um numero inteiro");
FIM_SE
```

* O buffer interno é de 64 bytes (`BG_TAMANHO_LINHA_NUMERO`). Linha maior que isso é
  drenada e reportada como falha — não há truncamento silencioso.
* Linha vazia, texto com sobra (`"42x"`), estouro de faixa e `2,5` (locale C) são falhas.
* O destino não é alterado quando a leitura falha.
* Como toda leitura por linha, consome exatamente uma linha: não dá para ler “dois
  números na mesma linha” com duas chamadas.

### 9.1 `LEIA` e o tipo do destino

`LEIA(x)`/`LEIA_OK(x)` escolhem o formato de `scanf` por `_Generic(&x)` — é isso que
permite ler um `NATURAL8` sem gravar por cima de um byte:

| Tipo do destino | Formato | Observação |
|---|---|---|
| `INTEIRO`, `INTEIRO_LONGO` | `%d`, `%ld` | — |
| `INTEIRO8`/`INTEIRO16`, `INTEIRO64` | `%hhd`/`%hd`, `%lld` | `INTEIRO64` é `long long` em todas as plataformas da linguagem |
| `NATURAL`, `NATURAL_LONGO` | `%u`, `%lu` | — |
| `NATURAL8`, `NATURAL16`, `NATURAL_CURTO`, `NATURAL64` | `%hhu`, `%hu`, `%hu`, `%llu` | valor negativo é aceito pelo `scanf` e convertido módulo 2^n (comportamento de C, não da linguagem) |
| `REAL_CURTO`, `REAL`, `REAL_LONGO` | `%f`, `%lf`, `%Lf` | `1e999` vira infinito: sem verificação de faixa (use `LEIA_REAL`) |
| `CARACTERE` | `" %c"` | pula espaço em branco antes do caractere |
| `LOGICO`, `TEXTO`, `TEXTO_FIXO` | — | **erro de compilação** (o `_Generic` do formato não tem `default`, C11 6.5.1.1p3 — desde a Fase 6 é erro, não aviso de `-Wformat`): leia um `INTEIRO` e compare, ou use `LEIA_TEXTO` |

O caso `LOGICO` é recusado de propósito: `scanf` não tem conversão para `_Bool`, e
`0/1`, `FALSO/VERDADEIRO`, `V/F` são todos razoáveis — escolher um seria inventar
semântica. Teste: `06_negativos/n16_leia_logico`.

---

## 10. Estouro, faixas e ponto flutuante — o que é verdade

Esta seção existe porque a alternativa é repetir frases otimistas que C não sustenta.

### 10.1 Inteiros com sinal

| Situação | Comportamento | Como evitar |
|---|---|---|
| soma/subtração/multiplicação estoura | **UB** (C11 6.5p5) | tipo mais largo (`INTEIRO64`), checagem antes, ou `SOMA_SEGURA`/`MULTIPLICA_SEGURA` (§11) |
| Quociente assinado não representável (em geral `MIN / -1`) | **UB** | `DIVIDE`/`RESTO` recusam com erro |
| divisão/resto por 0 | **UB** | `DIVIDE`/`RESTO` recusam com erro |
| `ABS(MINIMO_INTEIRO)` quando a magnitude não cabe | erro fatal documentado | compare antes, use real ou uma faixa/representação maior |
| conversão real→inteiro fora de faixa | **UB** | `REAL_PARA_INTEIRO` devolve `FALSO` |
| deslocamento negativo ou ≥ largura | **UB** | use `NATURAL` e valide o valor |
| conversão para tipo menor com sinal | definido pela implementação | use tipo sem sinal (módulo) ou valide |

A linguagem **não** promete aritmética com “wrap” para tipo com sinal. Programas
didáticos não devem depender de estouro (PT-3); quando o exemplo precisa mostrar o
fenômeno, ele usa tipo sem sinal ou `SOMA_SEGURA`.

### 10.2 Inteiros sem sinal

Aritmética módulo 2ᴺ, **definida**. `0u - 1u == MAXIMO_NATURAL`. É a escolha certa para
contadores que dão a volta, máscaras e *hashes* — e a escolha errada para valores que
podem ficar negativos.

### 10.3 Ponto flutuante

* `REAL` é binário. Valores decimais como `0.1`, `0.2`, `0.3`, `1.1` não têm
  representação exata; a soma acumula erro. `0.1 + 0.2 == 0.3` é **falso** em toda
  plataforma IEC 60559 (teste `09_tipos_expr/t02`).
* Comparação com tolerância: `ABS(a - b) < EPSILON_REAL` é o teste mais simples; para
  séries longas use tolerância relativa.
* `NAN` propaga: qualquer operação com NaN dá NaN, e toda comparação com NaN é falsa
  (inclusive `NAN == NAN`). Use `isnan(x)` para detectar.
* `INFINITY` resulta de estouro e de divisão por zero real (via `/`). `DIVIDE` recusa o
  divisor zero real, então infinito só aparece por estouro ou por `/` explícito.
* `printf("%g")` mostra 6 algarismos significativos — o que é **menos** que a precisão
  de `double`. Para ver o valor completo use `ESCREVA_FMT("%.17g", x)`; o padrão é
  escolha didática, não perda de dado (o valor na memória está intacto).
* BASICGOL não chama `setlocale` (D-17); por padrão o processo começa na locale C e usa `.`. Se a aplicação alterar a locale, `strtod` segue a configuração corrente.
  Se o programa chamar `setlocale`, `%f`/`%g` passam a usar a vírgula — decisão do
  programa, visível no fonte.
* `REAL_LONGO` tem precisão diferente por plataforma (80/128/64 bits). Não imprima a
  largura em teste dourado; compare (`sizeof(REAL_LONGO) >= sizeof(REAL)`).
* Conversão de real para inteiro com cast é UB fora de faixa; `REAL_PARA_INTEIRO` é a
  versão verificada. `ARREDONDA` devolve `INTEIRO_LONGO` e usa `llround`, que tem o
  mesmo limite na prática.

### 10.4 `NATURAL` e `size_t`

`TAMANHO_TEXTO`, `TAMANHO_VETOR`, `TAMANHO_DE` e `BITS_DE` devolvem `size_t`
(`NATURAL` na prática). Comparar com `INTEIRO` dispara `-Wsign-compare` — o que é bom:
ensina o idioma correto (D-20):

```c
NATURAL n = TAMANHO_TEXTO(s);
NATURAL i = 0;
ENQUANTO i < n FACA
    ...
FIM_ENQUANTO
```

---

## 11. Extensões de compilador (opcionais e isoladas)

Toda a linguagem é C11 estrito, com **uma** exceção: a seção `2b` do cabeçalho.

```c
#define BG_USAR_EXTENSOES      /* antes de #include <basicgol.h> */
#include <basicgol.h>

INTEIRO r = 0;
SE SOMA_SEGURA(a, b, &r) ENTAO
    ESCREVAL(r);
SENAO
    ESCREVAL("estourou");
FIM_SE
```

| Item | Regra |
|---|---|
| Onde vive | só no bloco `#ifdef BG_USAR_EXTENSOES` de `basicgol.h`; fora dele, nada depende de extensão |
| Como é implementada | `__builtin_add_overflow`, `__builtin_sub_overflow`, `__builtin_mul_overflow` (GCC/Clang ≥ 5) |
| Que tipos cobre | apenas `INTEIRO` (int) — o caso didático |
| Resultado | `LOGICO`: `VERDADEIRO` = operação exata; `FALSO` = estourou e `*resultado` não deve ser usado |
| Sem suporte | `#error` claro em vez de emulação silenciosa |
| Sem a macro | `BG_TEM_EXTENSOES`, `SOMA_SEGURA`, `SUBTRAI_SEGURA` e `MULTIPLICA_SEGURA` **não existem** |
| Portabilidade | desligada, o cabeçalho é C11 estrito e compila com `-Wpedantic` |
| Teste | `09_tipos_expr/t13` (ligada) e a checagem `01_sanidade` (desligada por padrão) |

A regra de projeto é: **extensão de compilador não pode vazar**. Quem não define a macro
não vê nenhum símbolo, nenhum aviso e nenhuma mudança de comportamento (D-22).

---

## 12. Limites conhecidos (por decisão, não por esquecimento)

| Limite | Motivo | Alternativa |
|---|---|---|
| Sem tipo decimal/moeda | C não tem; emular com `double` seria mentir | inteiro em centavos, ou biblioteca decimal explícita |
| `CARACTERE` é 1 byte | é o `char` de C; UTF-8 usa 2–4 bytes por acento | `TEXTO` para texto; `NATURAL8` para bytes |
| `TAMANHO_TEXTO` conta bytes | `strlen`; contar caracteres exigiria decodificação UTF-8 | percorra os bytes ou use biblioteca externa |
| Sem texto de tamanho dinâmico | exigiria alocação e dono; a linguagem não esconde memória | `malloc` (C) ou `TEXTO_FIXO` com limite |
| Sem `+` de texto | em C é aritmética de ponteiros | `JUNTA_TEXTO`/`ACRESCENTA_TEXTO`/`FORMATA_TEXTO` |
| Sem comparação de texto com `==` | compara ponteiros | `TEXTOS_IGUAIS`/`COMPARA_TEXTO` |
| `MOD` só em inteiros | C não tem resto real | `fmod` (C) ou `x - PISO(x/y)*y` |
| Sem `**`/`^` de potência | `^` é XOR em C; operador novo exigiria reescrever expressões (C-01) | `POTENCIA` |
| Estouro com sinal não é detectado em `+ - *` | detectar exigiria código gerado, não macro | tipos largos ou `SOMA_SEGURA` (opcional) |
| `FORMATA_TEXTO` sem checagem de formato | exigiria atributo de compilador | `ESCREVA_FMT` (o compilador confere o formato literal) |
| `LEIA` pode dessincronizar o `stdin` | é `scanf`, por definição | `LEIA_INTEIRO`/`LEIA_REAL`/`LEIA_TEXTO` |
| `TEXTO` não é dono da memória | é ponteiro | `TEXTO_FIXO`, `malloc`, ou o literal |
| Sem vetor de tamanho variável exposto | VLA é frágil (C-23) | tamanho fixo ou `malloc` |

---

## 13. Diferenças semânticas em relação a BASIC e a Portugol

A comparação abaixo é entre **BASICGOL** e os dialetos mais usados no ensino:
BASIC clássico (GW-BASIC/QBasic) e Portugol (VisuAlg e Portugol Studio). Dialetos
variam entre si; onde variam, a célula descreve o comportamento **mais comum** e a
coluna BASICGOL é a normativa. Nada aqui é “melhor”: é diferente, e o aluno precisa
saber por quê.

### 13.1 Sintaxe de atribuição e comparação

| Tema | BASIC clássico | Portugol | BASICGOL |
|---|---|---|---|
| atribuição | `LET x = 1` / `x = 1` | `x <- 1` | `x = 1` (C) |
| igualdade | `=` | `=` | `==` (ou `IGUAL(a,b)`) |
| diferença | `<>` | `<>` ou `!=` | `!=` (ou `DIFERENTE(a,b)`) |

**Consequência:** em BASICGOL `=` **não** compara. `SE x = 5 ENTAO` é atribuição dentro
da condição; o compilador avisa e o transpilador emite W-01. Quem vem de BASIC/Portugol
deve escrever `==` ou `IGUAL(x, 5)`.

### 13.2 Tipos e tipagem

| Tema | BASIC clássico | Portugol | BASICGOL |
|---|---|---|---|
| declaração | implícita, com sufixo (`a$`, `n%`, `r!`) | explícita (`inteiro n`) | explícita e obrigatória (`INTEIRO n`) |
| tipagem | dinâmica/frouxa | estática | estática, sem inferência (P-08) |
| `inteiro` | depende do dialeto (16/32 bits) | normalmente 32 bits fixos | `INTEIRO` = `int` da plataforma; `INTEIRO32` = 32 exatos |
| `real` | precisão simples em muitos dialetos | `double` | `REAL` = `double`; `REAL_CURTO`/`REAL_LONGO` explícitos |
| texto | `STRING` mutável, tamanho fixo em alguns dialetos | `cadeia` | `TEXTO` = `const char *` imutável; `TEXTO_FIXO` para mutável |
| caractere | `CHR$`/`ASC` para conversão | `caractere` | `CARACTERE` = 1 byte; aritmética promove a `INTEIRO` |

**Consequência:** não há conversão automática de texto para número nem vice-versa.
`TEXTO_PARA_INTEIRO` e `FORMATA_TEXTO` tornam a conversão explícita e verificável.

### 13.3 Aritmética

| Tema | BASIC clássico | Portugol | BASICGOL |
|---|---|---|---|
| `/` entre inteiros | divisão **real** | divisão real; divisão inteira por operador próprio no dialeto | divisão **inteira** (C); use `(REAL) a / b` para real |
| divisão inteira | `\` | `\` ou `div` conforme o dialeto | `/` entre inteiros, ou `DIVIDE` |
| resto | `MOD` | `%` ou `mod` | `MOD`/`%`, só inteiros, sinal do dividendo |
| resto de real | alguns dialetos convertem para inteiro | erro em geral | erro de compilação |
| potência | `^` | `^` | `POTENCIA(a, b)` (não há `^`; em C é XOR) |
| divisão por zero | erro em tempo de execução (em geral) | erro em tempo de execução | `/` = UB; `DIVIDE` = erro com linha |
| estouro inteiro | dependente do dialeto; muitos promovem a real | erro ou wrap, dependendo do dialeto | sinal: UB; sem sinal: módulo 2ᴺ (definido) |
| precedência | própria do dialeto | própria do dialeto | **a de C** |

**Consequência:** a divisão é a diferença que mais causa bug em migração. `7 / 2` vale
`3` em BASICGOL. E `^` não é potência: `2 ^ 3` é XOR (vale 1). Use `POTENCIA(2, 3)`.

### 13.4 Lógica e verdade

| Tema | BASIC clássico | Portugol | BASICGOL |
|---|---|---|---|
| operadores | `AND`, `OR`, `NOT` **bit a bit** | `e`, `ou`, `nao` lógicos | `E`, `OU`, `NAO` lógicos (`&&`, `\|\|`, `!`) |
| curto-circuito | não garantido (bit a bit) | depende do dialeto | garantido por C |
| valor de verdadeiro | `TRUE` = −1 em GW-BASIC | `verdadeiro` | `VERDADEIRO` = 1 (`true`) |
| teste de condição | qualquer valor ≠ 0 | booleano | qualquer escalar ≠ 0 |
| XOR lógico | `XOR` bit a bit | varia | `OU_EXCLUSIVO(a, b)` |

**Consequência:** `AND` bit a bit e `E` lógico não são a mesma operação. Quem escreve
`3 E 5` em BASICGOL obtém `1` (verdadeiro), não `1` por bit. Para bit a bit use `&`, `|`,
`^` (C nativo). E `VERDADEIRO` é 1: `ESCREVAL(VERDADEIRO)` imprime `1`, não `-1`.

### 13.5 Texto

| Tema | BASIC clássico | Portugol | BASICGOL |
|---|---|---|---|
| concatenação | `+` | `+` | `JUNTA_TEXTO`/`ACRESCENTA_TEXTO` (sem `+`) |
| comparação | `=` compara conteúdo | `=` compara conteúdo | `==` compara ponteiro; `TEXTOS_IGUAIS` compara conteúdo |
| mutabilidade | mutável | mutável | `TEXTO` imutável; `TEXTO_FIXO` mutável |
| tamanho | `LEN` | `comprimento` | `TAMANHO_TEXTO` (bytes, não caracteres) |
| maiúsculas | `UCASE$` | `maiusculo` | `CARACTERE_MAIUSCULO` (ASCII, 1 byte) ou laço |
| índice | 1-based em alguns dialetos | 1-based em VisuAlg | **0-based** (C) |

**Consequência:** `"a" + "b"` em BASICGOL é aritmética de ponteiros — o compilador avisa,
mas o erro é grave. Vetores e textos começam no índice 0.

### 13.6 Vetores e estruturas

| Tema | BASIC clássico | Portugol | BASICGOL |
|---|---|---|---|
| declaração | `DIM v(10)` | `v: vetor[1..10] de inteiro` | `VETOR(INTEIRO, v, 10);` |
| índices | 0 ou 1, conforme `OPTION BASE` | 1..n (VisuAlg) | **0 … n−1** |
| limite | verificado em alguns dialetos | verificado | **não verificado** (UB, como C) |
| registro | `TYPE` | `registro` | `REGISTRO`/`FIM_REGISTRO` |
| passagem para função | por referência em muitos dialetos | por referência | **por valor**; vetor decai para ponteiro |

**Consequência:** índice fora do vetor não gera erro em BASICGOL — é UB. Passe sempre
vetor **e** tamanho (`f(v, TAMANHO_VETOR(v))`) e valide o índice quando ele vem do
usuário.

### 13.7 Entrada, saída e erros

| Tema | BASIC clássico | Portugol | BASICGOL |
|---|---|---|---|
| escrever | `PRINT a; b` | `escreva(a, b)` | um valor por `ESCREVA`; `ESCREVA_FMT` para vários |
| ler número | `INPUT` | `leia(n)` | `LEIA(n)` (scanf) ou `LEIA_INTEIRO(n)` (linha, verificado) |
| erro de entrada | `ON ERROR`/reexecução | mensagem do interpretador | valor de retorno `LOGICO` |
| exceções | `ON ERROR GOTO` | não há (em geral) | não há; `ERRO`/`AFIRME` + retorno |
| terminador de bloco | `NEXT`/`END IF` | `fimse`/`fimenquanto` | `FIM_SE`/`FIM_ENQUANTO` |

### 13.8 O que BASICGOL **não** tem de propósito

`GOTO` e rótulos, `ON ERROR`, variáveis implícitas, `OPTION BASE`, variáveis
“variant”, `PRINT` com separadores mágicos, `INPUT` com prompt embutido, funções
`LEFT$`/`MID$` (há `TRECHO_TEXTO`, e `strncpy`/`memcpy` continuam disponíveis), e
qualquer “modo texto” que esconda a memória. Todas essas escolhas estão registradas em
[04 — Restrições e decisões](04-restricoes-e-decisoes.md).

---

## 14. Exemplos e testes

| O que | Onde |
|---|---|
| Um valor de cada tipo, executável | [`docs/exemplos/tipos.c`](exemplos/tipos.c) |
| Precedência, conversões, verdade, divisão e texto | [`docs/exemplos/expressoes.c`](exemplos/expressoes.c) |
| Tipos inteiros e aritmética definida | `tests/casos/09_tipos_expr/t01_inteiros` |
| Reais, tolerância, NaN/infinito | `tests/casos/09_tipos_expr/t02_reais` |
| Caractere, escapes e helpers ASCII | `tests/casos/09_tipos_expr/t03_caractere` |
| Lógico, `LOGICO_DE`, XOR, curto-circuito | `tests/casos/09_tipos_expr/t04_logico` |
| Precedência | `tests/casos/09_tipos_expr/t05_precedencia` |
| Conversões válidas e inválidas | `tests/casos/09_tipos_expr/t06_conversoes` |
| `DIVIDE`/`RESTO` válidos | `tests/casos/09_tipos_expr/t07_divisao` |
| Divisão/resto por zero e quociente mínimo não representável | `tests/casos/09_tipos_expr/t08…t10` (t10 verifica erro ou resultado válido conforme a representação) |
| Texto seguro e truncamento | `tests/casos/09_tipos_expr/t11_texto_seguro` |
| `LEIA_INTEIRO`/`LEIA_REAL` e entrada inválida | `tests/casos/09_tipos_expr/t12_leia_numeros` |
| Extensões opcionais | `tests/casos/09_tipos_expr/t13_extensoes` |
| Propriedades de tipo em `_Static_assert` | `tests/casos/09_tipos_expr/t14_propriedades` |
| `LEIA` com cada tipo escalar e o formato escolhido | `tests/casos/09_tipos_expr/t15_leia_tipos` |
| Erros de tipo em tempo de compilação | `tests/casos/06_negativos/n12…n16` |

A matriz completa, gerada a partir da árvore de testes, está em
[`tests/COBERTURA.md`](../tests/COBERTURA.md).
