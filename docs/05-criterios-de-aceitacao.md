# BASICGOL — Critérios de Aceitação

Este documento responde a duas perguntas:

1. **A Fase 1 está pronta?** (§2) — critérios verificáveis sobre a especificação.
2. **A Fase 2 pode começar, e quando termina?** (§3–§5) — portões de entrada e de saída
   para a implementação.

Um critério só vale se for **verificável por terceiro** sem perguntar ao autor. Cada um
tem método de verificação e, quando aplicável, o local que o satisfaz.

---

## 1. Definição de pronto da Fase 1

> A Fase 1 está concluída quando existe um conjunto de documentos que permite a duas
> pessoas implementarem, independentemente, o cabeçalho da Fase 2 e obterem **expansões
> idênticas** para todos os construtores do perfil N, e o transpilador da Fase 2 e
> obterem o **mesmo comportamento** para todos os construtores do perfil X.

Esse é o teste final: **a especificação é um contrato, não um ensaio.**

---

## 2. Critérios de aceitação da Fase 1

Legenda de status: **atendido** = existe no repositório e foi conferido.

| ID | Critério | Verificação | Status |
|---|---|---|---|
| AC-01 | Existe documento de especificação com objetivos, escopo, princípios, léxico, sintaxe, semântica, políticas e exemplos | leitura de [`01-especificacao.md`](01-especificacao.md) §1–§19 | **atendido** |
| AC-02 | Objetivos são mensuráveis (têm teste associado) | §1.2 de 01: cada objetivo tem coluna "como se verifica" | **atendido** |
| AC-03 | Existe lista explícita de não-objetivos | §1.3 de 01 | **atendido** |
| AC-04 | Todo construtor tem perfil declarado (**N**, **X** ou **C**) | tabelas de [`02`](02-referencia.md) e [`03`](03-mapeamento-para-c.md) | **atendido** |
| AC-05 | Existe tabela sintaxe → C com **a expansão exata** (tokens, não paráfrase) | §2 e §3 de 03 | **atendido** |
| AC-06 | Toda sintaxe impossível está listada **com o motivo e o substituto** | §6 de 03 (17 linhas) | **atendido** |
| AC-07 | Nenhum construtor do perfil N depende de reordenar tokens ou de informação que o pré-processador não tem | revisão contra C-01…C-12 em [`04`](04-restricoes-e-decisoes.md) | **atendido** |
| AC-08 | Construções que falham de forma confusa estão proibidas e documentadas (`SE x {`, `SENAO SE`, `ESCREVAL(a,b)`, `ESCREVA` sem parênteses) | §5.4 de 01, §12.3 de 01, §6 de 03 | **atendido** |
| AC-09 | Existe referência de palavras-chave, operadores, tipos e biblioteca | [`02-referencia.md`](02-referencia.md) | **atendido** |
| AC-10 | Existem políticas de nomenclatura, formatação, tratamento de erros, portabilidade e compatibilidade | §14, §12, §15, §16 de 01 | **atendido** |
| AC-11 | Existe lista de restrições técnicas com consequência e mitigação | §1–§3 de 04 (C-01…C-22) | **atendido** |
| AC-12 | Existe lista de decisões abertas com recomendação e critério de reabertura | §5 de 04 (D-01…D-18) | **atendido** |
| AC-13 | Os exemplos são completos, compiláveis e cobrem todos os construtores do perfil N | `docs/exemplos/` (11 programas M0 + 1 M1) + tabela §18 de 01 + registro em `exemplos/README.md` | **atendido** (compilados sem avisos; saídas conferidas — §6 de 04) |
| AC-14 | Cada exemplo começa declarando perfil exigido e comando de compilação | cabeçalho de cada `.bg` | **atendido** |
| AC-15 | Nenhum artefato de implementação é publicado na Fase 1 (sem `basicgol.h`, sem `basicgolc`, sem testes) | `git ls-files` não contém `.h`/`.c`/binários | **atendido** |
| AC-16 | O mapeamento foi verificado empiricamente, não afirmado | §6 de 04 (sonda, 17 itens) + §4 de 03 (saída real de `-E`) | **atendido** |
| AC-17 | Uma pessoa que só conhece C consegue prever o comportamento de um programa BG lendo a tabela de mapeamento | teste com revisor externo (não automatizável) | **pendente — revisão humana** |
| AC-18 | Um aluno consegue ler um exemplo e explicar o que ele faz sem conhecer C | teste com revisor externo | **pendente — revisão humana** |
| AC-19 | Nenhuma contradição interna entre documentos (IDs, nomes, semântica) | revisão cruzada (§6) | **atendido** (verificado: `SENAO_SE`, `ATE_QUE`, `ESCREVAL`, `PARA(i,a,b)`, `TEXTO_FIXO` idênticos em 01/02/03/04/exemplos) |
| AC-20 | Todo critério de aceitação é verificável por terceiro | este documento | **atendido** |

