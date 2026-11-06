# BASICGOL — Referência de Palavras-chave e Operadores

Referência de consulta. Para o *porquê* de cada decisão, ver
[01 — Especificação](01-especificacao.md); para a expansão exata de cada construtor, ver
[03 — Mapeamento para C](03-mapeamento-para-c.md).

## Como ler esta referência

| Coluna | Significado |
|---|---|
| **Forma** | como se escreve no programa `.c` |
| **Perfil** | `N` = funciona no caminho M0 (macros); `X` = só com `basicgolc` (M1); `C` = C nativo |
| **Tipo** | `objeto` = macro sem argumentos; `função(n)` = macro com argumentos; `função C` = helper compilado da biblioteca padrão |

Desde a Fase 5, funções de biblioteca são implementadas em módulos C e exigem
`build/libbasicgol.a` (ou os fontes de `src/biblioteca/`) na ligação. Build, dependências,
contratos de retorno e ownership estão em [09 — Biblioteca padrão](09-biblioteca-padrao.md).

Convenção tipográfica: `MAIÚSCULAS` = palavra-chave; `minusculas` = identificador do
usuário; `expressao` = qualquer expressão C; `cmd*` = zero ou mais comandos.

---

## 1. Estrutura do programa

| Forma | Perfil | Tipo | Significado |
|---|---|---|---|
| `INICIO` | N | objeto | abre a função de entrada: `int main(void) {` |
| `FIM` | N | objeto | fecha a função de entrada: `}` |
| `FUNCAO tipo nome(params) { … }` | N | objeto vazio | declara função com valor de retorno (`FUNCAO` expande para nada; o tipo vem em seguida) |
| `PROCEDIMENTO nome(params) { … }` | N | objeto | declara procedimento (`void`) |
| `RETORNE expressao;` | N | objeto | devolve valor |
| `ESTATICO` | N | objeto | `static`: função/variável privada do arquivo |
| `EXTERNO` | N | objeto | `extern`: declara símbolo de outra unidade |

## 2. Tipos

| Palavra-chave | Tipo C | Observação |
|---|---|---|
| `LOGICO` | `bool` | valores: `VERDADEIRO`, `FALSO` |
| `CARACTERE` | `char` | 1 byte; sinal depende da plataforma |
| `INTEIRO_CURTO` | `short` | ≥ 16 bits |
| `INTEIRO` | `int` | ≥ 16 bits |
| `INTEIRO_LONGO` | `long` | ≥ 32 bits |
| `INTEIRO8` `INTEIRO16` `INTEIRO32` `INTEIRO64` | `int8_t` … `int64_t` | largura exata |
| `NATURAL` | `unsigned` | ≥ 16 bits |
| `NATURAL_CURTO` | `unsigned short` | ≥ 16 bits |
| `NATURAL_LONGO` | `unsigned long` | ≥ 32 bits |
| `NATURAL8` `NATURAL16` `NATURAL32` `NATURAL64` | `uint8_t` … `uint64_t` | largura exata, sem sinal |
| `REAL_CURTO` | `float` | precisão simples |
| `REAL` | `double` | precisão dupla |
| `REAL_LONGO` | `long double` | ≥ `double` |
| `TEXTO` | `const char *` | referência imutável a texto UTF-8 |
| `VAZIO` | `void` | ausência de valor |

Qualificadores: `CONSTANTE` → `static const`; `ESTATICO` → `static`; `EXTERNO` → `extern`.

Inteiros e reais têm a largura da plataforma, exceto os de largura exata. As constantes
abaixo dizem o valor **real** do ambiente:

| Forma | Valor | Uso |
|---|---|---|
| `TAMANHO_DE(t)` | `((size_t) sizeof(t))` | bytes do tipo |
| `BITS_DE(t)` | `((size_t) (sizeof(t) * CHAR_BIT))` | bits do tipo |
| `MAXIMO_INTEIRO` / `MINIMO_INTEIRO` | `INT_MAX` / `INT_MIN` | faixa de `INTEIRO` |
| `MAXIMO_NATURAL` | `UINT_MAX` | faixa de `NATURAL` |
| `MAXIMO_REAL` / `EPSILON_REAL` | `DBL_MAX` / `DBL_EPSILON` | faixa e precisão de `REAL` |

A referência completa de representação, faixa e conversão está em
[06 — Tipos e Expressões](06-tipos-e-expressoes.md).

