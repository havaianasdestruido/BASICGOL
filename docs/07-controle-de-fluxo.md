# BASICGOL — Controle de fluxo (Fase 3)

Este documento é a referência **normativa** do controle de fluxo: a sintaxe exata de cada
construtor, a expansão C que o cabeçalho e o transpilador produzem, os casos de borda, os
limites conhecidos e o que as macros **não** conseguem fazer.

> **Leia primeiro (§8).** Macros C não leem BASICGOL. O que o caminho M0 faz é
> *substituir palavras por tokens de C*; ele não analisa expressões nem blocos. Só o
> transpilador M1 (`basicgolc`) mantém uma pilha de blocos e recusa blocos mal fechados
> antes de o compilador C entrar em cena — e mesmo ele é um reescritor de tokens, não um
> analisador completo.

Perfis: **N** = núcleo (M0, só macros, `include/basicgol.h`); **X** = açúcar (exige M1).
Números `c24…c40` referem-se aos casos em `tests/casos/03_controle/`.

---

## 1. Visão geral

| Construtor | Perfil | Abre | Fecha | C produzido (M0) |
|---|---|---|---|---|
| condicional | N | `SE cond ENTAO` | `FIM_SE` | `if (cond) {` … `}` |
| alternativa encadeada | N | `SENAO_SE cond ENTAO` | — | `} else if (cond) {` |
| alternativa final | N | `SENAO` | — | `} else {` |
| laço com teste antes | N | `ENQUANTO cond FACA` | `FIM_ENQUANTO` | `while (cond) {` … `}` |
| laço com teste depois | N | `REPITA` | `ATE_QUE(cond);` | `do {` … `} while (!(cond));` |
| laço contado | N | `PARA (i, a, b) {` | `}` (C) | `for ((i) = (a); (i) <= (b); ++(i)) {` |
| laço contado com passo | N | `PARA_PASSO (i, a, b, p) {` | `}` (C) | `for (…; sinal(p) …; (i) += (p)) {` |
| laço contado (sintaxe PT) | **X** | `PARA i DE a ATE b [PASSO p] FACA` | `FIM_PARA` | `for (…) {` (M1) |
| repita com `ATE` | **X** | `REPITA` … `ATE cond;` | — | `} while (!(cond));` (M1) |
| seleção múltipla | N | `ESCOLHA (e)` | `FIM_ESCOLHA` | `switch (e) {` … `}` |
| rótulo de caso | N | `CASO v:` | — | `case v:` |
| caso padrão | N | `CONTRARIO:` | — | `default:` |
| sai do laço/escolha | N | `PARE;` | — | `break;` |
| próxima iteração | N | `PROSSIGA;` | — | `continue;` |

Não existe `goto`, `PARE rotulado`, `CASO a ATE b` (faixa) nem `SENAO` para laços
(§8 lista o que falta e por quê).

---

## 2. Regras de bloco

1. **Abertura e fechamento.** Cada bloco tem uma palavra de abertura e uma de fechamento
   `FIM_*`, com **uma exceção de forma**: `REPITA` fecha com `ATE_QUE(cond);`, porque o
   teste faz parte do `do … while` e vem por último.
2. **Os blocos aninham por pilha.** Um `FIM_*` fecha sempre o bloco **aberto mais
   recentemente**. Isso é o que torna o `SENAO` não-ambíguo (§6).
3. **As formas com parênteses de C ficam como C.** `PARA (i, a, b) {` e
   `PARA_PASSO (i, a, b, p) {` abrem com `{` e fecham com `}`. Não há `FIM_PARA` no M0
   (§8.6).
4. **Cada comando simples termina em `;`.** Os blocos não terminam em `;`: `FIM_SE`,
   `FIM_ENQUANTO`, `FIM_ESCOLHA` e `}` aceitam um `;` a mais, que vira comando vazio
   (CND-6). Os estilos canônicos o omitem.
5. **Condições não usam `;`.** Terminam em `ENTAO` ou `FACA` (ou em `)` nas macros de
   função, §7).

### 2.1 Gramática (EBNF, forma aproximada)

```
se        := SE cond ENTAO comandos
             { SENAO_SE cond ENTAO comandos }
             [ SENAO comandos ]
             FIM_SE
enquanto  := ENQUANTO cond FACA comandos FIM_ENQUANTO
repita0   := REPITA comandos ATE_QUE '(' cond ')' ';'                 (N)
repita1   := REPITA comandos ATE cond ';'                             (X)
para0     := PARA   '(' var ',' ini ',' fim ')'  '{' comandos '}'     (N)
para_pass := PARA_PASSO '(' var ',' ini ',' fim ',' passo ')' '{' comandos '}'  (N)
para1     := PARA var DE ini ATE fim [ PASSO passo ] FACA comandos FIM_PARA     (X)
escolha   := ESCOLHA '(' expr ')' { CASO const ':' comandos }
             [ CONTRARIO ':' comandos ]
             FIM_ESCOLHA
```