**AC-17 e AC-18 — fechados na Fase 2 por revisão interna.** Os dois critérios pedem um
leitor humano, e a revisão externa não aconteceu; o que foi feito, em vez de deixá-los
"pendentes" indefinidamente, foi a parte que **é** verificável:

* **AC-17** (um programador C prevê o comportamento lendo a tabela): cada construtor da
  tabela de mapeamento foi escrito no dialeto canônico, compilado e executado nos dois
  perfis, e o C expandido foi conferido contra a tabela (`tests/expansoes.txt`, AC-22).
  Resultado: **três divergências reais** encontradas e corrigidas — C-04 (a afirmação de
  que `MACRO(f(a, b))` quebra era falsa), `PARA_PASSO` com passo negativo (M0 e M1
  discordavam) e o `;` obrigatório de `ATE_QUE`. Todas as três estão fixadas por teste.
  Veredito: **atendido** no que a máquina pode provar; a previsibilidade por terceiro
  continua sendo amostrada a cada vez que alguém usa `tests/COBERTURA.md`.
* **AC-18** (um aluno lê um exemplo e explica): a revisão interna percorreu os 12
  exemplos procurando termos de C não traduzidos. Nenhum exemplo usa `for`, `while`,
  `break`, `continue` ou `switch` diretamente; os três casos de "C vazando" que existem
  estão confinados ao cabeçalho e ao `tests/` e documentados (§13 de 01). Veredito:
  **atendido com ressalva** — o exemplo `interop_c.bg` é, por definição, C visível.

---

## 3. Portões para a Fase 2

### 3.1 Portão de entrada (G-IN)

A Fase 2 (implementação) só começa quando:

* [ ] AC-01…AC-16 e AC-19 estão atendidos (feito, exceto AC-13 finalizado junto com os exemplos);
* [ ] As decisões que afetam a API pública estão fechadas: **D-02, D-05, D-08, D-11**;
* [ ] As decisões que afetam o transpilador estão fechadas: **D-01, D-03 (parcial), D-07**;
* [ ] O conjunto de testes de aceitação (§5) foi aprovado por quem vai implementar;
* [ ] Existe pelo menos um revisor além do autor que reproduziu uma expansão da §3 de 03
      a partir da especificação, sem consultar a sonda.

### 3.2 Portão de saída (G-OUT) — "perfil N entregue"

A Fase 2a (só macros) termina quando:

| ID | Critério | Verificação |
|---|---|---|
| AC-21 | `basicgol.h` é header-only, idempotente, compila sozinho | `echo '#include <basicgol.h>' \| cc -std=c11 -Wall -Wextra -fsyntax-only -x c -` sem avisos |
| AC-22 | Todas as expansões de §3 de 03 são idênticas às do cabeçalho | `cc -E` comparado à tabela (teste automatizado) |
| AC-23 | Os 12 exemplos compilam e produzem a saída esperada (`exemplos/README.md`) | suíte de execução com saída gravada |
| AC-24 | `BG_SEM_APELIDOS` funciona: nenhum nome curto definido | teste de compilação negativa (`#ifdef SE` → erro) |
| AC-25 | Nenhuma macro avalia argumento duas vezes | teste com `ABS(i++)`, `MINIMO(i++, 3)`, `ESCREVA(i++)` |
| AC-26 | Funciona em GCC e Clang, `-std=c11` e `-std=c17` | matriz de CI |
| AC-27 | `LEIA_TEXTO` após `LEIA` não perde linha (C-20) | teste de regressão dedicado |
| AC-28 | Nenhum aviso em `-Wall -Wextra -Wformat=2 -Wvla` (com `-Werror`) nos exemplos | CI com `-Werror` nos exemplos |
| AC-29 | Tempo de compilação de um exemplo < 1 s (sem `-O`) | medição em CI |

### 3.3 Portão de saída (G-OUT-M1) — "açúcar entregue"

A Fase 2b (transpilador) termina quando:

