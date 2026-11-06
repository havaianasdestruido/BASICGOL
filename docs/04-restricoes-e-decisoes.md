# BASICGOL — Restrições de Implementação e Decisões

Dois registros:

* **§1–§4 — Restrições (`C-xx`)**: o que o pré-processador C e o C11 *não* permitem, com a
  consequência de cada limite na especificação e a mitigação adotada. Restrição não é
  bug: é o terreno.
* **§5 — Decisões (`D-xx`)**: escolhas de projeto, incluindo as que ainda estão abertas.

§6 registra a sonda de viabilidade usada para validar tudo isto; §7 lista os riscos.

---

## 1. Restrições do pré-processador

### C-01 — O pré-processador não reordena tokens

**Fato.** `#define` só substitui a sequência de tokens da macro pela sua expansão. Não há
como mover o argumento `INTEIRO` para depois do nome, nem o `10` para dentro de `[ ]`.

**Consequência.** `VETOR INTEIRO v DE 10` e `PARA i DE 1 ATE 10 FACA` são impossíveis em
M0: por isso `VETOR(tipo, nome, tam)` é macro de função (perfil N) e o laço contado em M0
é `PARA(i, a, b)` (três argumentos na ordem certa). As formas "naturais" ficam em M1
(perfil X).

A restrição também vale para **operadores compostos**: `%=` é um token único, então
`MOD=` não pode existir — `x MOD= 2` expandiria para `x % = 2`. Use `x = x MOD 2` ou
`x %= 2` (C nativo). O mesmo raciocínio explica por que não existe `E=` nem `OU=`.

**Mitigação.** Todo construtor X tem um equivalente N documentado
([03 §6](03-mapeamento-para-c.md)) e a especificação exige que os exemplos didáticos
escolham um dos dois explicitamente.

### C-02 — O pré-processador não insere `;` com segurança

**Fato.** Uma macro pode terminar em `;`, mas isso quebra qualquer uso em
`if (…) MACRO else …` e produz `;;` quando o programador já escreveu o `;`.

**Consequência.** P-07: comando simples **sempre** termina em `;` e o `;` é do
programador. `ATE_QUE(c)` é a única exceção deliberada: a macro fecha o `do/while` e o
`;` do programador é o terminador do `while` — coerente com C.

**Mitigação.** Nenhuma macro de comando inclui `;` final. A regra "bloco não leva `;`,
comando leva" é ensinada em uma linha.

### C-03 — Não há contagem nem classificação de argumentos

**Fato.** O pré-processador não sabe quantos argumentos recebeu (fora `__VA_ARGS__`), nem
seus tipos. `_Generic` é uma construção de **C**, não do pré-processador: ela escolhe uma
*expressão* por tipo, não gera código por argumento.

**Consequência.** `ESCREVAL(a, b, c)` não existe (D-07). `ESCREVA(x)` aceita exatamente um
valor e escolhe o formato por `_Generic`. Não há `MINIMO` variádico nem `SOMA(a, b, c)`.

**Mitigação.** Uma chamada por valor; `ESCREVA_FMT` para formatação fina.

### C-04 — Vírgula dentro de argumento de macro

**Fato (corrigido na Fase 2).** O pré-processador conta parênteses, colchetes e chaves ao
separar argumentos, então `MACRO(f(a, b))` passa **um** argumento — `f(a, b)`. O que
quebra é a **vírgula no nível superior do argumento**: `ESCOLHA(a, b)` vira dois
argumentos, e `POTENCIA(f(a, b), 2)` é lido corretamente porque a vírgula está dentro de
parênteses.

> A Fase 1 afirmava o contrário ("`MACRO(f(a, b))` é interpretado como dois argumentos").
> A sonda da Fase 2 (caso `03_controle/c22_escolha_com_chamada`, no caminho M0 **e** M1)
> mostrou que a afirmação era falsa; a redação foi corrigida aqui, em
> [01 §11.6](01-especificacao.md) e em [03 §6](03-mapeamento-para-c.md).

**Consequência.** Só expressões com vírgula **de nível superior** são frágeis: a lista de
valores de `ESCREVAL(a, b)` (que é erro por desenho, D-07), o operador vírgula
(`ESCOLHA((a, b))` passa, mas é um erro de estilo) e qualquer macro de 2+ argumentos cujo
argumento contenha vírgula solta.

**Mitigação.** (a) os construtores de bloco mais usados (`SE`, `ENQUANTO`) são macros
**objeto**; (b) quando houver vírgula de nível superior, atribua a uma variável
temporária; (c) o prefixo alemão ajuda: `ESCOLHA(t)` com `t` calculado antes;
(d) a mensagem do compilador (`macro "BG_ESCREVAL" passed 2 arguments, but takes just 1`)
está catalogada em [01 §12.3](01-especificacao.md).

### C-05 — `SENAO` + `SE` não encadeiam (descoberto na sonda)

**Fato.** Com `#define SENAO } else {` e `#define SE if (`, a sequência
`SENAO SE c ENTAO` expande para `} else { if ( c ) {`, gerando **um `{` a mais** e
fechando o bloco errado. O erro aparece longe da causa (`expected declaration or
statement at end of input`).

**Consequência.** A linguagem tem a palavra-chave própria **`SENAO_SE`** →
`} else if (`.

**Mitigação.** `SENAO_SE` é a única forma documentada; `SENAO SE` é proibido na
especificação e está na tabela de erros ([01 §12.3](01-especificacao.md)).

### C-06 — Modelo de chaves: o que `FIM_*` resolve e o que ele custa

**Fato.** Se `SE` expandisse para `if` (sem parêntese), o programador precisaria escrever
`SE (cond) {`. Essa forma funciona, mas: (a) obriga chaves em todo lugar; (b) não há como
fechar o bloco com um nome; (c) fica indistinguível de C com palavras trocadas. Se `SE`
expande para `if (`, então **o parêntese da condição já foi aberto** e `{` logo depois é
erro de sintaxe.

