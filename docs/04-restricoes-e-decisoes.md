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

**Fato.** `MACRO(f(a, b))` é interpretado como dois argumentos, salvo parênteses extras
(`MACRO((f(a, b)))`) — que nem sempre preservam a semântica desejada.

**Consequência.** Macros de função que recebem **expressões** (não listas) são pontos
frágeis: `ESCOLHA(f(a,b))`, `PARA(i, 0, g(x,y))`, `POTENCIA(f(a,b), 2)`.

**Mitigação.** (a) os construtores de bloco mais usados (`SE`, `ENQUANTO`) são macros
**objeto** justamente para não sofrer disso; (b) a especificação manda atribuir a uma
variável temporária quando houver vírgula; (c) os exemplos evitam o caso; (d) mensagem de
erro documentada em [01 §12.3](01-especificacao.md).

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

**Consequência.** P-11/RG-2: `ABS`, `MINIMO`, `MAXIMO` são **funções `inline`** escolhidas
por `_Generic`; macros com argumentos só os usam **uma vez**.

**Mitigação.** Revisão obrigatória de cada `#define` com argumentos (checklist da Fase 2,
item AC-12).

### C-11 — `_Generic` exige C11 e tem arestas

**Fato.** `_Generic` é C11. Além disso: (a) a expressão de controle sofre conversão de
valor e decaimento de vetor/função, mas **não** remove qualificadores do tipo apontado —
por isso `char *` e `const char *` precisam de associações separadas; (b) enumerações
casam com `int` quando o tipo subjacente é compatível (comportamento observado no GCC);
(c) tipo não listado cai em `default` (`%p`) **sem aviso**; (d) em C++ não existe.

**Consequência.** C11 é o piso (PT-A); a tabela `BG_FMT` precisa das duas associações de
texto; ponteiros imprimem como `%p`.

**Mitigação.** `ESCREVA_TEXTO` existe para quando o tipo é conhecido e não se quer
depender de `_Generic`; a lista de formatos está documentada; C++ fora de escopo (D-15).

### C-12 — Sem introspecção variádica útil

**Fato.** `__VA_ARGS__` só repassa tokens. Contar argumentos exige maquinaria tipo P99
(dezenas de macros, mensagens de erro ilegíveis).

**Consequência.** `ESCREVA_FMT(...)` repassa direto para `printf` (é a única macro
variádica da linguagem). Nada de `SOMA(a,b,c)`.

### C-13 — `.bg` não é reconhecido por compiladores

**Fato.** `cc prog.bg` trata o arquivo como linker input (ou não sabe o que fazer).

**Consequência.** Compilar exige `-x c` (GCC/Clang), `/TC` (MSVC) ou um `Makefile`
distribuído. Documentado em PT-I.

**Mitigação.** O manual da Fase 2 deve trazer um `Makefile`/`CMakeLists.txt` de modelo e
um script `bgcc` de uma linha.

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

**Fato.** `gdb` mostra `main` e as linhas reais do `.bg` (as macros preservam a linha), mas
um `break` em `FIM_SE` mostra `}`.

**Consequência.** Aceitável; depuração em nível de linha continua funcionando porque as
macros **não** movem linhas.

**Mitigação.** Recomendação: no M1, compilar com `-g` sobre o C gerado e usar
`#line` para mapear de volta ao `.bg`.

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
`contador_vogais.bg` (entrada limpa) e `cadastro.bg` (entrada numérica antes) são os dois
testes obrigatórios de regressão.

**Mitigação.** Exemplo `cadastro.bg` e `contador_vogais.bg` exercitam exatamente esse
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
`cadastro.bg` usa `VETOR(Aluno, turma, 30)` com comentário explicando o motivo. A Fase 2
pode emitir `W-07` quando `VETOR`/`TEXTO_FIXO` recebem um identificador `k_*` como
tamanho (o transpilador vê os tokens e pode avisar).

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
| C-10 | `ABS`/`MINIMO`/`MAXIMO` como funções `inline` |
| C-11 | C11 obrigatório; `ESCREVA_TEXTO` como alternativa |
| C-13 | documento de compilação obrigatório na Fase 2 |
| C-14 | `FIM_*` (herança feliz do estilo VisuAlg) |
| C-17, C-18 | `TEXTO`/`TEXTO_FIXO`; idioma vetor+tamanho |
| C-20 | contrato de `LEIA_TEXTO` |
| C-23 | tamanho de vetor precisa ser literal (ou `#define`), não `CONSTANTE` |

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
| A-7 | Compilação de um único arquivo `.bg` (ou vários, via linha de comando) | sem sistemas de módulos (D-18) |

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

