# BASICGOL — Relatório de testes da Fase 3 (controle de fluxo)

**Resultado: 294 verificações, 294 aprovadas, 0 reprovadas** (GCC 12.2, `-std=c11` e
`-std=c17`, `-Wall -Wextra -Wformat=2 -Wvla -Werror`). Antes da fase, a suíte tinha 244
verificações; a fase acrescentou 50 sem remover nenhuma.

Ambiente: Debian 12, GCC 12.2.0. **Clang não estava instalado neste ambiente** e não foi
executado localmente; a matriz GCC × Clang × c11 × c17 do CI continua sendo a validação
oficial para Clang (ver §5).

Comandos executados:

```sh
make                              # constrói build/basicgolc
make test                         # suíte completa      -> TUDO OK: 294
T_CFLAGS="-std=c17 -Wall -Wextra -Wformat=2 -Wvla -Werror" ./tests/run.sh   # -> TUDO OK: 294
make exemplos                     # 19 exemplos compilados com -Werror
make cobertura                    # regenera tests/COBERTURA.md (CI verifica que está atualizado)
```

---

## 1. Resultado por seção

| Seção | Verificações | Aprovadas | Reprovadas |
|---|---:|---:|---:|
| 01 — sanidade (cabeçalho, C11, expansões AC-22, avaliação única) | 19 | 19 | 0 |
| casos `02_basico` | 14 | 14 | 0 |
| casos `03_controle` (c01–c40) | 77 | 77 | 0 |
| casos `04_estruturas` | 22 | 22 | 0 |
| casos `05_entrada_saida` | 30 | 30 | 0 |
| casos `07_multiarquivo` | 4 | 4 | 0 |
| casos `08_aplicacao` | 2 | 2 | 0 |
| casos `09_tipos_expr` | 30 | 30 | 0 |
| 06 — negativos, erros do compilador C (M0) | 20 | 20 | 0 |
| 06b — negativos, diagnósticos do transpilador (M1) | 14 | 14 | 0 |
| 06c — avisos W-01…W-07 | 7 | 7 | 0 |
| 09 — transpilador M1 | 15 | 15 | 0 |
| 10 — exemplos (M0 e M1 contra a saída dourada) | 36 | 36 | 0 |
| 11 — estresse | 4 | 4 | 0 |
| **Total** | **294** | **294** | **0** |

Na seção `03_controle`, 37 casos rodam nos dois caminhos (M0 e M1) e 3 (c15, c16, c17) só
no M1, por serem açúcar. Por isso são 77 verificações e não 80.

## 2. Verificações novas desta fase (50)

| Caso | O que prova | M0 | M1 |
|---|---|:-:|:-:|
| `03_controle/c24_zero_iteracoes` | `PARA`, `PARA_PASSO`, `ENQUANTO` com zero iterações; `REPITA` roda uma vez | ✔ | ✔ |
| `c25_limites_inclusivos` | extremos incluídos; passo que não cai no fim; faixa com negativos | ✔ | ✔ |
| `c26_passo_zero_erro` | passo zero em execução: `PARA_PASSO: passo zero`, código 1 | ✔ | ✔ |
| `c27_passo_zero_intervalo_vazio` | passo zero é erro mesmo com intervalo vazio | ✔ | ✔ |
| `c28_aninhados_pare_prossiga` | `PARE` e `PROSSIGA` afetam só o laço mais interno | ✔ | ✔ |
| `c29_saida_de_dois_lacos` | saída de dois laços com bandeira; caso sem solução termina | ✔ | ✔ |
| `c30_repita_prossiga` | `PROSSIGA` em `REPITA` vai ao teste (sem laço infinito) | ✔ | ✔ |
| `c31_enquanto_prossiga` | `PROSSIGA` em `ENQUANTO` com contador já incrementado | ✔ | ✔ |
| `c32_para_prossiga_avanca` | `PROSSIGA` em `PARA` avança a variável; valor final `fim + 1` | ✔ | ✔ |
| `c33_condicoes_aninhadas` | quatro combinações de `SE` aninhado; cada `SENAO` no seu `SE` | ✔ | ✔ |
| `c34_faixas_senao_se` | cadeia `SENAO_SE` com faixas e `OU`; primeiro ramo verdadeiro vence | ✔ | ✔ |
| `c35_escolha_no_laco` | `PARE` sai só do `ESCOLHA`; `PROSSIGA` vai ao próximo `i`; rótulos agrupados | ✔ | ✔ |
| `c36_laco_entrada_sentinela` | leitura até a sentinela; segunda rodada com zero iterações; `LEIA_INTEIRO` | ✔ | ✔ |
| `c37_laco_interno_vazio` | laço interno vazio não encerra o externo | ✔ | ✔ |
| `c38_virgula_em_enquanto` | vírgula de nível superior na condição de `ENQUANTO` | ✔ | ✔ |
| `c39_senao_com_se_aninhado` | padrão recomendado para `SE` aninhado dentro de `SENAO` | ✔ | ✔ |
| `c40_passo_sem_sinal` | passo `NATURAL` sem avisos sob `-Wextra -Wpedantic` | ✔ | ✔ |
| `06_negativos/n17_pare_fora_de_laco` | `PARE` fora de laço é recusado pelo C | ✔ | — |
| `n18_prossiga_fora_de_laco` | `PROSSIGA` sem laço é recusado | ✔ | — |
| `n19_ate_que_com_virgula` | vírgula de nível superior em `ATE_QUE` é recusada | ✔ | — |
| `n20_escolha_com_virgula` | vírgula de nível superior em `ESCOLHA` é recusada | ✔ | — |
| `12_negativos_m1/t12_passo_zero_literal` | `PASSO 0` literal recusado pelo transpilador | — | ✔ |
| `t13_senao_sem_se` | `SENAO` sem `SE` aberto recusado | — | ✔ |
| `t14_ate_que_sem_ponto_e_virgula` | `ATE_QUE(c)` sem `;` recusado pelo transpilador | — | ✔ |
| `10_exemplos` `controle_condicoes` | condições aninhadas, `FUNCAO TEXTO`, sinal e paridade | ✔ | ✔ |
| `controle_lacos` | tabela com `PROSSIGA`, Collatz (`ENQUANTO`), `REPITA`, bandeira | ✔ | ✔ |
| `controle_acumuladores` | soma, produto, contagem, maior/menor, média `REAL`, laço vazio | ✔ | ✔ |
| `controle_entrada` | validação com `REPITA`, sentinela, zero iterações | ✔ | ✔ |
| `controle_para_acucar` | açúcar `PARA i DE … FACA`, `PASSO` negativo, `ATE` em `REPITA` | — | ✔ |