## 3. Declarações

| Forma | Perfil | Tipo | Significado |
|---|---|---|---|
| `INTEIRO x = 0;` | N/C | — | variável |
| `CONSTANTE REAL k_pi = 3.14159;` | N | objeto | constante |
| `VETOR(INTEIRO, v, 10);` | N | função(3) | vetor de 10 posições, índices 0…9 |
| `TAMANHO_VETOR(v)` | N | função(1) | número de elementos, devolve `NATURAL`; só para vetor real, não ponteiro (C-18, W-06) |
| `TEXTO_FIXO(nome, 40);` | N | função(2) | buffer de texto mutável (`char nome[40]`) |
| `REGISTRO(Nome)` … `FIM_REGISTRO(Nome);` | N | função(1) cada | define registro (`typedef struct`) |
| `ENUMERACAO(Nome)` … `FIM_ENUMERACAO(Nome);` | N | função(1) cada | define enumeração (`typedef enum`) |

## 4. Controle de fluxo

| Forma | Perfil | Significado |
|---|---|---|
| `SE cond ENTAO … FIM_SE` | N | condicional |
| `SENAO_SE cond ENTAO …` | N | novo ramo condicional na mesma cadeia |
| `SENAO …` | N | ramo final |
| `ENQUANTO cond FACA … FIM_ENQUANTO` | N | laço com teste antes |
| `REPITA … ATE_QUE(cond);` | N | laço com teste depois (ao menos uma execução) |
| `PARA (i, inicio, fim) { … }` | N | laço contado inclusivo, passo +1 |
| `PARA_PASSO (i, inicio, fim, passo) { … }` | N | laço contado inclusivo, passo genérico; passo zero é erro de execução |
| `PARA i DE inicio ATE fim [PASSO p] FACA … FIM_PARA` | **X** | mesma semântica, sintaxe Portugol/BASIC |
| `ESCOLHA (expr) … CASO v: … CONTRARIO: … FIM_ESCOLHA` | N | seleção múltipla |
| `PARE;` | N | sai do laço/`ESCOLHA` mais interno (`break`) |
| `PROSSIGA;` | N | próxima iteração (`continue`) |
| `ATE cond;` | **X** | forma alternativa de `ATE_QUE(cond)` |

### Regras de ouro do controle de fluxo

1. `SE` **sempre** tem `ENTAO`; `ENQUANTO` sempre tem `FACA`; `REPITA` sempre termina em
   `ATE_QUE(...)`.
2. Cada bloco aberto tem **um** `FIM_*` no fim (um por cadeia `SE`/`SENAO_SE`/`SENAO`,
   não um por ramo).
3. `SENAO SE` (duas palavras) é **proibido**; use `SENAO_SE`.
4. `SE x { … }` é **proibido**; use `SE x ENTAO … FIM_SE` (ou `if` de C).
5. Blocos criam escopo; declare antes do `SE` o que precisar sobreviver a ele.

## 5. Operadores

### 5.1 Por palavra (a forma canônica didática)

| Forma | Equivalente C | Aridade | Observação |
|---|---|---|---|
| `a E b` | `a && b` | binário | curto-circuito garantido |
| `a OU b` | `a \|\| b` | binário | curto-circuito garantido |
| `NAO a` | `!a` | unário | precedência alta: `NAO a E b` = `(!a) && b` |
| `a MOD b` | `a % b` | binário | só inteiros; sinal do dividendo |
| `VERDADEIRO` / `FALSO` | `true` / `false` | literal | tipo `LOGICO` |
| `IGUAL(a, b)` | `((a) == (b))` | função(2) | evita a confusão `=` × `==`. **É prefixado**: `x IGUAL 3` não compila; para infixo use `==` (§11.6 de 01) |
| `DIFERENTE(a, b)` | `((a) != (b))` | função(2) | idem |
| `TEXTOS_IGUAIS(a, b)` | `strcmp(a,b) == 0` | função(2) | **única** forma correta de comparar conteúdo de texto |
| `TEXTOS_DIFERENTES(a, b)` | `strcmp(a,b) != 0` | função(2) | idem |
| `OU_EXCLUSIVO(a, b)` | `((bool) (((a) != 0) != ((b) != 0)))` | função(2) | XOR **lógico**; normaliza os operandos |
| `LOGICO_DE(x)` | `((bool) ((x) != 0))` | função(1) | valor de verdade normalizado (0/1); aceita literal real (`LOGICO_DE(0.5)`) |
| `DIVIDE(a, b)` | `_Generic` → `bg_divide_*` | função(2) | divisão verificada: recusa divisor 0 e quociente assinado não representável |
| `RESTO(a, b)` | `_Generic` → `bg_resto_*` | função(2) | resto verificado, só inteiros |