`comandos` é uma sequência de comandos C/BG, incluindo outros blocos. `const` é uma
constante de compilação (regra de C). `var` é uma variável **já declarada** (§5.6).

---

## 3. Expansões verificadas

As expansões abaixo são as da tabela `tests/expansoes.txt` (AC-22), que o teste
`01_sanidade` compara com `cc -dM -E` do cabeçalho real:

```c
/* alvos BG_* do cabecalho (saida de cc -dM -E); os nomes curtos sao aliases */
#define SE  if (
#define ENTAO  ) {
#define SENAO_SE  } else if (
#define SENAO  } else {
#define FIM_SE  }
#define ENQUANTO  while (
#define FACA  ) {
#define FIM_ENQUANTO  }
#define REPITA  do {
#define ATE_QUE(c)  } while (!(c))
#define ESCOLHA(e)  switch (e) {
#define CASO  case
#define CONTRARIO  default
#define FIM_ESCOLHA  }
#define PARE  break
#define PROSSIGA  continue
#define PARA(i,a,b)  for ((i) = (a); (i) <= (b); ++(i))
#define PARA_PASSO(i,a,b,p)  for ((i) = (a); ((p) > 0) ? ((i) <= (b)) : (((p) == 0) ? bg_passo_zero(__FILE__, __LINE__) : ((i) >= (b))); (i) += (p))
```

O açúcar do M1 emite a mesma forma de `PARA_PASSO`, com `p` = `1` quando `PASSO` é omitido:

```c
PARA i DE 10 ATE 1 PASSO -1 FACA   ->
for (i = (10); (-1) > 0 ? (i <= (1)) : ((-1) == 0 ? bg_passo_zero(__FILE__, __LINE__) : (i >= (1))); i += (-1)) {
```

`ESCOLHA(e)`, `ATE_QUE(c)`, `PARA` e `PARA_PASSO` são **macros de função**: o argumento é
delimitado por vírgulas de nível superior (§7). `SE`, `ENTAO`, `ENQUANTO` e `FACA` são
macros sem argumento; a condição delas aceita vírgula (caso `c38`).

---

## 4. Sintaxe exata por construtor

### 4.1 Condicional

```
SE cond ENTAO
    comandos
[ SENAO_SE cond ENTAO
    comandos ]*
[ SENAO
    comandos ]
FIM_SE
```

* `SE` exige `ENTAO`. `SE x {` é recusado (`'SE' exige 'ENTAO'`).
* Encadeamento é **`SENAO_SE`**, uma palavra só. `SENAO SE` é recusado pelo M1 (§6.2).
* `SENAO` e `SENAO_SE` só valem depois de um `SE`/`SENAO_SE` aberto.
* Um único `FIM_SE` fecha a cadeia inteira (CND-4).

### 4.2 Enquanto

```
ENQUANTO cond FACA
    comandos
FIM_ENQUANTO
```

Testa **antes** de cada iteração: zero execuções é um caso normal (`c06`, `c24`).

### 4.3 Repita

```
REPITA
    comandos
ATE_QUE(cond);          // M0 e M1
ATE cond;               // só M1 (açúcar), e só dentro de REPITA aberto
```

* Executa **ao menos uma vez**. `ATE_QUE(cond)` termina quando `cond` é **verdadeira**
  (`while (!(cond))`).
* O `;` é obrigatório. Sem ele, o erro do C aparece na linha seguinte; por isso o M1
  acusa a falta antes do compilador (`ATE_QUE(cond)` sem `;`).
* `PROSSIGA` dentro de `REPITA` vai para o teste, não para o início (`c30`).

### 4.4 Contado — forma N (C com chaves)

```
PARA (i, inicio, fim) {
    comandos
}

PARA_PASSO (i, inicio, fim, passo) {
    comandos
}
```

* `i` é uma variável **existente**; não pode ser declarada na construção.
* **Inclusivo nos dois extremos.** `inicio` é avaliado uma vez; `fim` e `passo` são
  reavaliados a cada iteração — por isso devem ser livres de efeitos colaterais.
* `fim < inicio` (com passo positivo) → zero iterações (`c12`, `c24`).
* Passo negativo conta para trás; o fim ainda é incluído (`c23`, `c25`).
* Passo que não cai no fim para antes dele: `PARA_PASSO (i, 1, 10, 4)` → 1, 5, 9 (`c25`).
* Após o laço, a variável vale o primeiro valor **além** do fim (`fim + 1` com passo
  positivo; `c32`), como em qualquer `for` de C.