Contagem: 17 casos × 2 = 34; negativos 4 + 3 = 7; exemplos 4 × 2 + 1 = 9. Total 50.

## 3. Defeitos encontrados nesta fase

Três defeitos reais, todos observados **antes** da correção e fixados por teste depois.

| # | Defeito | Como apareceu (antes) | Correção | Teste que fixa |
|---|---|---|---|---|
| D1 | `PARA_PASSO` com passo 0 **não termina** | `PARA_PASSO (i, 10, 1, 0) { … }` sem `PARE`: a execução foi interrompida por `timeout` (código 124 após 3 s) | a condição chama `bg_passo_zero` (erro com linha, código 1); expansão igual para `p ≠ 0`; M1 usa a mesma forma | `c26`, `c27` |
| D2 | M1 **aceitava `ATE_QUE(cond)` sem `;`** | `ATE_QUE(a > 3)` seguido de outra instrução: o `basicgolc` saiu com código 0 e o erro só apareceria no C. A documentação do próprio README afirmava que o transpilador acusava isso | verificação em `basicgolc`: o `;` precisa vir logo após o `)` | `t14` |
| D3 | Mensagem de `SENAO SE` enganosa | dizia só "use SENAO_SE", que é errado para quem queria um `SE` aninhado dentro de `SENAO` | a mensagem agora diz como escrever o `SE` aninhado | `t02` (prefixo preservado) e `c39` (padrão recomendado) |

Erro de autoria, também corrigido: o caso `c36` foi escrito inicialmente com `SENAO` seguido
de `SE`. O M1 recusou, corretamente, pela regra CND-2. O programa é que estava fora da
especificação, não o transpilador.

## 4. Divergência conhecida (documentada, não corrigida)

**`SENAO` seguido de `SE` (dois tokens)** é aceito pelo M0 como bloco aninhado e recusado
pelo M1 pela regra CND-2. As macros não conseguem recusar essa sequência, porque ela é C
válido. Não foi corrigido porque a regra é normativa; o comportamento está em
[`07-controle-de-fluxo.md` §6.2](07-controle-de-fluxo.md) e o padrão recomendado (`c39`)
passa nos dois caminhos.

## 5. O que não foi verificado

| Item | Situação |
|---|---|
| Clang (CI: `gcc` e `clang` × `c11` e `c17`) | não executado localmente; as variantes de texto dos negativos foram escritas para GCC e Clang, mas só a execução no CI confirma |
| MSVC / MinGW | não executado |
| Revisão humana (AC-17, AC-18 de [05](05-criterios-de-aceitacao.md)) | não feita nesta fase |
| Desempenho de laços | não medido; a suíte não tem teste de tempo |
| Versão da linguagem | mantida em **1.1.0**: a mudança de passo zero é correção de comportamento indefinido, mas subir a versão é decisão de quem publica |

## 6. Como reproduzir

```sh
make && make test
./tests/run.sh -v 03_controle     # os casos de controle, um a um
./tests/run.sh -v 06_negativos   # negativos M0 e M1
make exemplos
```

Cada caso de execução é compilado e rodado nos dois caminhos e comparado à saída dourada;
a falha de qualquer um deles reprova o caso.