**Consequência.** Escolha normativa: `SE` → `if (`, `ENTAO` → `) {`, `FIM_SE` → `}`.
Portanto `SE x { … }` é **proibido** (erro claro: `expected ')' before '{'`). Corpos de
**função** e de **registro/enumeração** usam chaves C, porque ali não existe o problema do
`else` pendurado e porque a assinatura termina em `)`, onde uma palavra-chave de abertura
seria ruído.

**Mitigação.** Regra única, curta e ensinável (P-06); tabela de erros; a escotilha `if` de
C está sempre disponível (P-03).

### C-07 — Diagnósticos degradam quando chaves atravessam macros

**Fato.** No caminho M0, `{` e `}` vêm de macros diferentes. Um `FIM_SE` esquecido produz
erro no **fim do arquivo**, apontando para `FIM`, não para o `SE` sem fechamento. A
coluna e a “dica” do compilador podem citar a definição da macro.

**Consequência.** A especificação documenta os erros previsíveis
([01 §12.3](01-especificacao.md)) e trata a qualidade do diagnóstico como critério de
aceitação do M1, não do M0.

**Mitigação.** M1 emite C sem macros (RG-10) — as mensagens passam a citar `{`/`}` e as
linhas corretas. Ferramenta auxiliar prevista: `basicgolc --checar` apenas para validar
balanceamento de blocos sem compilar.

### C-08 — Não existe higiene nem escopo de macro

**Fato.** `#define` vale até `#undef` ou fim da unidade de tradução; não há namespaces nem
escopo léxico. Uma macro de biblioteca pode sequestrar um identificador do usuário.

**Consequência.** R-1/R-2 (identificadores do usuário com minúsculas; prefixos `bg_`,
`BG_`, `k_`) e o modo `BG_SEM_APELIDOS` (D-08).

**Mitigação.** Nomes de palavra-chave em MAIÚSCULAS nunca colidem com identificadores
válidos de usuário, por construção.

### C-09 — Palavras-chave ocupam o espaço de nomes de C

**Fato.** `SE`, `E`, `OU`, `NAO`, `MOD`, `FIM` passam a ser macros: não podem ser usados
como variáveis, campos, funções ou nomes de tipo, nem em código C incluído depois do
cabeçalho.

**Consequência.** I-3 (`BG_SEM_APELIDOS`) e a lista de reservados
([02 §10](02-referencia.md)). Nomes curtos (`E`, `OU`) são os mais arriscados.

**Mitigação.** Modo prefixado; documentação explícita de que incluir `basicgol.h` num
projeto C existente deve ser feito com `BG_SEM_APELIDOS`.

### C-10 — Dupla avaliação de argumentos

**Fato.** `#define ABS(x) ((x) < 0 ? -(x) : (x))` avalia `x` duas vezes: `ABS(i++)`
incrementa duas vezes.

**Consequência.** P-11/RG-2: `ABS`, `MINIMO`, `MAXIMO` são funções C escolhidas por
`_Generic`; desde a Fase 5, as implementações estão em `src/biblioteca/matematica.c`.
Macros com argumentos só avaliam cada operando **uma vez**.

**Mitigação.** Revisão obrigatória de cada `#define` com argumentos (checklist da Fase 2,
item AC-12).

### C-11 — `_Generic` exige C11 e tem arestas

**Fato.** `_Generic` é C11. Além disso: (a) a expressão de controle sofre conversão de
valor e decaimento de vetor/função, mas **não** remove qualificadores do tipo apontado —
por isso `char *` e `const char *` precisam de associações separadas; (b) enumerações
casam com `int` quando o tipo subjacente é compatível (comportamento observado no GCC);
(c) tipo não listado cai em `default`, se existir; (d) em C++ não existe.
Até a Fase 6, `BG_FMT` (saída) **e** `BG_SFMT` (entrada) tinham `default: "%p"`. O de
saída continua (imprimir ponteiro é útil e não escreve na memória); o de entrada foi
**removido** — sem `default`, um destino sem formato é violação de restrição do `_Generic`
(C11 6.5.1.1p3), ou seja **erro de compilação** em vez de `scanf` gravando 8 bytes num
objeto de 1 byte. Ver DF-01 em [13](13-defeitos-e-correcoes.md).

**Consequência.** C11 é o piso (PT-A); a tabela `BG_FMT` precisa das duas associações de
texto; ponteiros imprimem como `%p`.

**Mitigação.** `ESCREVA_TEXTO` existe para quando o tipo é conhecido e não se quer
depender de `_Generic`; a lista de formatos está documentada; C++ fora de escopo (D-15).

### C-12 — Sem introspecção variádica útil

**Fato.** `__VA_ARGS__` só repassa tokens. Contar argumentos exige maquinaria tipo P99
(dezenas de macros, mensagens de erro ilegíveis).

**Consequência.** `ESCREVA_FMT(...)` repassa direto para `printf` (é a única macro
variádica da linguagem). Nada de `SOMA(a,b,c)`.

### C-13 — Extensão de arquivo própria não é reconhecida por compiladores · **eliminada por D-14**

**Fato (quando a fonte tinha extensão própria, `.bg`).** `cc prog.bg` não reconhece a
extensão: o compilador trata o arquivo como *linker input* ou reclama do formato, e era
preciso `-x c` (GCC/Clang), `/TC` (MSVC) ou um `Makefile` distribuído.

**Consequência.** Deixou de existir na Fase 2: a fonte é **`.c`** (D-14), então `cc
programa.c` simplesmente funciona e nenhuma flag extra é necessária. A restrição fica
registrada porque foi a razão original de se inventar uma extensão — e é justamente o
custo que motivou revertê-la.

**Mitigação.** Nenhuma necessária. `make` e o `Makefile` do repositório continuam
existindo por conveniência (compilar o transpilador), não por obrigação.

### C-14 — Blocos exigem fechamento manual

**Fato.** Não há como a macro “fechar o bloco no fim da indentação”.