* Passo **zero** é erro de execução (§5.3).

### 4.5 Contado — forma X (açúcar do M1)

```
PARA i DE inicio ATE fim [ PASSO p ] FACA
    comandos
FIM_PARA
```

Mesma semântica de `PARA_PASSO`; `PASSO` omitido vale `1`. Não existe no caminho M0.
Programas que precisam compilar sem `basicgolc` usam §4.4.

### 4.6 Escolha

```
ESCOLHA (expr)
    CASO valor1:
        comandos
        PARE;
    CASO valor2:
    CASO valor3:          // rótulos vazios agrupados: não há queda (fall-through)
        comandos
        PARE;
    CONTRARIO:
        comandos
FIM_ESCOLHA
```

* `expr` é inteiro ou enumeração (regra de C); `CASO` aceita constantes de compilação.
* `PARE;` sai **só do `ESCOLHA`**, não do laço que o contém (§5.4, `c35`).
* Queda de um caso para o seguinte (sem `PARE`) é C e gera aviso `-Wimplicit-fallthrough`
  sob `-Wextra`; `c19` mostra o comentário `/* fall through */` e o aviso desligado.
* Um caso com variável local usa bloco explícito: `CASO 3: { INTEIRO t = 1; … PARE; }`.

### 4.7 Desvios

| Comando | Efeito | Onde é válido |
|---|---|---|
| `PARE;` | sai do laço ou do `ESCOLHA` **mais interno** | dentro de `ENQUANTO`, `REPITA`, `PARA`, `PARA_PASSO`, `PARA i DE`, `ESCOLHA` |
| `PROSSIGA;` | vai para a próxima iteração do laço **mais interno** | dentro de laço (não de `ESCOLHA` sem laço) |

Fora de laço/escolha, o compilador C recusa (`n17`, `n18`).

---

## 5. Semântica e casos de borda

### 5.1 Zero iterações

| Construtor | Zero iterações quando | Caso |
|---|---|---|
| `ENQUANTO` | `cond` é falsa na entrada | `c06`, `c24` |
| `PARA` / `PARA i DE` | `fim < inicio` | `c12`, `c24`, `c37`; exemplo `controle_para_acucar` (M1) |
| `PARA_PASSO` | intervalo não alcançado com o sinal do passo | `c24`, `c37` |
| `REPITA` | **nunca** — executa ao menos uma vez | `c09`, `c24` |

### 5.2 Limites

* Inclusivo nos dois extremos (`c25`): `PARA (i, 4, 4)` executa uma vez.
* Faixas que cruzam zero funcionam (`c25`: `-2 … 1`).
* Passo sem sinal (`NATURAL`) é aceito e sem avisos sob `-Wextra -Wpedantic` (`c40`).

### 5.3 Passo zero — erro, não laço infinito

Antes da Fase 3, `PARA_PASSO (i, 10, 1, p)` com `p = 0` girava para sempre (o teste
`i >= fim` continuava verdadeiro e `i += 0` nunca avança). Agora:

* **M0 e M1 com passo variável**: a condição chama `bg_passo_zero`, que imprime
  `arquivo:linha: erro: PARA_PASSO: passo zero (o laco nunca avancaria)` e sai com código 1
  (`c26`, `c27`). A checagem roda **antes** da primeira iteração, mesmo com intervalo vazio.
* **M1 com passo literal**: `PASSO 0` é recusado na tradução com
  `'PASSO 0' nunca avanca o laco: use um passo diferente de zero` (`12_negativos_m1/t12`).

Compatibilidade: para `p ≠ 0` o teste de parada é o mesmo de antes; só o caso `p = 0` mudou,
e ele era laço infinito (ou zero iterações, se `i < fim`). Nenhum programa válido com passo
não nulo muda de comportamento.

### 5.4 `PARE` e `PROSSIGA` com laços e escolhas aninhados

* `PARE` afeta **só o mais interno** (`c28`). Para sair de dois laços: bandeira (`c29`).
* `PARE` dentro de `ESCOLHA` que está dentro de laço sai do `ESCOLHA`; o laço segue (`c35`).
* `PROSSIGA` dentro de `ESCOLHA` que está dentro de laço vai para a próxima iteração do
  laço (`c35`, caso `CASO 0`).

### 5.5 Variável de controle alterada no corpo

`PARA` e `PARA i DE` com `i = i + 1` no corpo: o C define o comportamento (o incremento
do `for` acontece depois). O M1 avisa com **W-04**; o M0 não avisa. Evite.