### D-03 — Acentuação de palavras-chave · **decidido (núcleo), aberto (M1)**
`SENAO`, `LOGICO`, `ATE`, `NAO` sem acento no perfil N; a documentação sempre mostra a
leitura acentuada. *Justificativa*: C não garante identificadores não-ASCII (R-3), e a
portabilidade vale mais que a estética. *Aberto*: M1 **pode** aceitar `SENÃO`, `LÓGICO`,
`ATÉ` na entrada e reescrever para a forma ASCII — é seguro porque o transpilador vê
bytes UTF-8. Decidir na Fase 2, com sua própria tabela de equivalência.

### D-04 — Sintaxe livre (sem parênteses, sem `;`, indentação significativa) · **rejeitado**
*Justificativa*: exigiria um parser completo — exatamente o que a Fase 1 demonstrou ser
desnecessário. Ver C-01, C-02.

### D-05 — Ponto e vírgula opcional · **rejeitado para v1.0**
*Justificativa*: C-02. *Reabriria*: só com transpilador com gramática de instruções —
custo alto, ganho didático duvidoso (todo o material de referência usa `;`).

### D-06 — Modelo de erros · **decidido**
Sem exceções. Valor de retorno + `AFIRME` + `ERRO`. *Justificativa*: C-12, §13.

### D-07 — Impressão automática de vários valores · **rejeitado para v1.0**
`ESCREVAL(a, b, c)` não existe; use uma chamada por valor ou `ESCREVA_FMT`.
*Justificativa*: C-03. *Reabriria*: se a Fase 2 entregar M1 estável — o transpilador
**poderia** transformar `ESCREVAL(a, b)` em duas chamadas, pois vê os tokens. Fica
registrado como candidato a v1.1 (é o único açúcar em que M1 realmente agrega conforto).

### D-08 — Nomes curtos versus prefixados · **decidido, com ponto em aberto**
Padrão: nomes curtos (`SE`, `E`, `OU`). Opção: `#define BG_SEM_APELIDOS` antes do
`#include` deixa apenas `BG_SE`, `BG_ENTAO`, … *Justificativa*: legibilidade é o produto;
prefixo em tudo mataria a proposta. *Aberto*: se a experiência de uso em projetos C
mostrar colisões frequentes, inverter o padrão na v2.0 (grande quebra — exigiria
major). Mitigação imediata: documentar I-3 como prática obrigatória ao incluir
`basicgol.h` em C existente.

### D-09 — Modo estrito (`BG_ESTRITO`) · **aberto**
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

### D-12 — Funções de vetor na biblioteca · **aberto**
`COPIA_VETOR`, `ORDENA_VETOR`, `BUSCA_VETOR` não estão em v1.0 (ficaram fora para manter a
biblioteca mínima). **Recomendação**: `ORDENA_VETOR(v, n)` é didaticamente valioso; incluir
como `static inline` com `qsort` na v1.1, e **não** incluir `COPIA_VETOR` (ensinar o laço
tem valor).

### D-13 — Empacotamento · **aberto (com recomendação)**
Recomendação: `basicgol.h` **header-only** (um arquivo, sem build), com `basicgolc` como
binário separado opcional. Alternativa rejeitada: biblioteca `.a`/`.so` (atrito de
instalação em laboratório de escola).

### D-14 — Extensão do arquivo-fonte · **aberto (com recomendação)**
Recomendação: `.bg`. Alternativas: `.bgol` (mais distintivo, mais digitação), `.por`.
Custo de mudar depois: baixo (documentação + Makefile). Decidir até o fim da Fase 2.

### D-15 — C++ · **rejeitado para v1.0**
`_Generic` não existe em C++ (C-11); `bool`, `const char *` e conversões mudam de
comportamento. Declarar C++ como não suportado é mais honesto do que quase-suportar.

### D-16 — Idioma das mensagens · **decidido**
Mensagens de `ERRO`/`AFIRME` e do transpilador em **português sem acento** por padrão
(independente da codificação do terminal). *Aberto*: um modo `BG_IDIOMA=en` para
bibliotecas reutilizáveis — provavelmente desnecessário; reavaliar se houver uso fora do
Brasil.

### D-17 — `locale` numérico · **aberto**
`scanf("%lf")`/`printf("%g")` dependem de `LC_NUMERIC`: em locale `pt_BR.UTF-8`, o
separador decimal é `,`. Os programas BG **não** chamam `setlocale`, logo herdam o locale
"C" do início do processo (ponto decimal) — comportamento descrito como normativo.
*Aberto*: se a Fase 2 quiser respeitar o locale do sistema, precisa de uma decisão
explícita e de testes com `,` decimal.

### D-18 — Módulos e pacotes · **rejeitado para v1.0**
`#include` de C, mais `EXTERNO`. Sem sistema de módulos (§13.3).

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
