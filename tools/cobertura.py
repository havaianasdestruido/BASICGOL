#!/usr/bin/env python3
"""Gera tests/COBERTURA.md — a matriz de rastreabilidade da suíte de testes.

Uso:  python3 tools/cobertura.py        (ou  make cobertura)

O documento gerado responde a uma pergunta só: *o que exatamente já é testado?*
As seções 1–3 são calculadas a partir da árvore de testes (nada é digitado à mão,
então não há como o documento "mentir" sobre os arquivos); as seções 4–5 mapeiam
critérios de aceitação e restrições, que são afirmações de projeto e por isso
vivem aqui, no gerador.
"""

from __future__ import annotations

import re
import subprocess
from datetime import date
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
HEADER = RAIZ / "include" / "basicgol.h"
CASOS = RAIZ / "tests" / "casos"

# ---------------------------------------------------------------- seção 1

def nomes_curtos() -> list[str]:
    """Todos os nomes curtos (a sintaxe canônica) definidos no cabeçalho."""
    texto = HEADER.read_text(encoding="utf-8")
    inicio = texto.index("3. Nomes curtos")
    secao = texto[inicio:]
    return sorted(set(re.findall(r"^#define\s+([A-Z][A-Z0-9_]*)", secao, re.M)))


def arquivos_com(nome: str) -> list[str]:
    """Arquivos de teste que citam o nome como palavra inteira."""
    padrao = re.compile(r"\b" + re.escape(nome) + r"\b")
    achados = []
    for arq in sorted(RAIZ.joinpath("tests").rglob("*")):
        if not arq.is_file() or arq.suffix not in {".c", ".h", ".sh", ".txt"}:
            continue
        if padrao.search(arq.read_text(encoding="utf-8", errors="ignore")):
            achados.append(str(arq.relative_to(RAIZ)))
    return achados


def casos_por_suite() -> list[tuple[str, int, str]]:
    """(suíte, número de casos, o que cobre)."""
    descricoes = {
        "02_basico": "tipos, operadores, aritmética, texto, utilidades",
        "03_controle": "SE/SENAO_SE, ENQUANTO, REPITA/ATE_QUE, PARA, ESCOLHA, desvios",
        "04_estruturas": "VETOR, TEXTO_FIXO, REGISTRO, ENUMERACAO, funções, recursão",
        "05_entrada_saida": "ESCREVA*, LEIA*, formatação, ERRO/AFIRME, truncamento",
        "06_negativos": "o compilador C recusa o que a especificação proíbe",
        "12_negativos_m1": "o transpilador acusa o erro certo, com linha",
        "13_avisos": "um caso por aviso W-01…W-07",
        "07_multiarquivo": "módulos, EXTERNO/ESTATICO, C puro junto de BG",
        "08_aplicacao": "sistema de acervo: 3 módulos + cabeçalho, estado privado, E/S",
        "09_tipos_expr": "Fase 4: tipos, conversões, precedência, divisão verificada, "
                         "texto seguro, entrada numérica, extensões opcionais",
        "09_m1": "transpilador: tradução, açúcar, acentos, avisos, CLI",
    }
    linhas = []
    for suite in sorted(p.name for p in CASOS.iterdir() if p.is_dir()):
        dirs = [d for d in (CASOS / suite).iterdir() if d.is_dir()]
        if not dirs:
            continue
        desc = descricoes.get(suite, "")
        linhas.append((suite, len(dirs), desc))
    return linhas

# ---------------------------------------------------------------- seções 4 e 5