### 5.2 Símbolos C (sempre aceitos)

`+ - * /` `%` `= == != < <= > >=` `&& || !` `++ --` `+= -= *= /=` `[ ]` `.` `( )`
`& *` (ponteiros) `? :` `sizeof`.

### 5.3 Precedência (idêntica a C)

```
( )  [ ]  .  ->
++ -- (pós)   NAO (!)  ~  + - (unários)  (cast)  * (indireção)  & (endereço)  sizeof
* / MOD
+ -
< <= > >=
== !=
E (&&)
OU (||)
? :
= += -= *= /= %=
,
```

**R-6**: parentize ao misturar `E`/`OU`/`NAO` com comparações ou aritmética.

**Nota**: `MOD=` (com `MOD` por palavra) **não existe** — geraria `%= ` como dois tokens.
Use `x = x MOD 2` ou `x %= 2` (C nativo). Ver C-01 em [04](04-restricoes-e-decisoes.md).

`DIVIDE`/`RESTO` mantêm o tipo promovido dos operandos e trocam o comportamento
indefinido por um erro com arquivo e linha. `/` e `MOD` continuam sendo o C cru; a
diferença entre eles está em [06 §5.4–5.5](06-tipos-e-expressoes.md).

## 6. Entrada e saída

| Forma | Perfil | Significado |
|---|---|---|
| `ESCREVA(x);` | N | escreve **um** valor escalar, sem quebra de linha |
| `ESCREVAL(x);` | N | escreve **um** valor escalar e quebra a linha |
| `ESCREVA_LINHA();` | N | quebra a linha |
| `ESCREVA_TEXTO(s);` | N | escreve texto como `%s` (sem consultar tipo) |
| `ESCREVA_FMT(fmt, …);` | N | `printf` direto (formatação fina, múltiplos valores) |
| `ESCREVA_ERRO(s);` | N | escreve texto em `stderr` |
| `LEIA(x);` | N | lê **um** valor escalar para `x`; o formato sai de `_Generic` pelo tipo do destino (`NATURAL8`→`%hhu`, `NATURAL_CURTO`→`%hu`, `REAL_LONGO`→`%Lf`, `CARACTERE`→` %c`); `LOGICO`, `TEXTO` e `TEXTO_FIXO` são **recusados pelo compilador** (erro de `_Generic`, não aviso); texto se lê com `LEIA_TEXTO` |
| `LEIA_OK(x);` | N | lê e devolve `LOGICO` (sucesso/falha) |
| `LEIA_TEXTO(buf, tam);` | N | lê uma linha para `TEXTO_FIXO`; consome o `\n` pendente de um `LEIA` anterior, se houver |
| `LEIA_INTEIRO(x);` | N | lê **uma linha** e converte para `INTEIRO`; devolve `LOGICO`; não deixa lixo no `stdin` |
| `LEIA_REAL(x);` | N | idem para `REAL` |

Formatos usados por `ESCREVA`/`ESCREVAL` (escolhidos em tempo de compilação):

| Tipo do argumento | Formato |
|---|---|
| `LOGICO`, `INTEIRO`, `INTEIRO_CURTO` | `%d` |
| `INTEIRO_LONGO`, `INTEIRO64` | `%ld` / `%lld` |
| `NATURAL`, `NATURAL32`, `NATURAL64`, `size_t` | `%u` / `%lu` |
| `CARACTERE` | `%c` |
| `REAL_CURTO`, `REAL` | `%g` |
| `REAL_LONGO` | `%Lg` |
| `TEXTO`, `char *` | `%s` |
| enumeração | `%d` |
| qualquer outro (ponteiro) | `%p` |

Limitações declaradas: sem controle de casas decimais (`ESCREVA_FMT` resolve); um valor
por chamada; sem detecção de tipo em tempo de execução.

## 7. Matemática, texto e utilidades