**Consequência.** `FIM_SE`, `FIM_ENQUANTO`, `FIM_ESCOLHA`, `FIM_PARA`, `FIM` são
obrigatórios. É a decisão mais visível da linguagem — e a mais compatível com o estilo
Portugol/VisuAlg (`se … fimse`), o que ameniza o custo didático.

### C-15 — Ferramental não conhece as palavras-chave

**Fato.** Editores não realçam `SE`/`FIM_SE`, formatadores não sabem indentar
`FIM_*`, IDE não autocompleta nada.

**Consequência.** Fase 2 deve entregar: arquivos de realce (VS Code, Vim, Notepad++) e uma
regra de indentação simples (palavra-chave de abertura +4, `FIM_*` −4).

**Mitigação.** As `FIM_*` são previsíveis e alinhadas ao abridor, o que torna a
indentação mecânica (e automatizável por script).

### C-16 — Depuração aponta para o C expandido

**Fato.** `gdb` mostra `main` e as linhas reais do `.c` (as macros preservam a linha), mas
um `break` em `FIM_SE` mostra `}`.

**Consequência.** Aceitável; depuração em nível de linha continua funcionando porque as
macros **não** movem linhas.

**Mitigação.** Recomendação: no M1, compilar com `-g` sobre o C gerado e usar
`#line` para mapear de volta ao `.c`.

### C-17 — `TEXTO` é imutável

**Fato.** `const char *` não permite `s[0] = 'x'`; vetores de `char` não podem ser
atribuídos (`v2 = v1` não compila para arrays).

**Consequência.** Dois tipos de texto (`TEXTO` e `TEXTO_FIXO`) e funções de cópia/junção
com verificação de espaço (RB-3).

**Mitigação.** `LEIA_TEXTO`, `COPIA_TEXTO`, `JUNTA_TEXTO` cobrem os casos didáticos.

### C-18 — `sizeof` e decaimento de vetor

**Fato.** `TAMANHO_VETOR(v)` funciona em vetor verdadeiro e **mente** em ponteiro
(devolve o tamanho do ponteiro / do elemento).

**Consequência.** O idioma canônico é `f(vetor, TAMANHO_VETOR(vetor))` na chamada, com
`INTEIRO v[]` + `INTEIRO n` na assinatura.

**Mitigação.** Documentado como erro clássico; M1 pode avisar quando `TAMANHO_VETOR`
recebe parâmetro declarado como `[]`/`*` (W-06, proposto).

### C-19 — `main` duplicado

**Fato.** `INICIO` já define `main`.

**Consequência.** Escrever `int main(void)` num arquivo com `INICIO` é erro de
redefinição; arquivos de biblioteca não têm `INICIO`.

### C-20 — Misturar `scanf` e `fgets` deixa `\n` pendente (descoberto na sonda)

**Fato.** `LEIA(n)` consome o número e **deixa** o `\n` no buffer. O `fgets` seguinte lê
uma linha vazia. O programa “pula” a leitura de texto — o bug mais comum de quem começa
com `scanf`.

**Consequência.** Contrato de `LEIA_TEXTO`: consumir **um** terminador de linha pendente
(se houver) e então ler uma linha inteira. É obrigação da biblioteca, não do aluno.

**Segunda iteração (a primeira tentativa de correção estava errada).** A primeira versão
do contrato dizia “descartar o resto da linha corrente antes de ler”. Isso passou nos
testes que misturam `LEIA` + `LEIA_TEXTO` e **falhou** no caso oposto: com `stdin` limpo,
o `getchar` de descarte consumiu a primeira linha digitada e o programa abortou com
“falha ao ler a frase”. A versão correta usa espiada (`getchar` + `ungetc`) e só consome
o terminador quando o próximo caractere **é** o terminador. O exemplo
`contador_vogais.c` (entrada limpa) e `cadastro.c` (entrada numérica antes) são os dois
testes obrigatórios de regressão.

**Mitigação.** Exemplo `cadastro.c` e `contador_vogais.c` exercitam exatamente esse
caso; teste de regressão obrigatório na Fase 2.

### C-21 — Construtor de bloco não pode ser usado em expressão

**Fato.** `SE`/`ENTAO` produzem tokens de comando; usá-los em expressão
(`x = SE … ENTAO …`) não compila.

**Consequência.** Não há `if` como expressão, nem operador ternário em palavra-chave. Para
valor condicional: declare antes e atribua em cada ramo (E-5).

### C-22 — Palavras-chave não podem ser testadas em `#if`

**Fato.** `#if SE …` não faz sentido: `SE` expande para `if (`.

**Consequência.** Configuração de compilação usa apenas `BG_*`. Documentado em
[02 §9](02-referencia.md).

### C-23 — `CONSTANTE` não serve como tamanho de vetor

**Fato.** `CONSTANTE` expande para `static const`. Em C (ao contrário de C++), um objeto
`const` **não** é expressão constante de compilação. Logo
`VETOR(Aluno, turma, k_max_alunos)` expande para `Aluno turma[k_max_alunos]`, que dentro de
uma função é um **VLA** — alocado na pilha em tempo de execução, sem aviso com
`-Wall -Wextra`.

**Consequência.** O tamanho de `VETOR` deve ser um literal inteiro ou uma macro de
`#define`. `CONSTANTE` continua válida para limites, comparações e constantes matemáticas
(`k_pi`), mas não para dimensão de vetor.

**Mitigação.** Documentado em [01 §7.2](01-especificacao.md) e em [03 §6](03-mapeamento-para-c.md);
`cadastro.c` usa `VETOR(Aluno, turma, 30)` com comentário explicando o motivo. A Fase 2
pode emitir `W-07` quando `VETOR`/`TEXTO_FIXO` recebem um identificador `k_*` como
tamanho (o transpilador vê os tokens e pode avisar).

### C-24 — Não existe `string` de tamanho gerenciado, e `TEXTO` não é dono da memória

**Fato.** C não tem tipo de texto com tamanho e dono. `TEXTO` é `const char *`: um
ponteiro para bytes terminados por `'\0'`. O comprimento é uma varredura (`strlen`);
o espaço é responsabilidade do programador.