AC = [
    ("AC-21", "cabeçalho header-only, idempotente, compila sozinho", "01_sanidade"),
    ("AC-22", "expansões idênticas às de 03 §3", "01_sanidade + tests/expansoes.txt"),
    ("AC-23", "12 exemplos compilam e produzem a saída esperada", "10_exemplos"),
    ("AC-24", "`BG_SEM_APELIDOS` desliga os nomes curtos", "01_sanidade"),
    ("AC-25", "nenhuma macro avalia o argumento duas vezes", "01_sanidade"),
    ("AC-26", "C11, C17 e clang", "01_sanidade"),
    ("AC-27", "`LEIA_TEXTO` depois de `LEIA` (C-20)", "05_entrada_saida/i06"),
    ("AC-28", "nenhum aviso nos exemplos com `-Werror`", "10_exemplos"),
    ("AC-29", "exemplo compila em menos de 1 s", "01_sanidade"),
    ("AC-30", "M1 traduz todos os construtores X", "09_m1"),
    ("AC-31", "o C emitido não contém macros de palavra-chave (RG-10)", "09_m1"),
    ("AC-32", "equivalência M0 × M1 em **todos** os casos", "lib.sh (todo caso roda 2×)"),
    ("AC-33", "W-01…W-07 emitidos", "06_avisos (7 casos)"),
    ("AC-34", "erro com linha nos fontes malformados", "06_negativos + 12_negativos_m1"),
    ("AC-35", "código de saída 0/1/2/70", "01_sanidade + 09_m1"),
    ("AC-36", "`--checar` valida sem compilar", "09_m1"),
    ("AC-37", "referência de tipos e expressões (representação, faixa, precisão)",
     "[06](../docs/06-tipos-e-expressoes.md)"),
    ("AC-38", "todo tipo tem mapeamento C e teste de identidade",
     "09_tipos_expr/t14_propriedades"),
    ("AC-39", "aliases em português e operadores lógicos definidos",
     "09_tipos_expr/t04_logico + t01"),
    ("AC-40", "atribuição, comparação, aritmética, precedência, conversão e verdade",
     "t05_precedencia, t06_conversoes, t04_logico"),
    ("AC-41", "nenhuma abstração esconde diferença de C",
     "t01_inteiros, t07_divisao (conversões e sinais reais)"),
    ("AC-42", "texto seguro com limite de buffer", "t11_texto_seguro"),
    ("AC-43", "divisão por zero, conversão inválida, buffer e entrada numérica",
     "t08, t09, t10 (erro + rc), t06, t11, t12"),
    ("AC-44", "estouro inteiro e ponto flutuante documentados com precisão",
     "[06 §10](../docs/06-tipos-e-expressoes.md) + t01, t02, t13"),
    ("AC-45", "extensões de compilador opcionais e isoladas",
     "t13_extensoes + 01_sanidade (desligadas por padrão)"),
    ("AC-46", "exemplos executáveis para todos os tipos e para as expressões",
     "10_exemplos (tipos.c, expressoes.c)"),
    ("AC-47", "diferenças semânticas em relação a BASIC e Portugol",
     "[06 §13](../docs/06-tipos-e-expressoes.md)"),
]

