# Cobertura de testes — BASICGOL

> Documento **gerado** por `python3 tools/cobertura.py` (ou `make cobertura`).
> Gerado em 2026-11-05. Não edite à mão: as seções 1–3 saem da árvore de testes.

## 0. Como rodar

```sh
make                 # compila o transpilador em build/basicgolc
make test            # suíte completa (M0 + M1 em cada caso)
make test-v          # idem, listando cada verificação aprovada
./tests/run.sh 03    # uma suíte só (prefixo do nome)
```

Cada caso de execução roda **duas vezes** — caminho M0 (macros do cabeçalho) e
caminho M1 (transpilador) — e só passa se os dois produzirem exatamente a saída
esperada. Hoje são **87 casos em disco**; o total de verificações
(impresso no fim de `make test`) inclui também as checagens de sanidade, de linha de
comando, os 7 avisos, os 19 casos negativos e as 4 provas de estresse.

Legenda das suítes nas seções seguintes: os diretórios `tests/casos/*` são executados
pelo `run.sh`; as suítes `01_sanidade`, `09_m1`, `10_exemplos` e `11_estresse` são
blocos de shell dentro do próprio `run.sh` (não aparecem como diretório).

## 1. Superfície da linguagem (nomes curtos do cabeçalho)

Todos os **88** nomes curtos aparecem em pelo menos um arquivo de teste.