**Consequência.** A linguagem não pode oferecer concatenação com `+`, comparação com `==`
por conteúdo, nem redimensionamento. `TEXTO_FIXO` tem limite fixo; `TAMANHO_TEXTO` conta
bytes, não caracteres.

**Mitigação.** Biblioteca de texto segura (RB-3/RB-9): todas as operações com destino
recebem o tamanho e reportam truncamento; `TEXTOS_IGUAIS`/`COMPARA_TEXTO` para conteúdo;
`malloc` explícito quando o tamanho é dinâmico. Tudo documentado em
[06 §2.5 e §8](06-tipos-e-expressoes.md).

### C-25 — Não há operador de resto real nem potência por símbolo

**Fato.** Em C, `%` só aceita inteiros e `^` é XOR bit a bit. A linguagem não pode
sobrecarregar operadores (isso exigiria reescrever expressões — C-01).

**Consequência.** `7 MOD 2.5` é erro de compilação; `2 ^ 3` vale 1 (XOR), não 8. Não há
`**`.

**Mitigação.** `POTENCIA(a, b)` para potência; para resto real, `fmod` de `<math.h>` ou
`x - PISO(x / y) * y`. `DIVIDE`/`RESTO` cobrem o caso inteiro verificado. Testes:
`06_negativos/n14`, `09_tipos_expr/t07`.

### C-26 — O compilador não verifica o formato de `FORMATA_TEXTO`

**Fato.** Fazer o compilador conferir a string de formato exige
`__attribute__((format(printf, …)))`, que é extensão de GCC/Clang; a linguagem é C11
estrito (fora da seção opcional).

**Consequência.** Um `FORMATA_TEXTO(d, t, "%d", "texto")` compila e falha em execução.

**Mitigação.** `ESCREVA_FMT` mantém a checagem do compilador quando o formato é literal;
`FORMATA_TEXTO` existe para quando o formato precisa ser uma variável. Documentado em
[06 §8](06-tipos-e-expressoes.md).

---

## 2. Resumo por consequência

| Restrição | O que ela custou à linguagem |
|---|---|
| C-01 | `VETOR(…)`, `PARA(i,a,b)`; `PARA i DE …` só em M1 |
| C-02 | `;` obrigatório em comando simples |
| C-03, C-12 | um valor por `ESCREVA`; só `ESCREVA_FMT` é variádica |
| C-04 | macros objeto para `SE`/`ENQUANTO`; proibição de vírgula em `ESCOLHA` |
| C-05 | palavra-chave extra `SENAO_SE` |
| C-06 | `ENTAO`/`FACA`; chaves em função/registro; `SE x { }` proibido |
| C-07 | tabela de erros documentada; M1 como resposta de qualidade |
| C-08, C-09 | R-1/R-2, `BG_SEM_APELIDOS` |
| C-10 | `ABS`/`MINIMO`/`MAXIMO` como funções C para avaliação única |
| C-11 | C11 obrigatório; `ESCREVA_TEXTO` como alternativa |
| C-13 | **eliminada**: a fonte já é `.c` (D-14), então o compilador a reconhece sem `-x c`; helpers ainda exigem link com o archive |
| C-14 | `FIM_*` (herança feliz do estilo VisuAlg) |
| C-17, C-18 | `TEXTO`/`TEXTO_FIXO`; idioma vetor+tamanho |
| C-20 | contrato de `LEIA_TEXTO` |
| C-23 | tamanho de vetor precisa ser literal (ou `#define`), não `CONSTANTE` |
| C-24 | biblioteca de texto com tamanho explícito e truncamento reportado |
| C-25 | `POTENCIA` no lugar de `^`; `MOD` só em inteiros |
| C-26 | `ESCREVA_FMT` para formato conferido pelo compilador |

---

## 3. Restrições que a especificação **não** aceita (linhas vermelhas)

Alguns “atalhos” foram considerados e rejeitados. Ficam registrados para não voltarem:

| Rejeitado | Por quê |
|---|---|
| transpilador com parser C completo desde a Fase 2 | escopo explode; o valor está em macros + um reescritor de tokens |
| macros que redefinem `main`, `if`, `while` (nomes C) | sequestraria o C do usuário e violaria P-03 |
| `#define ESCREVA(...)` com heurística por `__VA_ARGS__` | mensagens de erro ilegíveis e comportamento surpresa |
| typedef de `int` para `INTEIRO` via `#define INTEIRO int` como **tipo** com sufixo | não compila (`int64_t` etc. resolvem) |
| usar `setjmp/longjmp` para simular exceções | não determinístico, destrói RAII mental, ensina errado |
| `goto` no vocabulário | incentiva código espaguete; C nativo continua disponível |
| indentação significativa | impossível sem parser; contrário a C |

---

## 4. Premissas de ambiente

A especificação assume, e a Fase 2 deve verificar:

| ID | Premissa | Se não valer |
|---|---|---|
| A-1 | Compilador com C11 completo (`_Generic`, `_Static_assert`, `_Noreturn`, `<stdint.h>`) | `ESCREVA`/`LEIA` degradam; a linguagem perde o perfil N de E/S |
| A-2 | `stdin`/`stdout` de console (terminal ou redirecionamento de arquivo) | E/S continua correta, mas interatividade muda (sem eco, sem prompt) |
| A-3 | `int` com pelo menos 32 bits na prática | programas didáticos que usam `INTEIRO` para contadores grandes podem estourar (PT-E) |
| A-4 | `char` com 8 bits e texto em UTF-8 | `TAMANHO_TEXTO` conta bytes, não caracteres acentuados |
| A-5 | `locale` "C" (ponto decimal) porque ninguém chama `setlocale` | ver D-17 |
| A-6 | `-lm` disponível quando separada | `POTENCIA`/`RAIZ`/`PISO`/`TETO`/`ARREDONDA` não ligam |
| A-7 | Compilação de um único arquivo `.c` (ou vários, via linha de comando) | sem sistemas de módulos (D-18) |

---

## 5. Decisões