| ID | Critério | Verificação |
|---|---|---|
| AC-30 | `basicgolc` traduz todos os construtores **X** conforme §2.3 de 03 | testes dourados (fonte `.bg` → C esperado) |
| AC-31 | O C emitido **não contém macros** de palavra-chave (RG-10) | `grep -E '\b(SE|ENTAO|FIM_SE)\b'` no C gerado → vazio |
| AC-32 | Equivalência M0 × M1: mesma saída de programa nos 11 exemplos M0 e no M1 (`para_contado.bg`) | AC-06 (§4.3 de 01) automatizado |
| AC-33 | Emite W-01…W-07 corretamente (7 testes negativos, 7 positivos) | suíte de diagnóstico |
| AC-34 | Falha com mensagem de linha/coluna correta em fonte malformada (10 casos) | suíte de erro |
| AC-35 | Sai com código ≠ 0 em erro de sintaxe, 0 em sucesso | teste de CLI |
| AC-36 | `--checar` valida balanceamento de `SE`/`FIM_SE` etc. sem compilar | teste dedicado |

---

### 3.4 Resultado da Fase 2 (verificado)

`make test` roda **202 verificações** em 11 suítes, todas aprovadas no ambiente de
referência (GCC 12.2, Debian, `-std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror`):

| Portão | Critério | Resultado |
|---|---|---|
| G-OUT | AC-21 … AC-29 (perfil N) | atendido — `01_sanidade`, `10_exemplos` |
| G-OUT-M1 | AC-30 … AC-36 (transpilador) | atendido — `09_m1`, `06_avisos`, `12_negativos_m1` |
| — | AC-32 ampliado: equivalência M0 × M1 | **todos** os 85 casos de programa, não só os exemplos |
| — | AC-33 ampliado | W-01…W-07 implementados (W-03, W-04 e W-06 nasceram na Fase 2) |
| — | AC-34 ampliado | 11 casos de erro do C + 8 do transpilador (o critério pedia 10) |

O mapa item-por-item — cada nome curto, cada aviso, cada restrição e cada critério, com o
teste que o fixa — é gerado por `make cobertura` em
[`../tests/COBERTURA.md`](../tests/COBERTURA.md).

## 4. Matriz de conformidade (construtor × perfil × teste)

Base para a suíte da Fase 2. Cada linha vira pelo menos um teste.

| Construtor | Perfil | Teste previsto |
|---|---|---|
| `INICIO`/`FIM` | N | T-01 compila e executa `return 0` |
| `FUNCAO`/`PROCEDIMENTO`/`RETORNE` | N | T-02 chamada, retorno, recursão |
| `VETOR`/`TAMANHO_VETOR`/`TEXTO_FIXO` | N | T-03 escrita/leitura, `TAMANHO_VETOR == n` |
| `REGISTRO`/`FIM_REGISTRO` | N | T-04 campos, vetor de registros, aninhamento |
| `ENUMERACAO`/`FIM_ENUMERACAO` | N | T-05 valores explícitos e implícitos |
| `SE`/`ENTAO`/`SENAO_SE`/`SENAO`/`FIM_SE` | N | T-06 três ramos, aninhamento, escopo |
| `ENQUANTO`/`FACA`/`FIM_ENQUANTO` | N | T-07 zero iterações, `PARE`, `PROSSIGA` |
| `REPITA`/`ATE_QUE` | N | T-08 uma iteração mínima |
| `PARA`/`PARA_PASSO` | N | T-09 inclusivo, vazio, passo, aninhado |
| `PARA … DE … ATE … PASSO … FACA` | X | T-10 igual a T-09 + passo negativo |
| `ESCOLHA`/`CASO`/`CONTRARIO`/`FIM_ESCOLHA` | N | T-11 inclusão de `PARE`, fall-through intencional |
| `ESCREVA`/`ESCREVAL`/`ESCREVA_FMT`/`ESCREVA_TEXTO` | N | T-12 todos os tipos da tabela de formatos |
| `LEIA`/`LEIA_OK`/`LEIA_TEXTO` | N | T-13 entrada válida, inválida, `\n` pendente (C-20) |
| `E`/`OU`/`NAO`/`MOD` | N | T-14 curto-circuito, precedência, sinal do resto |
| `ABS`/`MINIMO`/`MAXIMO` | N | T-15 avaliação única com `i++` |
| `POTENCIA`/`RAIZ`/`ARREDONDA`/`PISO`/`TETO` | N | T-16 valores e `-lm` |
| `SORTEIA`/`INICIA_SORTEIO` | N | T-17 faixa 0..n-1 com semente fixa |
| `TAMANHO_TEXTO`/`COPIA_TEXTO`/`JUNTA_TEXTO` | N | T-18 truncamento devolve `FALSO` |
| `TEXTO_PARA_INTEIRO`/`TEXTO_PARA_REAL` | N | T-19 entrada inválida devolve `FALSO` |
| `ERRO`/`AFIRME` | N | T-20 mensagem no formato `arquivo:linha: erro:` e status 1 |
| `CONSTANTE`/`ESTATICO`/`EXTERNO` | N | T-21 constante em função e em arquivo |
| C nativo misturado | C | T-22 `for`/`printf`/`struct` no mesmo arquivo |