### 5.6 Variável de controle

* Precisa existir antes do laço (`INTEIRO i = 0;` ou `INTEIRO i;`).
* Use `INTEIRO` (ou inteiro com sinal). Com `NATURAL`, `PARA (i, 0, n - 1)` com `n = 0`
  dá `UINT_MAX` como limite — laço que não termina (§8.8).

---

## 6. Ninhos, `SENAO` e o "dangling else"

### 6.1 Por que não há ambiguidade do `else` pendente

Em C, `if (a) if (b) x; else y;` liga o `else` ao `if (b)`, e o leitor pode errar. Em
BASICGOL, **cada `SE` pede seu próprio `FIM_SE`**. O `SENAO` pertence sempre ao `SE` aberto
mais recente, e o `FIM_SE` do aluno mostra onde esse `SE` acaba:

```
SE a > 0 ENTAO
    SE b > 0 ENTAO
        ESCREVAL("a e b");
    SENAO                        // do SE de dentro (b)
        ESCREVAL("so a");
    FIM_SE
SENAO                            // do SE de fora (a)
    ESCREVAL("nem a");
FIM_SE
```

(`c33` fixa a saída das quatro combinações.)

Se o `FIM_SE` interno faltar, as chaves não fecham: o C acusa **erro no fim do arquivo**, e
o M1 acusa `'FIM' fecha o bloco errado: esperado 'FIM_SE'` (verificado). O erro nunca é
silencioso.

### 6.2 `SENAO SE` (dois tokens) é recusado — e o M0 o aceita

A regra C-05 proíbe escrever `SENAO SE`, para não confundir com encadeamento. O M1 a aplica
**mesmo quando você quer um `SE` aninhado** dentro do `SENAO`:

```
SENAO
    SE b > 0 ENTAO           // M1: 'SENAO SE' nao encadeia ...
```

Solução: coloque um comando **antes** do `SE` aninhado (`c39`):

```
SENAO
    ESCREVAL("nao positivo");
    SE b > 0 ENTAO
        ESCREVAL("b positivo");
    FIM_SE
FIM_SE
```

O M0 **não** consegue recusar `SENAO SE`: as duas palavras viram `} else { if (`, que é C
válido, e o programa compila. É uma divergência real entre os caminhos; a regra é a
especificação, e a suíte fixa o padrão recomendado.

### 6.3 Ninho de laços

`PARE` e `PROSSIGA` atuam no laço mais interno; a construção de ninho não tem rótulos
(§5.4, `c28`). Os ninhos de `PARA` com zero iterações no interno não afetam o externo
(`c37`).

---

## 7. Ponto e vírgula, vírgulas e fronteiras de comando

### 7.1 `;`

| Onde | Obrigatório? | Erro se faltar |
|---|---|---|
| fim de comando simples (`x = 1;`, `ESCREVAL(x);`) | sim | erro do C na linha seguinte |
| `ATE_QUE(cond)` | sim | M1 acusa (`t14`); no M0 é erro do C |
| `ATE cond` (M1) | sim | erro do C |
| `FIM_SE`, `FIM_ENQUANTO`, `FIM_ESCOLHA`, `FIM_PARA` | não (é aceito) | — |
| depois de `ENTAO`, `FACA`, `CASO v:`, `CONTRARIO:` | não | — |

Um `;` logo após um bloco (`FIM_SE;`) é comando vazio e inofensivo.

### 7.2 Vírgulas de nível superior

`ESCOLHA(e)`, `ATE_QUE(c)`, `PARA(i, a, b)` e `PARA_PASSO(i, a, b, p)` são macros com
argumentos. Uma vírgula de nível superior **dentro** delas divide o argumento:

```
ATE_QUE(a = a + 1, b > 2);     // M0: "passed 2 arguments"  (n19)
ATE_QUE((a = a + 1, b > 2));   // correto: parênteses extras
ESCOLHA (a, b)                 // M0: "passed 2 arguments"  (n20)
ESCOLHA (f(a, b))              // correto: a vírgula está dentro de f(...)  (c22)
```

`SE`, `ENTAO`, `ENQUANTO` e `FACA` não têm argumentos, então `ENQUANTO (i++, i < 4) FACA`
funciona (`c38`).

### 7.3 Fronteiras

* A condição de `SE` termina em `ENTAO`; se faltar, o M1 diz
  `'SE' exige 'ENTAO' antes de ';'` (verificado) e o M0 falha no C.
* O corpo de um `PARA` com chaves é o que está entre `{` e `}`. **Sem chaves**, o C toma
  como corpo só a próxima instrução. Por isso o estilo canônico usa chaves sempre.