Formato: **ID — título — status — decisão — justificativa — o que reabriria**.

### D-01 — Dois caminhos, uma linguagem · **decidido**
M0 (macros, perfil N) e M1 (`basicgolc`, perfis N+X) compartilham a mesma fonte.
*Justificativa*: entrega valor imediato (M0) sem congelar a linguagem num teto baixo
(M1). *Reabriria*: se M1 se mostrar inviável em 3 semanas de Fase 2 — nesse caso a
linguagem fica só com o perfil N e o `PARA` com açúcar é removido.

### D-02 — Chaves em corpos de função/registro · **decidido**
`FUNCAO tipo nome(params) { … }` e `REGISTRO(N) … FIM_REGISTRO(N);`.
*Justificativa*: a assinatura termina em `)`, onde uma palavra-chave de abertura seria
ruído; não há risco de `else` pendurado. *Reabriria*: se testes com iniciantes mostrarem
confusão entre "bloco com `FIM_*`" e "bloco com `{ }`" — a alternativa é uma palavra
`INICIO_FUNCAO`.

### D-03 — Acentuação de palavras-chave · **decidido e implementado (Fase 2)**
`SENAO`, `LOGICO`, `ATE`, `NAO` **sem** acento no perfil N; a documentação sempre mostra a
leitura acentuada. *Justificativa*: C não garante identificadores não-ASCII (R-3), e a
portabilidade vale mais que a estética.

**Fechamento (Fase 2).** O M1 aceita as formas acentuadas na entrada — `INÍCIO`, `ENTÃO`,
`SENÃO`, `SENÃO_SE`, `FAÇA`, `ATÉ_QUE`, `LÓGICO`, `NÃO`, `VETOR`… — e reescreve para ASCII
antes de traduzir, porque vê bytes UTF-8. A tabela de equivalência é `APELIDOS[]` em
`tools/basicgolc.c` e a normalização acontece uma única vez, num pré-passe
(`normaliza_apelidos`), de modo que **todos** os diagnósticos já trabalham com a forma
ASCII. O perfil N continua exigindo ASCII puro: o mesmo arquivo com acentos **não**
compila em M0 (R-3), e a suíte cobre os dois lados (`09` aceita em M1; `12_negativos_m1`
não depende de acento).

### D-04 — Sintaxe livre (sem parênteses, sem `;`, indentação significativa) · **rejeitado**
*Justificativa*: exigiria um parser completo — exatamente o que a Fase 1 demonstrou ser
desnecessário. Ver C-01, C-02.

### D-05 — Ponto e vírgula opcional · **rejeitado para v1.0**
*Justificativa*: C-02. *Reabriria*: só com transpilador com gramática de instruções —
custo alto, ganho didático duvidoso (todo o material de referência usa `;`).

### D-06 — Modelo de erros · **decidido**
Sem exceções. Valor de retorno + `AFIRME` + `ERRO`. *Justificativa*: C-12, §13.

### D-07 — Impressão automática de vários valores · **rejeitado (definitivo na Fase 2)**
`ESCREVAL(a, b, c)` não existe; use uma chamada por valor ou `ESCREVA_FMT`.
*Justificativa*: C-03.

**Fechamento (Fase 2).** O M1 *poderia* transformar `ESCREVAL(a, b)` em duas chamadas,
mas isso quebraria a promessa que sustenta a linguagem: **a mesma fonte tem a mesma
semântica nos dois perfis** (AC-06/AC-32). Uma construção que compila em M1 e não compila
em M0 cria exatamente a assimetria que o desenho quis evitar (por isso o açúcar
`PARA i DE …` é declarado **X**, só M1, e nunca esconde um erro de M0). O erro de
`ESCREVAL(a, b)` continua sendo um diagnóstico de projeto, testado em
`06_negativos/n04_escreval_dois_valores`.

### D-08 — Nomes curtos versus prefixados · **decidido (mantido na Fase 2)**
Padrão: nomes curtos (`SE`, `E`, `OU`). Opção: `#define BG_SEM_APELIDOS` antes do
`#include` deixa apenas `BG_SE`, `BG_ENTAO`, … *Justificativa*: legibilidade é o produto;
prefixo em tudo mataria a proposta. *Aberto*: se a experiência de uso em projetos C
mostrar colisões frequentes, inverter o padrão na v2.0 (grande quebra — exigiria
major). Mitigação imediata: documentar I-3 como prática obrigatória ao incluir
`basicgol.h` em C existente.

### D-09 — Modo estrito (`BG_ESTRITO`) · **adiado para v1.1**
Ideia: com `BG_ESTRITO`, `ESCREVA`/`LEIA` deixariam de usar `_Generic` frouxo e
verificariam melhor os tipos (por exemplo, `_Static_assert` de que o argumento é escalar
conhecido). Custo: complexidade e mensagens piores. **Recomendação**: adiar; entregar
como opt-in na v1.1 se houver demanda.

### D-10 — `=` como igualdade (estilo BASIC) · **rejeitado**
`=` atribui, `==` compara (é C). Mitigação: `IGUAL(a,b)`/`DIFERENTE(a,b)` e aviso W-01 do
M1. *Justificativa*: mudar isso exigiria reescrever expressões (C-01) e ensinaria um
operador que não existe em nenhuma outra linguagem derivada de C.

### D-11 — Nome do terminador do `REPITA` · **decidido**
`ATE_QUE(cond);` no núcleo; `ATE cond;` como açúcar M1. *Justificativa*: `ATE_QUE(c)`
funciona como macro de função que fecha o `do/while`; `ATE` fica livre para o açúcar de
`PARA … ATE …`, evitando ambiguidade para o autômato do transpilador.

### D-12 — Funções de vetor na biblioteca · **fora da v1.0 (candidato a v1.1)**
`COPIA_VETOR`, `ORDENA_VETOR`, `BUSCA_VETOR` não estão em v1.0 (ficaram fora para manter a
biblioteca mínima). **Recomendação**: `ORDENA_VETOR(v, n)` é didaticamente valioso; incluir
como `static inline` com `qsort` na v1.1, e **não** incluir `COPIA_VETOR` (ensinar o laço
tem valor).