| nome | onde é exercitado |
|---|---|
| `ABS` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/casos/07_multiarquivo/m01_modulos/matematica.bg`, `tests/run.sh` |
| `AFIRME` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/05_entrada_saida/i13_afirme_ok/programa.bg`, `tests/casos/05_entrada_saida/i14_afirme_dispara/programa.bg` |
| `ARREDONDA` | `tests/casos/08_aplicacao/app_acervo/acervo.bg` |
| `ATE_QUE` | `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c09_repita_uma_vez/programa.bg`, `tests/casos/03_controle/c10_repita_ate_que/programa.bg` |
| `CARACTERE` | `tests/casos/02_basico/b04_tipos/programa.bg`, `tests/casos/02_basico/b05_caractere/programa.bg`, `tests/casos/05_entrada_saida/i01_formatos/programa.bg`, `tests/casos/05_entrada_saida/i04_leia_tipos/programa.bg` (+1) |
| `CASO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c18_escolha_pare/programa.bg`, `tests/casos/04_estruturas/e07_enumeracao/programa.bg` (+4) |
| `CONSTANTE` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/13_avisos/w07_tamanho_constante/programa.bg`, `tests/run.sh` |
| `CONTRARIO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c18_escolha_pare/programa.bg`, `tests/casos/04_estruturas/e07_enumeracao/programa.bg` (+4) |
| `COPIA_TEXTO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/casos/04_estruturas/e03_texto_buffer/programa.bg`, `tests/casos/05_entrada_saida/i09_texto_truncamento/programa.bg` |
| `DIFERENTE` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `E` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg` (+5) |
| `ENQUANTO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c06_enquanto_zero/programa.bg`, `tests/casos/03_controle/c07_enquanto_pare/programa.bg` (+5) |
| `ENTAO` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/02_basico/b05_caractere/programa.bg`, `tests/casos/03_controle/c02_se_senao/programa.bg` (+30) |
| `ENUMERACAO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/04_estruturas/e07_enumeracao/programa.bg`, `tests/casos/12_negativos_m1/t10_enumeracao_com_chaves/programa.bg`, `tests/casos/12_negativos_m1/t10_enumeracao_com_chaves/esperado_erro.txt` |
| `ERRO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/05_entrada_saida/i12_erro_fatal/programa.bg` |
| `ESCOLHA` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c18_escolha_pare/programa.bg` (+5) |
| `ESCREVA` | `tests/casos/03_controle/c03_senao_se/programa.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c14_para_passo/programa.bg`, `tests/casos/04_estruturas/e04_registro/programa.bg` (+18) |
| `ESCREVAL` | `tests/casos/02_basico/b01_ola/programa.bg`, `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/02_basico/b04_tipos/programa.bg`, `tests/casos/02_basico/b05_caractere/programa.bg` (+72) |
| `ESCREVA_ERRO` | `tests/casos/08_aplicacao/app_acervo/principal.bg` |
| `ESCREVA_FMT` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/05_entrada_saida/i03_fmt/programa.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg` (+8) |
| `ESCREVA_LINHA` | `tests/casos/03_controle/c03_senao_se/programa.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c14_para_passo/programa.bg`, `tests/casos/03_controle/c11_para_basico/programa.bg` (+10) |
| `ESCREVA_TEXTO` | `tests/casos/04_estruturas/e06_vetor_registros/programa.bg`, `tests/casos/04_estruturas/e05_registro_aninhado/programa.bg`, `tests/casos/05_entrada_saida/i07_leia_texto_limpo/programa.bg`, `tests/casos/05_entrada_saida/i02_escreval_linha_texto/programa.bg` (+2) |
| `ESTATICO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/07_multiarquivo/m02_estatico_misto/contas.bg`, `tests/lib.sh` |
| `EXTERNO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/07_multiarquivo/m01_modulos/comum.h`, `tests/casos/07_multiarquivo/m02_estatico_misto/contas.h`, `tests/lib.sh` |
| `FACA` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c15_para_acucar/programa.bg`, `tests/casos/03_controle/c06_enquanto_zero/programa.bg` (+9) |
| `FALSO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/07_multiarquivo/m01_modulos/matematica.bg`, `tests/casos/05_entrada_saida/i09_texto_truncamento/programa.bg` (+2) |
| `FIM` | `tests/casos/02_basico/b01_ola/programa.bg`, `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/02_basico/b04_tipos/programa.bg`, `tests/casos/02_basico/b05_caractere/programa.bg` (+84) |
| `FIM_ENQUANTO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c06_enquanto_zero/programa.bg`, `tests/casos/03_controle/c07_enquanto_pare/programa.bg` (+5) |
| `FIM_ENUMERACAO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/04_estruturas/e07_enumeracao/programa.bg`, `tests/casos/12_negativos_m1/t10_enumeracao_com_chaves/programa.bg` |
| `FIM_ESCOLHA` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c18_escolha_pare/programa.bg`, `tests/casos/04_estruturas/e07_enumeracao/programa.bg` (+4) |
| `FIM_REGISTRO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/04_estruturas/e04_registro/programa.bg`, `tests/casos/04_estruturas/e06_vetor_registros/programa.bg`, `tests/casos/04_estruturas/e05_registro_aninhado/programa.bg` (+2) |
| `FIM_SE` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/02_basico/b05_caractere/programa.bg`, `tests/casos/03_controle/c02_se_senao/programa.bg` (+29) |
| `FUNCAO` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/07_multiarquivo/m01_modulos/comum.h` (+10) |
| `IGUAL` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/casos/07_multiarquivo/m01_modulos/matematica.bg` |
| `INICIA_SORTEIO` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `INICIO` | `tests/casos/02_basico/b01_ola/programa.bg`, `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/02_basico/b04_tipos/programa.bg`, `tests/casos/02_basico/b05_caractere/programa.bg` (+84) |
| `INTEIRO` | `tests/casos/02_basico/b04_tipos/programa.bg`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/02_basico/b05_caractere/programa.bg` (+61) |
| `INTEIRO16` | `tests/casos/02_basico/b04_tipos/programa.bg` |
| `INTEIRO32` | `tests/casos/02_basico/b04_tipos/programa.bg` |
| `INTEIRO64` | `tests/casos/02_basico/b04_tipos/programa.bg`, `tests/casos/04_estruturas/e08_recursao/programa.bg`, `tests/casos/05_entrada_saida/i01_formatos/programa.bg`, `tests/run.sh` |
| `INTEIRO8` | `tests/casos/02_basico/b04_tipos/programa.bg` |
| `INTEIRO_CURTO` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `INTEIRO_LONGO` | `tests/casos/05_entrada_saida/i01_formatos/programa.bg` |
| `JUNTA_TEXTO` | `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/casos/04_estruturas/e03_texto_buffer/programa.bg`, `tests/casos/05_entrada_saida/i09_texto_truncamento/programa.bg` |
| `LEIA` | `tests/casos/05_entrada_saida/i06_leia_texto_apos_leia/programa.bg`, `tests/casos/05_entrada_saida/i15_leia_caractere_apos_numero/programa.bg` |
| `LEIA_OK` | `tests/casos/05_entrada_saida/i04_leia_tipos/programa.bg`, `tests/casos/05_entrada_saida/i05_leia_falha/programa.bg`, `tests/casos/05_entrada_saida/i06_leia_texto_apos_leia/programa.bg`, `tests/casos/05_entrada_saida/i15_leia_caractere_apos_numero/programa.bg` |
| `LEIA_TEXTO` | `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/05_entrada_saida/i07_leia_texto_limpo/programa.bg`, `tests/casos/05_entrada_saida/i06_leia_texto_apos_leia/programa.bg`, `tests/casos/05_entrada_saida/i08_leia_texto_truncamento/programa.bg` |
| `LOGICO` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/07_multiarquivo/m01_modulos/comum.h` (+4) |
| `MAXIMO` | `tests/casos/08_aplicacao/app_acervo/relatorio.bg` |
| `MINIMO` | `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/run.sh` |
| `MOD` | `tests/casos/02_basico/b02_aritmetica/programa.bg`, `tests/casos/03_controle/c02_se_senao/programa.bg`, `tests/casos/06_negativos/n07_mod_igual/programa.bg`, `tests/casos/07_multiarquivo/m01_modulos/matematica.bg` (+2) |
| `NAO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/05_entrada_saida/i08_leia_texto_truncamento/programa.bg` |
| `NATURAL` | `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/05_entrada_saida/i01_formatos/programa.bg` |
| `NATURAL16` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `NATURAL32` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `NATURAL64` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `NATURAL8` | `tests/casos/02_basico/b04_tipos/programa.bg` |
| `OU` | `tests/casos/07_multiarquivo/m01_modulos/matematica.bg` |
| `PARA` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/casos/03_controle/c12_para_vazio/programa.bg`, `tests/casos/03_controle/c11_para_basico/programa.bg` (+11) |
| `PARA_PASSO` | `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c14_para_passo/programa.bg`, `tests/casos/03_controle/c23_para_passo_negativo/programa.bg` |
| `PARE` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c18_escolha_pare/programa.bg`, `tests/casos/03_controle/c07_enquanto_pare/programa.bg` (+6) |
| `PISO` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `POTENCIA` | `tests/casos/08_aplicacao/app_acervo/acervo.bg` |
| `PROCEDIMENTO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/07_multiarquivo/m01_modulos/comum.h`, `tests/casos/03_controle/c03_senao_se/programa.bg` (+10) |
| `PROSSIGA` | `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/casos/03_controle/c08_enquanto_prossiga/programa.bg`, `tests/run.sh` |
| `RAIZ` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/run.sh` |
| `REAL` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/02_basico/b04_tipos/programa.bg`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg` (+8) |
| `REAL_CURTO` | `tests/casos/02_basico/b04_tipos/programa.bg`, `tests/casos/05_entrada_saida/i01_formatos/programa.bg` |
| `REAL_LONGO` | `tests/casos/05_entrada_saida/i01_formatos/programa.bg` |
| `REGISTRO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/04_estruturas/e04_registro/programa.bg`, `tests/casos/04_estruturas/e06_vetor_registros/programa.bg`, `tests/casos/04_estruturas/e05_registro_aninhado/programa.bg` (+3) |
| `REPITA` | `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/03_controle/c09_repita_uma_vez/programa.bg`, `tests/casos/03_controle/c10_repita_ate_que/programa.bg`, `tests/casos/03_controle/c17_repita_ate_acucar/programa.bg` (+2) |
| `RETORNE` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/04_estruturas/e08_recursao/programa.bg` (+7) |
| `SE` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/02_basico/b05_caractere/programa.bg`, `tests/casos/03_controle/c02_se_senao/programa.bg` (+34) |
| `SENAO` | `tests/casos/02_basico/b03_real/programa.bg`, `tests/casos/03_controle/c02_se_senao/programa.bg`, `tests/casos/03_controle/c03_senao_se/programa.bg`, `tests/casos/03_controle/c04_aninhado/programa.bg` (+12) |
| `SENAO_SE` | `tests/casos/03_controle/c03_senao_se/programa.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/12_negativos_m1/t02_senao_se/esperado_erro.txt`, `tests/run.sh` |
| `SORTEIA` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `TAMANHO_TEXTO` | `tests/casos/02_basico/b05_caractere/programa.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/casos/04_estruturas/e03_texto_buffer/programa.bg` (+2) |
| `TAMANHO_VETOR` | `tests/casos/05_entrada_saida/i01_formatos/programa.bg`, `tests/casos/04_estruturas/e09_funcao_vetor/programa.bg`, `tests/casos/04_estruturas/e02_tamanho_vetor/programa.bg`, `tests/casos/13_avisos/w06_tamanho_de_parametro/programa.bg` |
| `TETO` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `TEXTO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/02_basico/b05_caractere/programa.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg` (+6) |
| `TEXTOS_DIFERENTES` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.bg` |
| `TEXTOS_IGUAIS` | `tests/casos/08_aplicacao/app_acervo/relatorio.bg` |
| `TEXTO_FIXO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/08_aplicacao/app_acervo/relatorio.bg`, `tests/casos/04_estruturas/e03_texto_buffer/programa.bg` (+4) |
| `TEXTO_PARA_INTEIRO` | `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/05_entrada_saida/i10_conversoes/programa.bg` |
| `TEXTO_PARA_REAL` | `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/05_entrada_saida/i10_conversoes/programa.bg` |
| `VAZIO` | `tests/casos/04_estruturas/e11_procedimento_vazio/programa.bg` |
| `VERDADEIRO` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/08_aplicacao/app_acervo/principal.bg`, `tests/casos/05_entrada_saida/i01_formatos/programa.bg`, `tests/casos/07_multiarquivo/m01_modulos/matematica.bg` (+2) |
| `VETOR` | `tests/casos/08_aplicacao/app_acervo/acervo.bg`, `tests/casos/04_estruturas/e01_vetor_basico/programa.bg`, `tests/casos/04_estruturas/e09_funcao_vetor/programa.bg`, `tests/casos/04_estruturas/e02_tamanho_vetor/programa.bg` (+5) |

## 2. Diagnósticos

### 2.1 Avisos do transpilador (implementados e testados)

| código | caso |
|---|---|
| W-01 | `tests/casos/13_avisos/w01_atribuicao_em_condicao` |
| W-02 | `tests/casos/13_avisos/w02_sem_inicializador` |
| W-03 | `tests/casos/13_avisos/w03_condicao_literal` |
| W-04 | `tests/casos/13_avisos/w04_controle_modificado` |
| W-05 | `tests/casos/13_avisos/w05_nome_todo_maiusculo` |
| W-06 | `tests/casos/13_avisos/w06_tamanho_de_parametro` |
| W-07 | `tests/casos/13_avisos/w07_tamanho_constante` |

### 2.2 Erros

* **11 casos** de erro do compilador C (`tests/casos/06_negativos/`), cada um
  afirmando um trecho exato da mensagem (o catálogo humano está em
  [01 §12.3](docs/01-especificacao.md)).
* **10 casos** de erro do transpilador (`tests/casos/12_negativos_m1/`), que
  acusa antes do C e com a linha do fonte `.bg`.

## 3. Suítes e casos em disco

| suíte | casos | o que cobre |
|---|---|---|
| `02_basico` | 7 | tipos, operadores, aritmética, texto, utilidades |
| `03_controle` | 23 | SE/SENAO_SE, ENQUANTO, REPITA/ATE_QUE, PARA, ESCOLHA, desvios |
| `04_estruturas` | 11 | VETOR, TEXTO_FIXO, REGISTRO, ENUMERACAO, funções, recursão |
| `05_entrada_saida` | 15 | ESCREVA*, LEIA*, formatação, ERRO/AFIRME, truncamento |
| `06_negativos` | 11 | o compilador C recusa o que a especificação proíbe (11 casos) |
| `07_multiarquivo` | 2 | módulos, EXTERNO/ESTATICO, C puro junto de BG |
| `08_aplicacao` | 1 | sistema de acervo: 3 módulos + cabeçalho, estado privado, E/S |
| `12_negativos_m1` | 10 | o transpilador acusa o erro certo, com linha (8 casos) |
| `13_avisos` | 7 | um caso por aviso W-01…W-07 |

## 4. Critérios de aceitação da Fase 2

| AC | critério | verificação automatizada |
|---|---|---|
| AC-21 | cabeçalho header-only, idempotente, compila sozinho | 01_sanidade |
| AC-22 | expansões idênticas às de 03 §3 | 01_sanidade + tests/expansoes.txt |
| AC-23 | 12 exemplos compilam e produzem a saída esperada | 10_exemplos |
| AC-24 | `BG_SEM_APELIDOS` desliga os nomes curtos | 01_sanidade |
| AC-25 | nenhuma macro avalia o argumento duas vezes | 01_sanidade |
| AC-26 | C11, C17 e clang | 01_sanidade |
| AC-27 | `LEIA_TEXTO` depois de `LEIA` (C-20) | 05_entrada_saida/i06 |
| AC-28 | nenhum aviso nos exemplos com `-Werror` | 10_exemplos |
| AC-29 | exemplo compila em menos de 1 s | 01_sanidade |
| AC-30 | M1 traduz todos os construtores X | 09_m1 |
| AC-31 | o C emitido não contém macros de palavra-chave (RG-10) | 09_m1 |
| AC-32 | equivalência M0 × M1 em **todos** os casos | lib.sh (todo caso roda 2×) |
| AC-33 | W-01…W-07 emitidos | 06_avisos (7 casos) |
| AC-34 | erro com linha nos fontes malformados | 06_negativos + 12_negativos_m1 |
| AC-35 | código de saída 0/1/2/70 | 01_sanidade + 09_m1 |
| AC-36 | `--checar` valida sem compilar | 09_m1 |

## 5. Restrições técnicas e o teste que as fixa

| restrição | o que a suíte faz |
|---|---|
| C-01 | sem reordenar tokens — 09_m1 (traduções) + 12_negativos_m1/t05,t06 |
| C-02 | `;` obrigatório em comando simples — 12_negativos_m1/t07 + 06_negativos |
| C-03 | um valor por `ESCREVA` — 06_negativos/n04 |
| C-04 | vírgula de nível superior em macro — 03_controle/c22 (o caso que a Fase 2 corrigiu) |
| C-05 | `SENAO SE` não encadeia — 06_negativos/n03 + 12_negativos_m1/t02 |
| C-06 | `ENTAO`/`FACA`, sem chaves — 06_negativos/n02 + 12_negativos_m1/t01 |
| C-07 | tabela de erros + M1 como resposta de qualidade — 06_negativos, 12_negativos_m1 |
| C-08 | R-1/R-2 (nomes reservados) — 06_avisos/w05 |
| C-09 | `BG_SEM_APELIDOS` — 01_sanidade |
| C-10 | `ABS`/`MINIMO`/`MAXIMO` inline — 01_sanidade (avaliação única) + 02_basico |
| C-11 | C11 obrigatório — 01_sanidade (C99 é rejeitado) |
| C-12 | sem `MOD=` — 06_negativos/n07 |
| C-13 | documento de compilação — README.md + este arquivo |
| C-14 | `FIM_*` explícito — toda a árvore de casos |
| C-17 | `TEXTO`/`TEXTO_FIXO` — 02_basico + 06_negativos/n05 |
| C-18 | vetor + tamanho, nunca `sizeof` de ponteiro — 06_avisos/w06 |
| C-20 | contrato de `LEIA_TEXTO` — 05_entrada_saida/i06, i15 |
| C-23 | tamanho de vetor não pode ser `CONSTANTE` — 06_avisos/w07 |

## 6. Estresse

| situação | onde |
|---|---|
| 40 níveis de `SE` aninhados | `11_estresse` |
| 20 níveis de `ENQUANTO` aninhados | `11_estresse` |
| 300 funções + 5000 iterações | `11_estresse` |
| literal de 4000 caracteres | `11_estresse` |
| caso com 3 módulos e 500 linhas | `08_aplicacao/app_acervo` |

## 7. O que ainda não é automatizado

* **AC-17 / AC-18** (leitura por pessoa de fora): não há como um programa julgar
  legibilidade. O que a Fase 2 fez foi a parte verificável — percorrer a tabela de
  mapeamento e executar cada construtor — e essa revisão encontrou **três** erros
  reais de especificação (C-04, `PARA_PASSO` com passo negativo e o `;` de
  `ATE_QUE`), todos corrigidos e fixados por teste.
* **D-09** (`BG_ESTRITO`), **D-12** (funções de vetor) e **D-17** (locale do
  sistema): fora da v1.0 por decisão registrada em
  [04](docs/04-restricoes-e-decisoes.md).
* Desempenho além do limite de 1 s por compilação (AC-29): medido, não otimizado.