| Forma | Perfil | Tipo | Significado |
|---|---|---|---|
| `ABS(x)` | N | função C selecionada por `_Generic` | valor absoluto; mínimo de inteiro assinado é erro fatal porque o resultado não cabe |
| `MINIMO(a, b)` / `MAXIMO(a, b)` | N | função C selecionada por `_Generic` | mínimo/máximo; cada argumento é avaliado uma vez |
| `POTENCIA(base, exp)` | N | macro → libm | potência (`pow`, `-lm`) |
| `RAIZ(x)` | N | macro → libm | raiz quadrada (`sqrt`, `-lm`) |
| `ARREDONDA(x)` | N | função C (`llround`) | inteiro mais próximo, empate longe de zero |
| `PISO(x)` / `TETO(x)` | N | macro → libm | piso/teto (`floor`/`ceil`) |
| `SORTEIA(n)` | N | função C | inteiro pseudoaleatório em `0 … n-1`; para limites inválidos devolve 0 |
| `INICIA_SORTEIO(semente)` | N | função C | inicializa explicitamente o PRNG próprio (`xorshift64*`, derivado por `splitmix64`); mesma semente → mesma sequência em qualquer plataforma |
| `TAMANHO_TEXTO(s)` | N | função C | comprimento em bytes (`strlen`), devolve `NATURAL`; cada acento UTF-8 ocupa 2 bytes (D-20) |
| `COPIA_TEXTO(destino, tam, origem)` | N | função C → `LOGICO` | copia com verificação de espaço |
| `JUNTA_TEXTO(destino, tam, a, b)` | N | função C → `LOGICO` | concatena dois textos em `destino` |
| `TEXTO_PARA_INTEIRO(s, &var)` / `TEXTO_PARA_REAL(s, &var)` | N | função C → `LOGICO` | converte texto em número; **rejeita** texto com sobra (`"4x2"`, `"12abc"`) e estouro de faixa |
| `TEXTO_PARA_NATURAL(s, &var)` | N | função C → `LOGICO` | idem para `NATURAL`; recusa `-` |
| `REAL_PARA_INTEIRO(x, &var)` | N | função C → `LOGICO` | converte real em inteiro com checagem de faixa e NaN |
| `ACRESCENTA_TEXTO(destino, tam, origem)` | N | função C → `LOGICO` | acrescenta ao fim do conteúdo atual |
| `TRECHO_TEXTO(destino, tam, origem, inicio, n)` | N | função C → `LOGICO` | copia até `n` bytes a partir de `inicio` |
| `COMPARA_TEXTO(a, b)` | N | função C → `INTEIRO` | −1/0/+1; `NULL` conta como `""` |
| `TEXTO_VAZIO(s)` | N | função C → `LOGICO` | verdadeiro para `""` e `NULL` |
| `FORMATA_TEXTO(destino, tam, formato, …)` | N | função C → `LOGICO` | `vsnprintf` verificável; `false` se truncou |
| `E_DIGITO(c)` / `E_LETRA(c)` | N | função C → `LOGICO` | classificação ASCII, independente de locale |
| `CARACTERE_MAIUSCULO(c)` / `CARACTERE_MINUSCULO(c)` | N | função C → `CARACTERE` | conversão ASCII de 1 byte |

**Não existe** concatenação com `+`: `"a" + "b"` é aritmética de ponteiros em C e produz
lixo. Use `JUNTA_TEXTO` ou `ACRESCENTA_TEXTO`.

Todo truncamento é reportado por valor de retorno, e em caso de falha o destino fica
vazio (`dest[0] == '\0'`) — nunca conteúdo parcial. Origem `NULL` é texto vazio, e origem
e destino podem ser o mesmo buffer (`memmove`). Contrato completo em
[06 §8](06-tipos-e-expressoes.md).

## 8. Erros

| Forma | Perfil | Significado |
|---|---|---|
| `ERRO("mensagem");` | N | imprime `arquivo:linha: erro: mensagem` em `stderr` e encerra com código 1 |
| `AFIRME(cond, "mensagem");` | N | como `ERRO`, mas só dispara quando `cond` é falsa |
| `LEIA_OK(x)` / funções que devolvem `LOGICO` | N | tratamento por valor de retorno (forma preferida) |

Não há `TENTE`/`CAPTURE`/`FINALMENTE` em v1.0 (ver [D-06](04-restricoes-e-decisoes.md)).

## 9. Configuração (macros `BG_*`)