### D-13 — Empacotamento · **revisado na Fase 5**
**Decisão original (Fase 2).** A sintaxe BASICGOL ficou em `basicgol.h`, sem etapa de
build própria; na época os helpers eram `static inline` no mesmo arquivo. O custo de
manter I/O, conversões, matemática e texto num cabeçalho monolítico cresceu com a Fase 4.

**Decisão atual (Fase 5).** A sintaxe, tipos e aliases continuam em `include/basicgol.h`;
a biblioteca padrão fica em headers por módulo (`include/basicgol/`) e implementações
C11 em `src/biblioteca/`, ligadas como `build/libbasicgol.a`. Isso separa as interfaces,
permite compilar/testar cada responsabilidade e mantém as dependências em libc/libm,
sem instalar `.so` nem pacote de terceiros. O Makefile oferece `make biblioteca`; também
é possível compilar os `.c` diretamente. A API didática em maiúsculas permanece
compatível, mas programas que chamam helpers agora precisam ligar a implementação.

### D-14 — Extensão do arquivo-fonte · **decidido: `.c`** (revisto na Fase 2)
A recomendação original era `.bg` ("distintivo, comunica que não é C de verdade"). O dono
do projeto reverteu, e a razão é boa: **todo `.c` do BASICGOL já é um C válido** — é o
mesmo arquivo com `#include <basicgol.h>` no topo. Dar-lhe outra extensão só criava
atrito:

* `cc programa.c` reconhece o fonte sem `-x c`; `make`, `CMakeLists.txt` e
  `compile_commands.json` funcionam sem regra extra (helpers exigem linkar a biblioteca);
* editores, realce de sintaxe, formatadores, `clangd` e `gcc -E` tratam o arquivo como C,
  que é exatamente o que ele é;
* quem sai da linguagem leva os arquivos junto: não há "migração" de `.bg` para `.c`.

Alternativas descartadas: `.bg` (atrito acima), `.bgol` (mais digitação, mesmo atrito) e
`.por` (colide com nomes de build). A distinção entre "fonte BG" e "C comum" passa a ser
de conteúdo, não de nome: um fonte BG inclui `basicgol.h` e usa as palavras-chave; um
módulo C puro não. `Makefile`, `basicgolc` e a suíte aceitam qualquer nome, então a
decisão é reversível sem tocar em código.

### D-19 — `LEIA` de caractere pula espaços · **decidido na Fase 2**
A tabela `BG_SFMT` usa `" %c"` para `char`/`signed char`. Sem isso, `LEIA(n); LEIA(c);`
leria o `'\n'` deixado pelo número e o programa "perderia" a letra — a armadilha clássica
de `scanf("%c")`. *Custo*: não dá para ler um espaço ou uma quebra de linha como
caractere; quem precisar usa C (`getchar()`). *Teste*:
`05_entrada_saida/i15_leia_caractere_apos_numero`.

### D-20 — `TAMANHO_TEXTO`/`TAMANHO_VETOR` devolvem `NATURAL` · **decidido na Fase 2**
São `size_t` (é o que `sizeof`/`strlen` dão), e misturar com `INTEIRO` dispara
`-Wsign-compare` — que é bom: ensina a usar `NATURAL` para tamanhos. O idioma canônico é

```c
NATURAL n = TAMANHO_TEXTO(s);
NATURAL i = 0;
ENQUANTO i < n FACA
    ...
```

*Teste*: o caso `08_aplicacao/app_acervo` usa exatamente esse idioma em `le_id`, e a suíte
roda com `-Wall -Wextra -Werror`.

### D-21 — Queda intencional em `ESCOLHA` · **decidido na Fase 2**
`-Wextra` transforma `-Wimplicit-fallthrough` em erro, e o comentário `/* fall through */`
não é reconhecido quando o `case` vem de macro. Decisão: queda intencional é permitida,
mas o projeto anota o caso com o recurso do compilador (`__attribute__((fallthrough));`)
ou desliga só esse aviso. O caso `03_controle/c19_escolha_fallthrough` faz a segunda
coisa e documenta o motivo no próprio fonte.

### D-15 — C++ · **rejeitado para v1.0**
`_Generic` não existe em C++ (C-11); `bool`, `const char *` e conversões mudam de
comportamento. Declarar C++ como não suportado é mais honesto do que quase-suportar.

### D-16 — Idioma das mensagens · **decidido**
Mensagens de `ERRO`/`AFIRME` e do transpilador em **português sem acento** por padrão
(independente da codificação do terminal). *Aberto*: um modo `BG_IDIOMA=en` para
bibliotecas reutilizáveis — provavelmente desnecessário; reavaliar se houver uso fora do
Brasil.

### D-17 — `locale` numérico · **decidido: nunca chamar `setlocale`**
`scanf("%lf")`/`printf("%g")` dependem de `LC_NUMERIC`: em locale `pt_BR.UTF-8`, o
separador decimal é `,`. Os programas BG **não** chamam `setlocale`, logo herdam o locale
"C" do início do processo (ponto decimal) — comportamento descrito como normativo.
**Fechamento (Fase 2).** `basicgol.h` **não** chama `setlocale`: a biblioteca nunca muda
o locale do processo, então `%g`/`%lf` usam sempre `.` como separador decimal e o
comportamento não depende do ambiente do aluno. Quem quiser locale do sistema chama
`setlocale` explicitamente no próprio programa (é C, é visível no fonte). O oposto — a
biblioteca "adivinhar" o locale — foi rejeitado porque tornaria a saída dos testes
dependente da máquina. Consequência documentada em [01 §16](01-especificacao.md).

### D-18 — Módulos e pacotes · **rejeitado para v1.0**
`#include` de C, mais `EXTERNO`. Sem sistema de módulos (§13.3).

### D-22 — Extensões de compilador · **opcionais, isoladas e desligadas**