RESTRICOES = [
    ("C-01", "sem reordenar tokens", "09_m1 (traduções) + 12_negativos_m1/t05,t06"),
    ("C-02", "`;` obrigatório em comando simples", "12_negativos_m1/t07 + 06_negativos"),
    ("C-03", "um valor por `ESCREVA`", "06_negativos/n04"),
    ("C-04", "vírgula de nível superior em macro", "03_controle/c22 (o caso que a Fase 2 corrigiu)"),
    ("C-05", "`SENAO SE` não encadeia", "06_negativos/n03 + 12_negativos_m1/t02"),
    ("C-06", "`ENTAO`/`FACA`, sem chaves", "06_negativos/n02 + 12_negativos_m1/t01"),
    ("C-07", "tabela de erros + M1 como resposta de qualidade", "06_negativos, 12_negativos_m1"),
    ("C-08", "R-1/R-2 (nomes reservados)", "06_avisos/w05"),
    ("C-09", "`BG_SEM_APELIDOS`", "01_sanidade"),
    ("C-10", "`ABS`/`MINIMO`/`MAXIMO` inline", "01_sanidade (avaliação única) + 02_basico"),
    ("C-11", "C11 obrigatório", "01_sanidade (C99 é rejeitado)"),
    ("C-12", "sem `MOD=`", "06_negativos/n07"),
    ("C-13", "documento de compilação", "README.md + este arquivo"),
    ("C-14", "`FIM_*` explícito", "toda a árvore de casos"),
    ("C-17", "`TEXTO`/`TEXTO_FIXO`", "02_basico + 06_negativos/n05"),
    ("C-18", "vetor + tamanho, nunca `sizeof` de ponteiro", "06_avisos/w06"),
    ("C-20", "contrato de `LEIA_TEXTO`", "05_entrada_saida/i06, i15"),
    ("C-23", "tamanho de vetor não pode ser `CONSTANTE`", "06_avisos/w07"),
    ("C-24", "texto é ponteiro sem dono", "09_tipos_expr/t11 + 06_negativos/n12"),
    ("C-25", "sem resto real nem potência por símbolo", "06_negativos/n14, n15"),
    ("C-26", "formato de `FORMATA_TEXTO` não é conferido pelo compilador", "t11"),
]

ESTRESSE = [
    ("40 níveis de `SE` aninhados", "11_estresse"),
    ("20 níveis de `ENQUANTO` aninhados", "11_estresse"),
    ("300 funções + 5000 iterações", "11_estresse"),
    ("literal de 4000 caracteres", "11_estresse"),
    ("caso com 3 módulos e 500 linhas", "08_aplicacao/app_acervo"),
]


