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
esperada. Hoje são **132 casos em disco**; o total de verificações
(impresso no fim de `make test`) inclui também as checagens de sanidade, de linha de
comando, os 7 avisos, os 34 casos negativos e as 4 provas de estresse.

Legenda das suítes nas seções seguintes: os diretórios `tests/casos/*` são executados
pelo `run.sh`; as suítes `01_sanidade`, `09_m1`, `10_exemplos` e `11_estresse` são
blocos de shell dentro do próprio `run.sh` (não aparecem como diretório).

## 1. Superfície da linguagem (nomes curtos do cabeçalho)

Todos os **117** nomes curtos aparecem em pelo menos um arquivo de teste.

| nome | onde é exercitado |
|---|---|
| `ABS` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/casos/07_multiarquivo/m01_modulos/matematica.c` (+1) |
| `ACRESCENTA_TEXTO` | `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c` |
| `AFIRME` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/05_entrada_saida/i13_afirme_ok/programa.c`, `tests/casos/05_entrada_saida/i14_afirme_dispara/programa.c` |
| `ARREDONDA` | `tests/casos/08_aplicacao/app_acervo/acervo.c` |
| `ATE_QUE` | `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c09_repita_uma_vez/programa.c`, `tests/casos/03_controle/c10_repita_ate_que/programa.c`, `tests/casos/03_controle/c24_zero_iteracoes/programa.c` (+4) |
| `BITS_DE` | `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `CARACTERE` | `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/02_basico/b05_caractere/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t03_caractere/programa.c` (+8) |
| `CARACTERE_MAIUSCULO` | `tests/casos/09_tipos_expr/t03_caractere/programa.c` |
| `CARACTERE_MINUSCULO` | `tests/casos/09_tipos_expr/t03_caractere/programa.c` |
| `CASO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c18_escolha_pare/programa.c`, `tests/casos/04_estruturas/e07_enumeracao/programa.c` (+6) |
| `COMPARA_TEXTO` | `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c` |
| `CONSTANTE` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/13_avisos/w07_tamanho_constante/programa.c`, `tests/run.sh` |
| `CONTRARIO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c18_escolha_pare/programa.c`, `tests/casos/04_estruturas/e07_enumeracao/programa.c` (+5) |
| `COPIA_TEXTO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/casos/04_estruturas/e03_texto_buffer/programa.c`, `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c` (+1) |
| `DIFERENTE` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `DIVIDE` | `tests/casos/09_tipos_expr/t07_divisao/programa.c`, `tests/casos/09_tipos_expr/t10_estouro_divisao/programa.c`, `tests/casos/09_tipos_expr/t08_divisao_por_zero/programa.c`, `tests/run.sh` |
| `E` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c` (+7) |
| `ENQUANTO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c06_enquanto_zero/programa.c`, `tests/casos/03_controle/c07_enquanto_pare/programa.c` (+9) |
| `ENTAO` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/02_basico/b05_caractere/programa.c`, `tests/casos/03_controle/c02_se_senao/programa.c` (+44) |
| `ENUMERACAO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/04_estruturas/e07_enumeracao/programa.c`, `tests/casos/12_negativos_m1/t10_enumeracao_com_chaves/programa.c`, `tests/casos/12_negativos_m1/t10_enumeracao_com_chaves/esperado_erro.txt` |
| `EPSILON_REAL` | `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `ERRO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/05_entrada_saida/i12_erro_fatal/programa.c`, `tests/casos/03_controle/c36_laco_entrada_sentinela/programa.c` |
| `ESCOLHA` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c18_escolha_pare/programa.c` (+9) |
| `ESCREVA` | `tests/casos/03_controle/c03_senao_se/programa.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c14_para_passo/programa.c`, `tests/casos/04_estruturas/e04_registro/programa.c` (+21) |
| `ESCREVAL` | `tests/casos/02_basico/b01_ola/programa.c`, `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/02_basico/b05_caractere/programa.c` (+115) |
| `ESCREVA_ERRO` | `tests/casos/08_aplicacao/app_acervo/principal.c` |
| `ESCREVA_FMT` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/05_entrada_saida/i03_fmt/programa.c`, `tests/casos/08_aplicacao/app_acervo/principal.c` (+15) |
| `ESCREVA_LINHA` | `tests/casos/03_controle/c03_senao_se/programa.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c14_para_passo/programa.c`, `tests/casos/03_controle/c11_para_basico/programa.c` (+14) |
| `ESCREVA_TEXTO` | `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c`, `tests/casos/04_estruturas/e06_vetor_registros/programa.c`, `tests/casos/04_estruturas/e05_registro_aninhado/programa.c`, `tests/casos/05_entrada_saida/i07_leia_texto_limpo/programa.c` (+3) |
| `ESTATICO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/07_multiarquivo/m02_estatico_misto/contas.c`, `tests/lib.sh` |
| `EXTERNO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/07_multiarquivo/m01_modulos/comum.h`, `tests/casos/07_multiarquivo/m02_estatico_misto/contas.h`, `tests/lib.sh` |
| `E_DIGITO` | `tests/casos/09_tipos_expr/t03_caractere/programa.c` |
| `E_LETRA` | `tests/casos/09_tipos_expr/t03_caractere/programa.c` |
| `FACA` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c15_para_acucar/programa.c`, `tests/casos/03_controle/c06_enquanto_zero/programa.c` (+14) |
| `FALSO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/09_tipos_expr/t04_logico/programa.c`, `tests/casos/09_tipos_expr/t13_extensoes/programa.c` (+5) |
| `FIM` | `tests/casos/02_basico/b01_ola/programa.c`, `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/02_basico/b05_caractere/programa.c` (+129) |
| `FIM_ENQUANTO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c06_enquanto_zero/programa.c`, `tests/casos/03_controle/c07_enquanto_pare/programa.c` (+9) |
| `FIM_ENUMERACAO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/04_estruturas/e07_enumeracao/programa.c`, `tests/casos/12_negativos_m1/t10_enumeracao_com_chaves/programa.c` |
| `FIM_ESCOLHA` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c18_escolha_pare/programa.c`, `tests/casos/04_estruturas/e07_enumeracao/programa.c` (+6) |
| `FIM_REGISTRO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/04_estruturas/e04_registro/programa.c`, `tests/casos/04_estruturas/e06_vetor_registros/programa.c`, `tests/casos/04_estruturas/e05_registro_aninhado/programa.c` (+2) |
| `FIM_SE` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/02_basico/b05_caractere/programa.c`, `tests/casos/03_controle/c02_se_senao/programa.c` (+44) |
| `FORMATA_TEXTO` | `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c` |
| `FUNCAO` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/07_multiarquivo/m01_modulos/comum.h` (+10) |
| `IGUAL` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/casos/07_multiarquivo/m01_modulos/matematica.c` (+2) |
| `INICIA_SORTEIO` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `INICIO` | `tests/casos/02_basico/b01_ola/programa.c`, `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/02_basico/b05_caractere/programa.c` (+129) |
| `INTEIRO` | `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/02_basico/b05_caractere/programa.c` (+98) |
| `INTEIRO16` | `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `INTEIRO32` | `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `INTEIRO64` | `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/04_estruturas/e08_recursao/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/05_entrada_saida/i01_formatos/programa.c` (+2) |
| `INTEIRO8` | `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `INTEIRO_CURTO` | `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c`, `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `INTEIRO_LONGO` | `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/05_entrada_saida/i01_formatos/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `JUNTA_TEXTO` | `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/casos/04_estruturas/e03_texto_buffer/programa.c`, `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c`, `tests/casos/05_entrada_saida/i09_texto_truncamento/programa.c` |
| `LEIA` | `tests/casos/06_negativos/n16_leia_logico/programa.c`, `tests/casos/09_tipos_expr/t15_leia_tipos/programa.c`, `tests/casos/05_entrada_saida/i06_leia_texto_apos_leia/programa.c`, `tests/casos/05_entrada_saida/i15_leia_caractere_apos_numero/programa.c` |
| `LEIA_INTEIRO` | `tests/casos/09_tipos_expr/t12_leia_numeros/programa.c`, `tests/casos/03_controle/c36_laco_entrada_sentinela/programa.c`, `tests/run.sh` |
| `LEIA_OK` | `tests/casos/05_entrada_saida/i04_leia_tipos/programa.c`, `tests/casos/05_entrada_saida/i05_leia_falha/programa.c`, `tests/casos/05_entrada_saida/i06_leia_texto_apos_leia/programa.c`, `tests/casos/05_entrada_saida/i15_leia_caractere_apos_numero/programa.c` |
| `LEIA_REAL` | `tests/casos/09_tipos_expr/t12_leia_numeros/programa.c` |
| `LEIA_TEXTO` | `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/05_entrada_saida/i07_leia_texto_limpo/programa.c`, `tests/casos/05_entrada_saida/i06_leia_texto_apos_leia/programa.c`, `tests/casos/05_entrada_saida/i08_leia_texto_truncamento/programa.c` |
| `LOGICO` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/07_multiarquivo/m01_modulos/comum.h` (+11) |
| `LOGICO_DE` | `tests/casos/09_tipos_expr/t04_logico/programa.c` |
| `MAXIMO` | `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c` |
| `MAXIMO_INTEIRO` | `tests/casos/09_tipos_expr/t13_extensoes/programa.c`, `tests/casos/09_tipos_expr/t06_conversoes/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `MAXIMO_NATURAL` | `tests/casos/09_tipos_expr/t07_divisao/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `MAXIMO_REAL` | `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `MINIMO` | `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/run.sh` |
| `MINIMO_INTEIRO` | `tests/casos/09_tipos_expr/t07_divisao/programa.c`, `tests/casos/09_tipos_expr/t13_extensoes/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c`, `tests/casos/09_tipos_expr/t10_estouro_divisao/programa.c` |
| `MOD` | `tests/casos/02_basico/b02_aritmetica/programa.c`, `tests/casos/03_controle/c02_se_senao/programa.c`, `tests/casos/06_negativos/n07_mod_igual/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c` (+7) |
| `MULTIPLICA_SEGURA` | `tests/casos/09_tipos_expr/t13_extensoes/programa.c` |
| `NAO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/09_tipos_expr/t04_logico/programa.c`, `tests/casos/09_tipos_expr/t05_precedencia/programa.c` (+2) |
| `NATURAL` | `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/09_tipos_expr/t07_divisao/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t06_conversoes/programa.c` (+3) |
| `NATURAL16` | `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t15_leia_tipos/programa.c`, `tests/casos/09_tipos_expr/t15_leia_tipos/esperado.txt`, `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `NATURAL32` | `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `NATURAL64` | `tests/casos/09_tipos_expr/t14_propriedades/programa.c`, `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `NATURAL8` | `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t06_conversoes/programa.c`, `tests/casos/09_tipos_expr/t15_leia_tipos/programa.c` (+2) |
| `NATURAL_CURTO` | `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t15_leia_tipos/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c`, `tests/casos/09_tipos_expr/t15_leia_tipos/esperado.txt` (+1) |
| `NATURAL_LONGO` | `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `OU` | `tests/casos/09_tipos_expr/t04_logico/programa.c`, `tests/casos/07_multiarquivo/m01_modulos/matematica.c`, `tests/casos/09_tipos_expr/t05_precedencia/programa.c`, `tests/casos/03_controle/c34_faixas_senao_se/programa.c` |
| `OU_EXCLUSIVO` | `tests/casos/09_tipos_expr/t04_logico/programa.c`, `tests/run.sh` |
| `PARA` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/casos/03_controle/c12_para_vazio/programa.c`, `tests/casos/03_controle/c11_para_basico/programa.c` (+21) |
| `PARA_PASSO` | `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c14_para_passo/programa.c`, `tests/casos/03_controle/c24_zero_iteracoes/programa.c`, `tests/casos/03_controle/c26_passo_zero_erro/programa.c` (+6) |
| `PARE` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c18_escolha_pare/programa.c`, `tests/casos/03_controle/c07_enquanto_pare/programa.c` (+11) |
| `PISO` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `POTENCIA` | `tests/casos/08_aplicacao/app_acervo/acervo.c` |
| `PROCEDIMENTO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/03_controle/c03_senao_se/programa.c`, `tests/casos/07_multiarquivo/m01_modulos/comum.h` (+10) |
| `PROSSIGA` | `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/casos/03_controle/c30_repita_prossiga/programa.c`, `tests/casos/03_controle/c35_escolha_no_laco/programa.c` (+6) |
| `RAIZ` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/run.sh` |
| `REAL` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h` (+12) |
| `REAL_CURTO` | `tests/casos/02_basico/b04_tipos/programa.c`, `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/09_tipos_expr/t15_leia_tipos/programa.c`, `tests/casos/05_entrada_saida/i01_formatos/programa.c` (+2) |
| `REAL_LONGO` | `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/09_tipos_expr/t15_leia_tipos/programa.c`, `tests/casos/05_entrada_saida/i01_formatos/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` (+1) |
| `REAL_PARA_INTEIRO` | `tests/casos/09_tipos_expr/t06_conversoes/programa.c` |
| `REGISTRO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/04_estruturas/e04_registro/programa.c`, `tests/casos/04_estruturas/e06_vetor_registros/programa.c`, `tests/casos/04_estruturas/e05_registro_aninhado/programa.c` (+3) |
| `REPITA` | `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c09_repita_uma_vez/programa.c`, `tests/casos/03_controle/c10_repita_ate_que/programa.c`, `tests/casos/03_controle/c24_zero_iteracoes/programa.c` (+6) |
| `RESTO` | `tests/casos/09_tipos_expr/t07_divisao/programa.c`, `tests/casos/06_negativos/n15_resto_com_real/programa.c`, `tests/casos/09_tipos_expr/t09_resto_por_zero/programa.c` |
| `RETORNE` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/04_estruturas/e08_recursao/programa.c` (+7) |
| `SE` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/02_basico/b05_caractere/programa.c`, `tests/casos/03_controle/c02_se_senao/programa.c` (+50) |
| `SENAO` | `tests/casos/02_basico/b03_real/programa.c`, `tests/casos/03_controle/c02_se_senao/programa.c`, `tests/casos/03_controle/c03_senao_se/programa.c`, `tests/casos/03_controle/c04_aninhado/programa.c` (+18) |
| `SENAO_SE` | `tests/casos/03_controle/c03_senao_se/programa.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/03_controle/c34_faixas_senao_se/programa.c`, `tests/casos/12_negativos_m1/t02_senao_se/esperado_erro.txt` (+1) |
| `SOMA_SEGURA` | `tests/casos/09_tipos_expr/t13_extensoes/programa.c`, `tests/run.sh` |
| `SORTEIA` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `SUBTRAI_SEGURA` | `tests/casos/09_tipos_expr/t13_extensoes/programa.c` |
| `TAMANHO_DE` | `tests/casos/09_tipos_expr/t02_reais/programa.c`, `tests/casos/09_tipos_expr/t01_inteiros/programa.c`, `tests/casos/09_tipos_expr/t14_propriedades/programa.c` |
| `TAMANHO_TEXTO` | `tests/casos/02_basico/b05_caractere/programa.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/casos/04_estruturas/e03_texto_buffer/programa.c` (+3) |
| `TAMANHO_VETOR` | `tests/casos/05_entrada_saida/i01_formatos/programa.c`, `tests/casos/04_estruturas/e09_funcao_vetor/programa.c`, `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c`, `tests/casos/04_estruturas/e02_tamanho_vetor/programa.c` (+1) |
| `TETO` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `TEXTO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/02_basico/b05_caractere/programa.c`, `tests/casos/08_aplicacao/app_acervo/principal.c` (+9) |
| `TEXTOS_DIFERENTES` | `tests/casos/02_basico/b07_tipos_e_utilitarios/programa.c` |
| `TEXTOS_IGUAIS` | `tests/casos/08_aplicacao/app_acervo/relatorio.c` |
| `TEXTO_FIXO` | `tests/casos/08_aplicacao/app_acervo/acervo.h`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/08_aplicacao/app_acervo/relatorio.c`, `tests/casos/04_estruturas/e03_texto_buffer/programa.c` (+5) |
| `TEXTO_PARA_INTEIRO` | `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/09_tipos_expr/t06_conversoes/programa.c`, `tests/casos/05_entrada_saida/i10_conversoes/programa.c`, `tests/casos/06_negativos/n13_texto_para_inteiro/programa.c` |
| `TEXTO_PARA_NATURAL` | `tests/casos/09_tipos_expr/t06_conversoes/programa.c` |
| `TEXTO_PARA_REAL` | `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/09_tipos_expr/t06_conversoes/programa.c`, `tests/casos/05_entrada_saida/i10_conversoes/programa.c` |
| `TEXTO_VAZIO` | `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c` |
| `TRECHO_TEXTO` | `tests/casos/09_tipos_expr/t11_texto_seguro/programa.c` |
| `VAZIO` | `tests/casos/04_estruturas/e11_procedimento_vazio/programa.c` |
| `VERDADEIRO` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/08_aplicacao/app_acervo/principal.c`, `tests/casos/09_tipos_expr/t04_logico/programa.c`, `tests/casos/06_negativos/n16_leia_logico/programa.c` (+4) |
| `VETOR` | `tests/casos/08_aplicacao/app_acervo/acervo.c`, `tests/casos/04_estruturas/e01_vetor_basico/programa.c`, `tests/casos/04_estruturas/e09_funcao_vetor/programa.c`, `tests/casos/04_estruturas/e02_tamanho_vetor/programa.c` (+5) |

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

* **20 casos** de erro do compilador C (`tests/casos/06_negativos/`), cada um
  afirmando um trecho exato da mensagem (o catálogo humano está em
  [01 §12.3](../docs/01-especificacao.md)).
* **14 casos** de erro do transpilador (`tests/casos/12_negativos_m1/`), que
  acusa antes do C e com a linha do fonte.

## 3. Suítes e casos em disco

| suíte | casos | o que cobre |
|---|---|---|
| `02_basico` | 7 | tipos, operadores, aritmética, texto, utilidades |
| `03_controle` | 40 | SE/SENAO_SE, ENQUANTO, REPITA/ATE_QUE, PARA, ESCOLHA, desvios |
| `04_estruturas` | 11 | VETOR, TEXTO_FIXO, REGISTRO, ENUMERACAO, funções, recursão |
| `05_entrada_saida` | 15 | ESCREVA*, LEIA*, formatação, ERRO/AFIRME, truncamento |
| `06_negativos` | 20 | o compilador C recusa o que a especificação proíbe |
| `07_multiarquivo` | 2 | módulos, EXTERNO/ESTATICO, C puro junto de BG |
| `08_aplicacao` | 1 | sistema de acervo: 3 módulos + cabeçalho, estado privado, E/S |
| `09_tipos_expr` | 15 | Fase 4: tipos, conversões, precedência, divisão verificada, texto seguro, entrada numérica, extensões opcionais |
| `12_negativos_m1` | 14 | o transpilador acusa o erro certo, com linha |
| `13_avisos` | 7 | um caso por aviso W-01…W-07 |

## 4. Critérios de aceitação (Fases 2 e 4)

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
| AC-37 | referência de tipos e expressões (representação, faixa, precisão) | [06](../docs/06-tipos-e-expressoes.md) |
| AC-38 | todo tipo tem mapeamento C e teste de identidade | 09_tipos_expr/t14_propriedades |
| AC-39 | aliases em português e operadores lógicos definidos | 09_tipos_expr/t04_logico + t01 |
| AC-40 | atribuição, comparação, aritmética, precedência, conversão e verdade | t05_precedencia, t06_conversoes, t04_logico |
| AC-41 | nenhuma abstração esconde diferença de C | t01_inteiros, t07_divisao (conversões e sinais reais) |
| AC-42 | texto seguro com limite de buffer | t11_texto_seguro |
| AC-43 | divisão por zero, conversão inválida, buffer e entrada numérica | t08, t09, t10 (erro + rc), t06, t11, t12 |
| AC-44 | estouro inteiro e ponto flutuante documentados com precisão | [06 §10](../docs/06-tipos-e-expressoes.md) + t01, t02, t13 |
| AC-45 | extensões de compilador opcionais e isoladas | t13_extensoes + 01_sanidade (desligadas por padrão) |
| AC-46 | exemplos executáveis para todos os tipos e para as expressões | 10_exemplos (tipos.c, expressoes.c) |
| AC-47 | diferenças semânticas em relação a BASIC e Portugol | [06 §13](../docs/06-tipos-e-expressoes.md) |

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
| C-24 | texto é ponteiro sem dono — 09_tipos_expr/t11 + 06_negativos/n12 |
| C-25 | sem resto real nem potência por símbolo — 06_negativos/n14, n15 |
| C-26 | formato de `FORMATA_TEXTO` não é conferido pelo compilador — t11 |

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
  [04](../docs/04-restricoes-e-decisoes.md).
* Desempenho além do limite de 1 s por compilação (AC-29): medido, não otimizado.