Toda a linguagem é C11 estrito, exceto um bloco: `#ifdef BG_USAR_EXTENSOES` (§2b de
`basicgol.h`), que oferece aritmética verificada (`SOMA_SEGURA`, `SUBTRAI_SEGURA`,
`MULTIPLICA_SEGURA`) sobre `__builtin_*_overflow` de GCC/Clang. Desligada, nenhum símbolo
existe, nada muda e o cabeçalho compila com `-Wpedantic`.

*Justificativa*: detecção de estouro em `+ - *` não é possível com macro portátil; a
alternativa seria (a) não oferecer, (b) prometer wrap falso, ou (c) usar extensão
escondida. (c) é a única que viola portabilidade sem avisar. A escolha foi oferecer,
explicitamente, atrás de uma macro.

*Custo*: cobre só `INTEIRO` (o caso didático) e não funciona em MSVC. Quem precisa de
tipos largos usa `INTEIRO64` e valida a faixa manualmente. *Teste*:
`09_tipos_expr/t13_extensoes` (ligada) e `01_sanidade` (desligada por padrão).

### D-23 — `DIVIDE`/`RESTO` verificados em vez de mudar `/` e `MOD` · **decidido na Fase 4**

`/` e `MOD` continuam sendo o C cru (P-09): a linguagem documenta o UB de divisor zero
e de quociente assinado não representável (em geral `MIN/-1`), mas não muda o operador.
Para quem quer a checagem, existem `DIVIDE` e `RESTO`, que fazem a mesma conta com o
tipo promovido dos operandos e chamam `bg_erro(__FILE__, __LINE__)` no caso inválido.

*Justificativa*: mudar `/` quebraria a promessa “expressões são expressões C” e
desligaria otimizações/idiomas de C. A forma verificada é opt-in e explícita na leitura.
*Alternativa rejeitada*: `DIVIDE` retornar um `LOGICO` com ponteiro de saída (ruído em
toda expressão aritmética). *Testes*: `t07` (válido), `t08`, `t09`, `t10` (erro + código
de saída 1).

### D-24 — Conversões verificadas em vez de casts que “funcionam” · **decidido na Fase 4**

`TEXTO_PARA_INTEIRO`, `TEXTO_PARA_NATURAL`, `TEXTO_PARA_REAL` e `REAL_PARA_INTEIRO`
devolvem `LOGICO`. Casts continuam permitidos, com semântica de C.

*Justificativa*: o cast real→inteiro fora de faixa é UB, e texto→número por `atoi` é
indefinido em erro. Devolver `LOGICO` obriga o programador a decidir o que fazer. As
funções antigas foram endurecidas (antes `TEXTO_PARA_INTEIRO("2147483648")` devolvia
`VERDADEIRO` com valor truncado — bug corrigido e fixado em `t06`).

### D-25 — Texto novo = `ACRESCENTA`/`TRECHO`/`FORMATA`, sem “mini-biblioteca de string”

*Justificativa*: quatro operações cobrem o uso didático (copiar, juntar, acrescentar,
recortar) mais formatação verificável. Busca/substituição de substring ficam para C
(`strstr`), porque uma API própria exigiria dono de memória (C-24). *Contrato*: RB-3/RB-9
— truncamento sempre reportado, destino limpo na falha.

### D-26 — Helpers de caractere são ASCII e não usam `<ctype.h>`

`E_DIGITO`, `E_LETRA`, `CARACTERE_MAIUSCULO`, `CARACTERE_MINUSCULO` operam só em ASCII.

*Justificativa*: as funções de `<ctype.h>` dependem de `locale` e exigem argumento
convertido para `unsigned char`; usar `toupper` em `'ç'` daria resultados diferentes por
ambiente. ASCII é exato, testável e suficiente para o que `CARACTERE` representa (um
byte). *Consequência*: acentos não são convertidos — use `TEXTO` e uma biblioteca de
Unicode, de forma explícita.

### D-27 — Versão 1.1.0 e compatibilidade

A Fase 4 é **aditiva**: nenhum construtor existente mudou de expansão, exceto
`TEXTO_PARA_INTEIRO`/`TEXTO_PARA_REAL`, que passaram a **recusar** estouro de faixa
(antes devolviam `VERDADEIRO` com valor truncado). Isso é correção de contrato (RB-7),
não mudança de sintaxe: programas que tratavam a falha corretamente não mudam; programas
que dependiam do valor truncado estavam errados. Junto vem a correção de um **defeito**
de `LEIA`: `BG_SFMT` não tinha formato para `unsigned char*`, `unsigned short*` nem
`long double*`, então `LEIA` de `NATURAL8`, `NATURAL16`, `NATURAL_CURTO` e `REAL_LONGO`
caía no `default` (`"%p"`) e gravava um ponteiro sobre a variável. Agora cada tipo
escalar tem o seu formato (`%hhu`, `%hu`, `%hu`, `%Lf`).

**Revisto na Fase 6 (DF-01).** `LOGICO` era recusado só pelo `-Wformat`, isto é, por um
**aviso** — e a linha de compilação publicada no README não usa `-Werror`. O `default`
de `BG_SFMT` foi removido: hoje `LEIA(LOGICO)`, `LEIA(TEXTO)` e `LEIA(TEXTO_FIXO(...))`
são erro de compilação por violação de restrição, com ou sem `-Werror`, nos dois
compiladores testados. Ver §9.1 de [06](06-tipos-e-expressoes.md) e os casos
`06_negativos/n16_leia_logico`, `n21_leia_texto` e `n22_leia_texto_fixo`. A versão sobe para 1.1.0 (minor), conforme §16.1 de
[01](01-especificacao.md).

### D-28 — Contrato e escopo da biblioteca padrão · **decidido na Fase 5**

A biblioteca é dividida em módulos portáveis (console, matemática, texto, caractere,
utilidades) com nomes públicos `bg_*`; os nomes de linguagem `BG_*`/curtos permanecem
como aliases de compatibilidade. Falhas recuperáveis retornam `bool`; parâmetros de
saída numéricos não mudam em falha; buffers de texto têm capacidade explícita, não
retêm conteúdo parcial e continuam pertencendo ao chamador. `bg_erro` e os macros
legados de divisão/resto/absoluto são as exceções fatais documentadas.