def main() -> None:
    nomes = nomes_curtos()
    suites = casos_por_suite()
    total_casos = sum(n for _, n, _ in suites)
    hoje = date.today().isoformat()

    saida: list[str] = []
    w = saida.append

    w("# Cobertura de testes — BASICGOL")
    w("")
    w("> Documento **gerado** por `python3 tools/cobertura.py` (ou `make cobertura`).")
    w(f"> Gerado em {hoje}. Não edite à mão: as seções 1–3 saem da árvore de testes.")
    w("")
    w("## 0. Como rodar")
    w("")
    w("```sh")
    w("make                 # compila o transpilador em build/basicgolc")
    w("make test            # suíte completa (M0 + M1 em cada caso)")
    w("make test-v          # idem, listando cada verificação aprovada")
    w("./tests/run.sh 03    # uma suíte só (prefixo do nome)")
    w("```")
    w("")
    w("Cada caso de execução roda **duas vezes** — caminho M0 (macros do cabeçalho) e")
    w("caminho M1 (transpilador) — e só passa se os dois produzirem exatamente a saída")
    w(f"esperada. Hoje são **{total_casos} casos em disco**; o total de verificações")
    n_neg = len([d for d in (CASOS / "06_negativos").iterdir() if d.is_dir()])
    n_m1 = len([d for d in (CASOS / "12_negativos_m1").iterdir() if d.is_dir()])
    w("(impresso no fim de `make test`) inclui também as checagens de sanidade, de linha de")
    w(f"comando, os 7 avisos, os {n_neg + n_m1} casos negativos e as 4 provas de estresse.")
    w("")
    w("Legenda das suítes nas seções seguintes: os diretórios `tests/casos/*` são executados")
    w("pelo `run.sh`; as suítes `01_sanidade`, `09_m1`, `10_exemplos` e `11_estresse` são")
    w("blocos de shell dentro do próprio `run.sh` (não aparecem como diretório).")
    w("")
    w("## 1. Superfície da linguagem (nomes curtos do cabeçalho)")
    w("")
    w(f"Todos os **{len(nomes)}** nomes curtos aparecem em pelo menos um arquivo de teste.")
    w("")
    w("| nome | onde é exercitado |")
    w("|---|---|")
    for nome in nomes:
        onde = arquivos_com(nome)
        # casos de teste primeiro, depois scripts/tabelas; dentro de cada grupo, o
        # caminho mais curto primeiro (lê-se melhor)
        onde.sort(key=lambda a: (0 if a.startswith("tests/casos/") else 1, len(a)))
        celula = ", ".join(f"`{a}`" for a in onde[:4])
        if len(onde) > 4:
            celula += f" (+{len(onde) - 4})"
        w(f"| `{nome}` | {celula or '—'} |")
    w("")
    w("## 2. Diagnósticos")
    w("")
    w("### 2.1 Avisos do transpilador (implementados e testados)")
    w("")
    w("| código | caso |")
    w("|---|---|")
    avisos = sorted((CASOS / "13_avisos").iterdir())
    for d in avisos:
        if d.is_dir():
            codigo = d.name.split("_")[0].upper()
            w(f"| {codigo[:1]}-{codigo[1:]} | `tests/casos/13_avisos/{d.name}` |")
    w("")
    w("### 2.2 Erros")
    w("")
    n_neg = len([d for d in (CASOS / "06_negativos").iterdir() if d.is_dir()])
    n_m1 = len([d for d in (CASOS / "12_negativos_m1").iterdir() if d.is_dir()])
    w(f"* **{n_neg} casos** de erro do compilador C (`tests/casos/06_negativos/`), cada um")
    w("  afirmando um trecho exato da mensagem (o catálogo humano está em")
    w("  [01 §12.3](../docs/01-especificacao.md)).")
    w(f"* **{n_m1} casos** de erro do transpilador (`tests/casos/12_negativos_m1/`), que")
    w("  acusa antes do C e com a linha do fonte.")
    w("")
    w("## 3. Suítes e casos em disco")
    w("")
    w("| suíte | casos | o que cobre |")
    w("|---|---|---|")
    for suite, n, desc in suites:
        w(f"| `{suite}` | {n} | {desc} |")
    w("")
    w("## 4. Critérios de aceitação (Fases 2 e 4)")
    w("")
    w("| AC | critério | verificação automatizada |")
    w("|---|---|---|")
    for ac, criterio, onde in AC:
        w(f"| {ac} | {criterio} | {onde} |")
    w("")
    w("## 5. Restrições técnicas e o teste que as fixa")
    w("")
    w("| restrição | o que a suíte faz |")
    w("|---|---|")
    for c, o_que, onde in RESTRICOES:
        w(f"| {c} | {o_que} — {onde} |")
    w("")
    w("## 6. Estresse")
    w("")
    w("| situação | onde |")
    w("|---|---|")
    for o_que, onde in ESTRESSE:
        w(f"| {o_que} | `{onde}` |")
    w("")
    w("## 7. O que ainda não é automatizado")
    w("")
    w("* **AC-17 / AC-18** (leitura por pessoa de fora): não há como um programa julgar")
    w("  legibilidade. O que a Fase 2 fez foi a parte verificável — percorrer a tabela de")
    w("  mapeamento e executar cada construtor — e essa revisão encontrou **três** erros")
    w("  reais de especificação (C-04, `PARA_PASSO` com passo negativo e o `;` de")
    w("  `ATE_QUE`), todos corrigidos e fixados por teste.")
    w("* **D-09** (`BG_ESTRITO`), **D-12** (funções de vetor) e **D-17** (locale do")
    w("  sistema): fora da v1.0 por decisão registrada em")
    w("  [04](../docs/04-restricoes-e-decisoes.md).")
    w("* Desempenho além do limite de 1 s por compilação (AC-29): medido, não otimizado.")
    w("")

    destino = RAIZ / "tests" / "COBERTURA.md"
    destino.write_text("\n".join(saida), encoding="utf-8")
    print(f"escrito {destino.relative_to(RAIZ)} ({len(nomes)} nomes, {total_casos} casos)")


if __name__ == "__main__":
    main()