---

## 8. Limitações (leia antes de usar)

1. **As macros não leem BASICGOL.** O M0 é substituição de palavras. Não há análise de
   expressões, de tipos nem de blocos. Erros de sintaxe aparecem como erros do C, em termos
   do C. O M1 é um reescritor de tokens com uma pilha de blocos; acusa blocos mal
   fechados e os pares proibidos, mas **não** é um parser completo.
2. **Sem `PARE` nem `PROSSIGA` rotulados.** Para sair de vários laços, use uma bandeira (`c29`).
3. **`PARE` dentro de `ESCOLHA` não sai do laço** (§5.4). É o `break` do C.
4. **Sem `goto`.** Qualquer desvio arbitrário é C nativo, com comentário justificando.
5. **Sem `SENAO` em laços** (`ENQUANTO … SENAO`, `PARA … SENAO`). Não existe no C.
6. **`PARA`/`PARA_PASSO` (N) usam chaves de C.** A macro é de função: a expansão fecha o
   `for (…)`, então `FACA` não pode vir depois sem mudar a expansão, o que quebraria a
   tabela AC-22. Por isso o fechamento é `}`. Misturar as formas dá erro: `FIM_PARA` depois
   de `PARA (…) {` é `undeclared` no M0 e `fecha o bloco errado` no M1 (verificado).
7. **`SENAO SE` diverge entre M0 e M1** (§6.2).
8. **Limites que nunca são alcançados não terminam.** `PARA (i, 0, INT_MAX)` — o `i++`
   passa de `INT_MAX` (comportamento indefinido) e `i <= INT_MAX` sempre vale. Com
   `NATURAL`, `PARA (k, 0, n - 1)` com `n = 0` dá `UINT_MAX` e `k <= UINT_MAX` sempre vale:
   o laço não termina. Use contadores com sinal e confira o limite antes.
9. **`fim` e `passo` são reavaliados a cada iteração.** Expressões com efeito colateral
   (`PARA (i, 1, proximo())`) executam o efeito muitas vezes. Use variáveis já calculadas.
10. **Passo zero é detectado em tempo de execução para passo variável** (§5.3). O M0 não
    tem como detectá-lo na compilação.
11. **`CASO` não tem faixa** (`CASO 1 ATE 5`), porque o C não tem. Liste os valores ou use
    `SE` com `E`.
12. **`ESCOLHA` não aceita texto**: use `SE` com `TEXTOS_IGUAIS` (`11.6` da especificação).
13. **W-04 é só do M1.** O M0 aceita a modificação silenciosamente.
14. **Compiladores testados:** GCC 12.2 (Debian) nesta rodada. O Clang é coberto pelas
    variantes de texto em `esperado_erro.txt`, mas **não foi executado** neste ambiente;
    a validação com Clang fica para a matriz de CI.

---

## 9. Mudanças nesta fase e compatibilidade

| Mudança | Antes | Agora | Compatibilidade |
|---|---|---|---|
| `PARA_PASSO` com passo zero | laço infinito (ou zero iterações, se `i < fim`) | erro `PARA_PASSO: passo zero`, código 1 | só muda programa que já era defeituoso |
| `PASSO 0` literal no M1 | aceito e emitido (laço infinito) | erro de tradução | idem |
| expansão de `PARA_PASSO` (M0) e `PARA i DE` (M1) | `… : ((i) >= (b))` | `… : (((p) == 0) ? bg_passo_zero(__FILE__, __LINE__) : ((i) >= (b)))` | igual para `p ≠ 0` |
| mensagem de `SENAO SE` | `use SENAO_SE` | acrescenta a alternativa do SE aninhado | o prefixo é o mesmo |
| novos casos, exemplos e documentos | — | `c24…c40`, `n17…n20`, `t12`, `t13`, 5 exemplos, este documento | — |

Nenhuma sintaxe nova foi introduzida: a Fase 3 **fixa e testa** o que já era o núcleo de
controle, fecha o único defeito de terminação encontrado e documenta os limites.

---

## 10. Como verificar

```sh
make                  # build/basicgolc
make test             # suíte completa (inclui os casos deste documento)
./tests/run.sh -v 03_controle    # só os casos de controle, com cada verificação
./tests/run.sh -v 06_negativos   # negativos M0 (n17…n20) e M1 (t12, t13)
make exemplos         # compila e roda os exemplos (M0, e M1 para os de açúcar)
```

O relatório de aprovação e reprovação desta fase está em
[`08-relatorio-controle-de-fluxo.md`](08-relatorio-controle-de-fluxo.md).