A escolha é manter o core em C11 + libc, com libm quando separada, sem dependência de
terceiros ou helpers de sistema operacional. Não há semeadura implícita; o sorteio usa
`rand()` e declara seu limite `RAND_MAX + 1`, previsibilidade e qualidade não garantida.
Funções específicas de plataforma, se necessárias no futuro, terão header/fonte separados
e não serão incluídos por `<basicgol/biblioteca.h>`. O contrato completo está em
[09 — Biblioteca padrão](09-biblioteca-padrao.md); cada função pública é exercitada em
`tests/test_biblioteca.c`.

---

## 6. Registro da sonda de viabilidade (Fase 1)

**Objetivo**: provar que os construtores do perfil N funcionam, antes de escrevê-los na
especificação. **Natureza**: descartável; não versionada; **não** é a implementação
(entrega da Fase 2).

**Ambiente**: GCC 12.2 (Debian), `-std=c11 -Wall -Wextra -Wformat=2 -lm`.

**O que foi validado (compilou sem avisos e executou com a saída esperada)**:

| # | Item | Resultado |
|---|---|---|
| S-1 | `INICIO`/`FIM` gerando `main` | ok |
| S-2 | `SE`/`ENTAO`/`SENAO`/`FIM_SE`, inclusive aninhados | ok |
| S-3 | `SENAO_SE` encadeado | ok (após correção de C-05) |
| S-4 | `ENQUANTO`/`FACA`/`FIM_ENQUANTO` | ok |
| S-5 | `REPITA`/`ATE_QUE(c);` | ok |
| S-6 | `ESCOLHA`/`CASO`/`CONTRARIO`/`FIM_ESCOLHA` + `PARE` | ok |
| S-7 | `PARA(i,a,b)` e `PARA_PASSO(i,a,b,p)` | ok |
| S-8 | `VETOR`, `TAMANHO_VETOR`, `TEXTO_FIXO`, vetor de `TEXTO` | ok |
| S-9 | `REGISTRO`/`FIM_REGISTRO` + campos + vetor de registros | ok |
| S-10 | `ENUMERACAO`/`FIM_ENUMERACAO` | ok |
| S-11 | `CONSTANTE INTEIRO` = `static const int` (dentro e fora de função) | ok |
| S-12 | `FUNCAO` vazio + tipo; `PROCEDIMENTO` = `void`; `RETORNE` | ok |
| S-13 | `ESCREVA`/`ESCREVAL` com `_Generic` para `int`, `char`, `double`, `bool`, `TEXTO`, enum, `size_t` | ok |
| S-14 | `LEIA` com `_Generic` sobre `&(x)` para `int`/`double` | ok |
| S-15 | `E`/`OU`/`NAO`/`MOD`/`VERDADEIRO`/`FALSO` | ok |
| S-16 | `ABS`/`MINIMO`/`POTENCIA`/`RAIZ` com avaliação única | ok |
| S-17 | C nativo misturado (`printf`, laço `for`, `if`) no mesmo arquivo | ok |

**O que falhou e mudou o projeto**:

| Descoberta | Correção incorporada |
|---|---|
| `SENAO SE` gera `{` extra (C-05) | nova palavra-chave `SENAO_SE` |
| `LEIA` seguido de `LEIA_TEXTO` lê `"\n"` (C-20) | contrato: `bg_leia_texto` consome **um** terminador pendente, via espiada (`getchar`/`ungetc`) |
| 1ª versão do contrato (descartar até o fim da linha) engolia a primeira linha quando nada estava pendente | contrato corrigido: consome só se o próximo caractere for o terminador; dois testes de regressão |
| `TEXTO_PARA_INTEIRO("4x2")` devolvia `VERDADEIRO` (o `strtol` para no primeiro caractere inválido) | contrato endurecido: exige consumo do texto inteiro; sobra → `FALSO` (RB-7) |
| `ESCREVAL(x)` com `printf(BG_FMT(x) "\n", …)` não compila (macro de `_Generic` não é literal de string) | `ESCREVAL` usa sequência `(printf(…), fputc('\n', stdout))` |
| `LEIA(x);` como comando gera aviso `value computed is not used` | `(void)(…)` na definição de `LEIA`; `LEIA_OK` devolve o valor |
| `SE (x) { … }` não compila (C-06) | proibido na especificação; `ENTAO` obrigatório |

**Conclusão**: o perfil N da especificação é integralmente implementável com macros, e a
Fase 2 pode começar por ele com risco técnico baixo.

---

## 7. Riscos

| ID | Risco | Prob. | Impacto | Mitigação |
|---|---|---|---|---|
| R-01 | Mensagens de erro do M0 afastam iniciantes | alta | alto | tabela de erros (§01-12.3), `basicgolc` cedo, realce de sintaxe |
| R-02 | Colisão de palavras-chave em projeto C existente | média | alto | `BG_SEM_APELIDOS`, documentado como obrigatório em interop |
| R-03 | Aluno escreve `SENAO SE` / `SE x {` e se perde | alta | médio | documentação + futura mensagem do M1 + exercícios |
| R-04 | Escopo cresce (classes, exceções, módulos) | média | alto | §1.3 não-objetivos + linhas vermelhas (§3) |
| R-05 | M1 vira um compilador de fato e atrasa a entrega | média | alto | limite de 4 atividades no autômato (§4.2 do 01); C nativo como escape |
| R-06 | Divergência semântica M0 × M1 | baixa | alto | AC-06 (comparação de saídas) na Fase 2 |
| R-07 | `_Generic` com tipos inesperados (enum, ponteiro) imprime errado sem aviso | média | médio | `ESCREVA_TEXTO`/`ESCREVA_FMT`, documentação, D-09 |
| R-08 | Dependência de `-lm` em Windows/MinGW | baixa | baixo | PT-C documentado; exemplos testados em Linux e MinGW |