| Macro | Efeito |
|---|---|
| `BG_SEM_APELIDOS` | definida **antes** do `#include`: desliga os nomes curtos; só existem `BG_SE`, `BG_ENTAO`, … |
| `BG_VERSAO_MAIOR` / `BG_VERSAO_MENOR` / `BG_VERSAO_CORRECAO` | versão da implementação (inteiros) |
| `BG_VERSAO_MINIMA(mai, men)` | emite `#error` se a implementação for mais antiga |
| `BG_ESTRITO` | liga verificações extras em tempo de compilação (reservado; ver D-09) |
| `BG_USAR_EXTENSOES` | liga a seção opcional de extensões (`SOMA_SEGURA`, …); exige GCC/Clang (D-22) |
| `BG_TEM_EXTENSOES` | definida **pelo cabeçalho** quando as extensões estão ativas (`1`) |

## 10. Nomes reservados

Não podem ser usados como identificadores: todas as palavras-chave das tabelas acima,
os nomes `bg_*`, `BG_*`, `k_*` (convenção), identificadores iniciados por `_`, e os
reservados para o futuro: `PARA_CADA`, `TENTE`, `CAPTURE`, `FINALMENTE`, `CLASSE`,
`MODULO`, `IMPORTE`, `GLOBAL`, `LOCAL`, `REF`, `PONTEIRO`.

## 11. Índice alfabético

`ABS` · `ACRESCENTA_TEXTO` · `AFIRME` · `ARREDONDA` · `ATE` (X) · `ATE_QUE` · `BITS_DE` ·
`CARACTERE_MAIUSCULO` · `CARACTERE_MINUSCULO` · `CASO` · `CARACTERE` · `COMPARA_TEXTO` ·
`CONSTANTE` · `CONTRARIO` · `COPIA_TEXTO` · `DE` (X) · `DIFERENTE` · `DIVIDE` · `E` ·
`E_DIGITO` · `E_LETRA` · `ENTAO` · `EPSILON_REAL` ·
`ENQUANTO` · `ENUMERACAO` · `ERRO` · `ESCOLHA` · `ESCREVA` · `ESCREVA_ERRO` ·
`ESCREVA_FMT` · `ESCREVA_LINHA` · `ESCREVA_TEXTO` · `ESCREVAL` · `ESTATICO` · `EXTERNO` ·
`FACA` · `FALSO` · `FIM` · `FIM_ENQUANTO` · `FIM_ENUMERACAO` · `FIM_ESCOLHA` · `FORMATA_TEXTO` ·
`FIM_PARA` (X) · `FIM_REGISTRO` · `FIM_SE` · `FUNCAO` · `IGUAL` ·
`INICIA_SORTEIO` · `INICIO` · `INTEIRO` · `INTEIRO8/16/32/64` · `INTEIRO_CURTO` ·
`INTEIRO_LONGO` · `JUNTA_TEXTO` · `LEIA` · `LEIA_INTEIRO` · `LEIA_OK` · `LEIA_REAL` ·
`LEIA_TEXTO` · `LOGICO` · `LOGICO_DE` · `MAXIMO` · `MAXIMO_INTEIRO` · `MAXIMO_NATURAL` ·
`MAXIMO_REAL` · `MINIMO` · `MINIMO_INTEIRO` · `MOD` · `NAO` · `NATURAL` ·
`NATURAL_CURTO` · `NATURAL_LONGO` · `NATURAL8/16/32/64` · `OU` · `OU_EXCLUSIVO` ·
`PARA` · `PARA_PASSO` · `PARE` · `PASSO` (X) · `PISO` · `POTENCIA` · `PROCEDIMENTO` ·
`PROSSIGA` · `RAIZ` · `REAL` · `REAL_CURTO` · `REAL_LONGO` · `REAL_PARA_INTEIRO` ·
`REGISTRO` · `REPITA` · `RESTO` ·
`RETORNE` · `SE` · `SENAO` · `SENAO_SE` · `SORTEIA` · `TAMANHO_DE` · `TAMANHO_TEXTO` ·
`TAMANHO_VETOR` · `TETO` · `TEXTO` · `TEXTO_FIXO` · `TEXTO_PARA_INTEIRO` ·
`TEXTO_PARA_NATURAL` · `TEXTO_PARA_REAL` · `TEXTO_VAZIO` · `TEXTOS_DIFERENTES` ·
`TEXTOS_IGUAIS` · `TRECHO_TEXTO` · `VAZIO` · `VERDADEIRO` · `VETOR`