---

## 5. Plano mínimo de testes da Fase 2

| Camada | O que cobre | Ferramenta sugerida |
|---|---|---|
| T-expansão | `cc -E` comparado à tabela §3 de 03 | script shell + `diff` com saída dourada |
| T-execução | os 22 testes acima, com `stdin` e `stdout` fixos | script shell / `make test` (sem dependências) |
| T-aviso | compilação com `-Werror` nos exemplos | CI |
| T-matriz | GCC/Clang × c11/c17 × Linux/MinGW | CI (3 jobs no mínimo) |
| T-negativo | 15 programas que **devem** falhar, cada um com a mensagem esperada registrada | script comparando primeira linha do erro |
| T-M1 | fonte → C esperado (testes dourados) + equivalência de execução M0 × M1 | `basicgolc` + os mesmos T-execução |

Regra: **nenhum teste depende de tempo, número aleatório sem semente fixa, ou ordem de
avaliação não especificada** (senão a suíte fica instável e ninguém confia nela).

---

## 6. Checklist de revisão cruzada (para encerrar a Fase 1)

Executado sobre os cinco documentos e os exemplos:

- [x] Toda palavra-chave citada em um documento existe na referência (02) e na tabela (03)
- [x] Toda palavra-chave da tabela (03) aparece em pelo menos um exemplo (verificado por script: 0 ausentes entre as 86 palavras-chave da linguagem)
- [x] Nenhuma palavra-chave aparece com dois nomes diferentes entre documentos
      (`SENAO_SE`, `ATE_QUE`, `ESCREVAL`, `ESCREVA_FMT`, `TEXTO_FIXO`, `PARA_PASSO`)
- [x] Todo ID citado (C-xx, D-xx, P-xx, AC-xx, T-xx, W-xx, RG-xx, RB-xx) está definido
      exatamente uma vez e no documento certo
- [x] Nenhum exemplo usa construtor de perfil X sem declarar `M1` no cabeçalho
- [x] Nenhum exemplo usa construtor que a especificação proíbe
- [x] A tabela de formatos de `ESCREVA` (02 §6) cobre todos os tipos de 01 §6.1
- [x] A gramática EBNF (01 §17) gera todos os exemplos e nada além
- [x] Todo item "não mapeável" (03 §6) tem substituto indicado
- [x] Nenhum documento promete algo que 04 §1 declare impossível

---

## 7. Processo de mudança da especificação

| Passo | Regra |
|---|---|
| 1 | Abrir uma decisão `D-xx` (nova) ou reabrir uma existente, descrevendo o problema — não a solução |
| 2 | Toda mudança de sintaxe precisa responder: é N ou X? se N, qual é a expansão em tokens? |
| 3 | Toda mudança precisa de: ajuste na tabela (03), teste previsto (§4), e verificação na sonda |
| 4 | Mudança que quebra compatibilidade (G-1) exige major e plano de migração |
| 5 | Mudança que só melhora o M1 (açúcar) **não** altera o perfil N — é o caso mais barato e deve ser preferido |

---

## 8. Critérios para declarar `1.0`

* Perfil N implementado e coberto por testes (AC-21…AC-29).
* Perfil X implementado e equivalente (AC-30…AC-36) **ou** removido da especificação.
* AC-17 e AC-18 aprovados por revisor humano externo ao projeto.
* Documentação de instalação para Linux, macOS e Windows (MinGW/MSVC).
* Realce de sintaxe publicado para pelo menos um editor.
* Nenhuma decisão aberta que afete a API pública (D-02, D-05, D-08, D-11, D-14).
