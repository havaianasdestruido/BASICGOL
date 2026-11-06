"""Renderização MDX/Docusaurus a partir do inventário estático."""
from __future__ import annotations

import json
import re
from collections import Counter, defaultdict
from pathlib import Path

from .analise import Inventory, Reference, Source, Symbol, mask_code

CAT_LABELS = {
    "estrutura": "Programa e declarações", "tipos": "Tipos e limites",
    "controle": "Controle de fluxo", "operadores": "Operadores e lógica",
    "entrada-saida": "Entrada e saída", "matematica": "Matemática",
    "texto": "Texto", "caracteres": "Caracteres", "utilidades": "Utilidades",
    "configuracao": "Configuração e versão", "transpilador": "Macros do transpilador",
    "testes": "Macros dos testes", "outros": "Outras macros",
}
MODULE_LABELS = {"consola": "Console e diagnóstico", "matematica": "Matemática",
                 "texto": "Texto e conversões", "caractere": "Caracteres ASCII",
                 "utilidades": "Utilidades e sorteio", "basicgol": "Extensões opcionais"}
CONTROL = {"INICIO", "FIM", "FUNCAO", "PROCEDIMENTO", "RETORNE", "SE", "ENTAO", "SENAO_SE", "SENAO",
           "FIM_SE", "ENQUANTO", "FACA", "FIM_ENQUANTO", "REPITA", "ATE_QUE", "PARA", "PARA_PASSO",
           "ESCOLHA", "CASO", "CONTRARIO", "FIM_ESCOLHA", "PARE", "PROSSIGA", "VETOR", "TEXTO_FIXO",
           "REGISTRO", "FIM_REGISTRO", "ENUMERACAO", "FIM_ENUMERACAO", "TAMANHO_VETOR", "CONSTANTE",
           "ESTATICO", "EXTERNO"}
IO = {"ESCREVA", "ESCREVAL", "ESCREVA_LINHA", "ESCREVA_TEXTO", "ESCREVA_FMT", "ESCREVA_ERRO",
      "LEIA", "LEIA_OK", "LEIA_TEXTO", "LEIA_INTEIRO", "LEIA_REAL", "FMT", "SFMT", "ERRO", "AFIRME"}
MATH = {"ABS", "MINIMO", "MAXIMO", "POTENCIA", "RAIZ", "ARREDONDA", "PISO", "TETO", "DIVIDE", "RESTO"}
TEXT = {"TAMANHO_TEXTO", "COPIA_TEXTO", "JUNTA_TEXTO", "ACRESCENTA_TEXTO", "TRECHO_TEXTO",
        "COMPARA_TEXTO", "TEXTO_VAZIO", "FORMATA_TEXTO", "TEXTO_PARA_INTEIRO", "TEXTO_PARA_NATURAL",
        "TEXTO_PARA_REAL", "REAL_PARA_INTEIRO", "TEXTOS_IGUAIS", "TEXTOS_DIFERENTES"}
CHAR = {"E_DIGITO", "E_LETRA", "CARACTERE_MAIUSCULO", "CARACTERE_MINUSCULO"}
UTIL = {"SORTEIA", "INICIA_SORTEIO"}
OPS = {"E", "OU", "NAO", "MOD", "VERDADEIRO", "FALSO", "IGUAL", "DIFERENTE", "OU_EXCLUSIVO", "LOGICO_DE"}
PLATFORM = re.compile(r"__(?:GNUC__|clang__|MINGW32__|MINGW64__|cplusplus|STDC_VERSION__|"
                      r"APPLE__|linux__|unix__)|_MSC_VER|_WIN32|BG_USAR_EXTENSOES|BG_SEM_APELIDOS|"
                      r"INT_MIN|LONG_MIN|LLONG_MIN|CHAR_BIT|UINT_MAX")


def fence(text: str, language: str = "text") -> str:
    """Cria code fence MDX seguro para código que contenha crases."""
    runs = re.findall(r"`+", text)
    ticks = "`" * max(3, max((len(r) + 1 for r in runs), default=3))
    return f"{ticks}{language}\n{text.rstrip()}\n{ticks}\n"


def protect(text: str) -> str:
    """Escapa sintaxe MDX acidental em prosa, sem tocar em código Markdown."""
    parts = re.split(r"(`+[^`]*`+)", text, flags=re.S)
    for i, part in enumerate(parts):
        if part.startswith("`"):
            continue
        part = part.replace("{", "&#123;").replace("}", "&#125;")
        part = re.sub(r"<(https?://[^>]+|mailto:[^>]+)>", lambda m: f"[{m.group(1)}]({m.group(1)})", part)
        part = part.replace("<", "&lt;")
        parts[i] = part
    return "".join(parts)


def quote(text: str) -> str:
    """Transforma comentário em blockquote Markdown seguro para MDX."""
    return "\n".join("> " + protect(line) for line in text.splitlines()) + "\n"


def plain(text: str) -> str:
    """Reduz texto Markdown a uma descrição curta para metadados."""
    return re.sub(r"\s+", " ", re.sub(r"[`*#<>]", "", text)).strip()[:180]


def code_language(path: str) -> str:
    """Mapeia extensão de arquivo para linguagem de realce Prism."""
    return {".py": "python", ".sh": "bash", ".h": "c", ".c": "c", ".cpp": "cpp",
            ".json": "json", ".yml": "yaml", ".yaml": "yaml", ".txt": "text", ".md": "markdown"}.get(Path(path).suffix, "text")


class Renderer:
    def __init__(self, inv: Inventory, contracts: dict, repo_url: str, commit: str):
        """Inicializa índices de referências, símbolos e auditoria do renderizador."""
        self.inv = inv
        self.contracts = contracts
        self.repo_url = repo_url.rstrip("/")
        self.commit = commit
        self.pages: dict[str, str] = {}
        self.refs: dict[str, list[Reference]] = defaultdict(list)
        for ref in inv.references:
            self.refs[ref.symbol].append(ref)
        self.production = [s for s in inv.symbols.values()
                           if s.path.startswith(("include/", "src/", "tools/"))]
        self.macros = [s for s in inv.symbols.values() if s.kind == "macro"]
        self.functions = [s for s in inv.symbols.values() if s.kind == "função C"]
        self.canonical_macros = [s for s in self.macros if not self.macro_target(s)]
        self.audit: dict[str, list[str]] = defaultdict(list)

    def macro_target(self, symbol: Symbol) -> Symbol | None:
        """Resolve apelido curto para a macro canônica definida no cabeçalho público."""
        if symbol.kind != "macro" or not symbol.path.startswith("include/") or symbol.parameters or symbol.name.startswith("BG_"):
            return None
        target = [s for s in self.inv.by_name.get(symbol.body.strip(), [])
                  if s.kind == "macro" and s.path.startswith("include/")]
        return target[0] if len(target) == 1 else None

    def macro_category(self, symbol: Symbol) -> str:
        """Classifica macro por família semântica para navegação e índice."""
        names = {symbol.name.removeprefix("BG_"), *symbol.aliases}
        if symbol.path.startswith("tests/"):
            return "testes"
        if symbol.path.startswith("tools/"):
            return "transpilador"
        if names & CONTROL:
            return "estrutura"
        if names & IO:
            return "entrada-saida"
        if names & MATH:
            return "matematica"
        if names & TEXT:
            return "texto"
        if names & CHAR:
            return "caracteres"
        if names & UTIL:
            return "utilidades"
        if names & OPS:
            return "operadores"
        if "VERSAO" in symbol.name or symbol.name in {"BG_SEM_APELIDOS", "BG_USAR_EXTENSOES", "BG_TEM_EXTENSOES"} or symbol.name.endswith("_H") or symbol.name == "__USE_MINGW_ANSI_STDIO" or symbol.name == "BG_NORETURN":
            return "configuracao"
        if any(n.startswith(("INTEIRO", "NATURAL", "REAL", "LOGICO", "CARACTERE", "TEXTO", "VAZIO", "TAMANHO_DE", "BITS_DE", "MAXIMO_", "MINIMO_INTEIRO", "EPSILON_")) for n in names):
            return "tipos"
        return "outros"

    def slug(self, symbol: Symbol) -> str:
        """Gera rota estável por tipo/módulo, reunindo apelidos de macro."""
        name = re.sub(r"[^a-zA-Z0-9_-]+", "-", symbol.name).strip("-").lower().lstrip("_") or "simbolo"
        if symbol.kind == "macro":
            short = next((a for a in symbol.aliases if not a.startswith("BG_")), symbol.name.removeprefix("BG_"))
            return f"referencia/linguagem/{self.macro_category(symbol)}/{short.lower().lstrip('_') or 'simbolo'}"
        if symbol.kind == "função C":
            if symbol.path.startswith(("include/", "src/")):
                return f"referencia/funcoes/{symbol.module}/{name}"
            if symbol.path.startswith("tools/"):
                return f"internos/transpilador/{name}"
            return f"internos/c/{Path(symbol.path).with_suffix('').as_posix().replace('/', '-')}/{name}"
        area = {"python": "python", "shell": "shell"}.get(symbol.language, "c")
        stem = Path(symbol.path).with_suffix("").as_posix().replace("/", "-")
        return f"internos/{area}/{stem}/{name.replace('.', '-')}"

    def link(self, symbol: Symbol) -> str:
        """Cria link Markdown para a página de um símbolo."""
        return f"[`{symbol.name}`](/{self.slug(symbol)})"

    def gh(self, path: str, line: int | None = None) -> str:
        """Monta permalink GitHub para arquivo e linha no commit do build."""
        url = f"{self.repo_url}/blob/{self.commit}/{path}"
        return url + (f"#L{line}" if line else "")

    def gh_link(self, path: str, line: int | None = None, label: str | None = None) -> str:
        """Cria link Markdown para fonte/linha no GitHub."""
        return f"[{label or path}]({self.gh(path, line)})"

    def page(self, path: str, title: str, body: str, label: str | None = None,
             description: str | None = None, slug: str | None = None, position: int | None = None) -> None:
        """Registra página MDX com frontmatter determinístico e metadados."""
        fm = {"title": title, "sidebar_label": label or title,
              "description": plain(description or body[:220])}
        if slug:
            fm["slug"] = slug
        if position is not None:
            fm["sidebar_position"] = position
        header = "---\n" + "\n".join(f"{k}: {json.dumps(v, ensure_ascii=False)}" for k, v in fm.items()) + "\n---\n\n"
        self.pages[path.rstrip("/") + ".mdx"] = header + body.rstrip() + "\n"

    def category(self, folder: str, label: str, position: int) -> None:
        """Registra categoria de sidebar Docusaurus."""
        self.pages[folder + "/_category_.json"] = json.dumps(
            {"label": label, "position": position, "collapsible": True, "collapsed": True}, ensure_ascii=False, indent=2) + "\n"

    def contract(self, symbol: Symbol) -> dict:
        """Resolve contrato editorial por nome, padrão, apelido ou função relacionada."""
        exact = self.contracts.get("symbols", {}).get(symbol.name)
        if exact:
            return exact
        target = self.macro_target(symbol)
        if target:
            return self.contract(target)
        if symbol.kind == "macro" and symbol.body.strip():
            targets = [s for s in self.inv.by_name.get(symbol.body.strip(), []) if s.kind == "função C"]
            if len(targets) == 1:
                inherited = dict(self.contract(targets[0]))
                inherited.setdefault("resumo", f"Forma macro de {symbol.name}; encaminha ao contrato de {targets[0].name}.")
                return inherited
        for pattern in self.contracts.get("patterns", []):
            if symbol.name.startswith(pattern["prefix"]):
                return {k: v for k, v in pattern.items() if k != "prefix"}
        # Macros de biblioteca cujo nome curto mapeia para uma função bg_.
        if symbol.kind == "macro":
            for name in re.findall(r"[A-Za-z_]\w*", symbol.body):
                candidates = self.inv.by_name.get(name, [])
                if len(candidates) == 1 and candidates[0].kind == "função C":
                    inherited = dict(self.contract(candidates[0]))
                    inherited["resumo"] = f"{symbol.name}: interface macro da biblioteca. " + inherited.get("resumo", "")
                    return inherited
        return {}

    def names_for(self, symbol: Symbol) -> set[str]:
        """Retorna nome canônico e apelidos usados na busca de evidências."""
        return {symbol.name, *symbol.aliases}

    def direct_refs(self, symbol: Symbol) -> list[Reference]:
        """Reúne referências diretas do símbolo e de seus apelidos sem duplicatas."""
        seen = set()
        result = []
        for name in sorted(self.names_for(symbol)):
            for ref in self.refs.get(name, []):
                key = (ref.path, ref.line, ref.kind, ref.symbol)
                if key not in seen:
                    seen.add(key)
                    result.append(ref)
        return sorted(result, key=lambda r: (r.path, r.line, r.symbol))

    def source_snippet(self, path: str, line: int, context: int = 2, end: int | None = None) -> tuple[str, int, int, str]:
        """Recorta linhas de contexto e resolve fonte real de heredoc virtual."""
        source = self.inv.sources[path]
        local = line - source.offset
        lines = source.lines
        start = max(1, local - context)
        stop = min(len(lines), (end - source.offset if end else local) + context)
        text = "\n".join(lines[start - 1:stop])
        return text, start + source.offset, stop + source.offset, source.parent or source.path

    def render_refs(self, symbol: Symbol) -> str:
        """Renderiza arquivos, linhas, contextos e trechos de uso em testes."""
        refs = [r for r in self.direct_refs(symbol) if r.kind != "tabela de expansão"]
        if not refs:
            return (":::note[Sem uso direto localizado]\n"
                    "Nenhuma referência direta a este nome foi encontrada nos arquivos de teste. "
                    "Isso pode ser esperado para guardas de inclusão, configuração ou código exercitado "
                    "indiretamente; não é uma afirmação de cobertura zero.\n:::\n")
        groups: dict[str, list[Reference]] = defaultdict(list)
        for ref in refs:
            groups[ref.path].append(ref)
        out = [f"**{len(refs)} ocorrência(s) em {len(groups)} arquivo(s) de teste.** ",
               "Os links apontam para linhas exatas; os trechos mostram o contexto local. ",
               "Uma menção não prova que todos os ramos foi executado.\n\n"]
        for path, items in sorted(groups.items()):
            containers = sorted({r.container for r in items})
            lines = sorted({r.line for r in items})
            kinds = sorted({r.kind for r in items})
            label = path if "::" not in path else f"{items[0].parent} · heredoc na linha {items[0].offset + 1}"
            out.append(f"### {label}\n\n")
            out.append(f"Contexto: {', '.join('`' + c + '`' for c in containers[:8])}{' …' if len(containers) > 8 else ''}. ")
            out.append(f"Linhas: {', '.join(str(n) for n in lines[:35])}{' …' if len(lines) > 35 else ''}. ")
            out.append(f"Tipo de evidência: {', '.join(kinds)}; via {', '.join(sorted({r.symbol for r in items}))}.\n\n")
            intervals = []
            for line in lines:
                lo, hi = max(1, line - 2), line + 2
                if intervals and lo <= intervals[-1][1] + 1:
                    intervals[-1][1] = max(intervals[-1][1], hi)
                else:
                    intervals.append([lo, hi])
            for lo, hi in intervals[:6]:
                text, start, stop, real = self.source_snippet(path, lo, context=0, end=hi)
                out.append(f"<details>\n<summary>Linhas {start}–{stop} · abrir contexto</summary>\n\n")
                out.append(fence(text, code_language(real)) + "\n")
                out.append(self.gh_link(real, start, label="Ver no GitHub") + "\n\n</details>\n\n")
            if len(intervals) > 6:
                out.append(f"_{len(intervals) - 6} outros blocos de contexto: use os links de linha e o JSON de inventário._\n\n")
        expansions = [r for r in self.direct_refs(symbol) if r.kind == "tabela de expansão"]
        if expansions:
            out.append("Também aparece na tabela de expansão conferida por `cc -E -dM`: " +
                       ", ".join(self.gh_link(r.path, r.line, label=f"L{r.line}") for r in expansions) + ".\n")
        return "".join(out)

    def indirect_refs(self, symbol: Symbol) -> str:
        """Mostra chamadores e testes deles como evidência indireta explícita."""
        callers = [s for s in self.production if s.key != symbol.key and symbol.name in s.calls]
        if not callers:
            return ""
        out = ["## Evidência indireta\n\n", "Estes símbolos chamam/encaminham para este nome. ",
               "As referências deles ajudam a rastrear o caminho, mas não substituem um teste específico deste símbolo.\n\n"]
        found = False
        for caller in callers[:12]:
            refs = self.direct_refs(caller)
            files = sorted({r.path for r in refs})
            if files:
                found = True
                out.append(f"- {self.link(caller)} → {', '.join(self.gh_link(p, label=p) for p in files[:5])}{' …' if len(files) > 5 else ''}\n")
        if not found:
            out.append("Nenhum arquivo de teste foi localizado para os chamadores diretos.\n")
        return "".join(out) + "\n"

    def modes_for(self, symbol: Symbol) -> list[dict]:
        """Calcula quais modos do harness alcançam o símbolo pelo grafo de chamadas."""
        graph = {s.name: set(s.calls) for s in self.inv.symbols.values() if s.kind == "função C"}
        result = []
        for mode in self.inv.modes:
            seen = set()
            todo = list(mode["calls"])
            while todo:
                name = todo.pop()
                if name in seen:
                    continue
                seen.add(name)
                todo.extend(graph.get(name, ()))
            if symbol.name in seen:
                result.append(mode)
        return result

    def runner_mode_lines(self, mode: str) -> list[tuple[int, str]]:
        """Localiza invocações literais de um modo no runner da biblioteca."""
        path = "tests/run_biblioteca.sh"
        source = self.inv.sources.get(path)
        if not source:
            return []
        pattern = re.compile(r"(?<![\w-])" + re.escape(mode) + r"(?![\w-])")
        return [(i, line) for i, line in enumerate(source.lines, 1) if pattern.search(line)]

    def render_modes(self, symbol: Symbol) -> str:
        """Renderiza modos de integração/falha fatal ligados ao símbolo."""
        modes = self.modes_for(symbol)
        if not modes:
            return ""
        out = ["### Modos de integração/falha fatal\n\n"]
        for mode in modes:
            out.append(f"- [`{mode['mode']}`](/testes/modos/{mode['mode'].lstrip('-')}) — ")
            runner = self.runner_mode_lines(mode["mode"])
            if runner:
                line, text = runner[0]
                out.append(self.gh_link("tests/run_biblioteca.sh", line, label=f"invocação L{line}") + ": `" + text.strip().replace("`", "") + "`")
            out.append("\n")
        return "".join(out) + "\n"

    def nearby_directives(self, symbol: Symbol) -> list[str]:
        """Diretivas vizinhas que afetam atributos/diagnósticos, não só o corpo."""
        source = self.inv.sources.get(symbol.path)
        if not source:
            return []
        lo, hi = max(0, symbol.line - 7), min(len(source.lines), symbol.end + 7)
        return [line.strip() for line in source.lines[lo:hi]
                if re.match(r"\s*#\s*(if|ifdef|ifndef|elif|else|endif|pragma)", line)
                and (PLATFORM.search(line) or "pragma" in line)]

    def portability(self, symbol: Symbol) -> str:
        """Renderiza guards, diretivas, marcadores e ressalvas de compilador/OS."""
        contract = self.contract(symbol)
        nearby = self.nearby_directives(symbol)
        markers = sorted(set(PLATFORM.findall(symbol.body + " " + symbol.signature + " " + " ".join(c["condition"] for c in symbol.conditions) + " " + " ".join(nearby))))
        directives = [line.strip() for line in symbol.body.splitlines() if re.match(r"\s*#\s*(if|ifdef|ifndef|elif|else)", line)]
        out = ["## Compiladores, padrão C e sistema operacional\n\n"]
        if symbol.conditions:
            out.append("**Condições que envolvem a definição:**\n\n")
            for cond in symbol.conditions:
                out.append(f"- `{cond['condition']}` ({self.gh_link(symbol.path, cond['line'], label='fonte')})\n")
            out.append("\n")
        else:
            out.append("Nenhum ramo condicional externo foi detectado para este símbolo. ")
        if directives:
            out.append("**Diretivas dentro do corpo:**\n\n")
            for directive in directives:
                out.append(f"- `{directive}`\n")
            out.append("\n")
        if nearby:
            out.append("**Diretivas próximas no fonte** (podem afetar atributos/diagnósticos ao redor da definição):\n\n")
            for directive in nearby:
                out.append(f"- `{directive}`\n")
            out.append("\n")
        if markers:
            out.append("Marcadores de plataforma/representação presentes: " + ", ".join(f"`{m}`" for m in markers) + ".\n\n")
        if not symbol.conditions and not directives and not nearby:
            out.append("A implementação é compartilhada no fonte; isso não certifica compiladores ou sistemas não executados.\n\n")
        notes = list(contract.get("portabilidade", []))
        if symbol.language == "shell":
            notes.append("Runner Bash: requer Bash e utilitários Unix; no Windows o projeto usa Git Bash/MSYS2. Não é um script CMD/PowerShell.")
        elif symbol.language == "python":
            notes.append("Script Python: usa a biblioteca padrão e pathlib/subprocess quando aplicável; caminhos não devem depender do separador de um único OS.")
        elif symbol.path.startswith("tools/"):
            notes.append("O transpilador abre entrada/saída em modo binário; BG_FORMATO só adiciona atributo printf em GCC/Clang.")
        elif symbol.module == "caractere":
            notes.append("ASCII explícito, sem ctype/locale; o sinal de char não muda a classificação.")
        elif symbol.module == "texto":
            notes.append("Tamanhos são bytes. Conversões reais usam a locale C corrente; o projeto não chama setlocale.")
        elif symbol.module == "matematica":
            notes.append("Limites de tipos vêm do alvo. Funções libm exigem -lm no caminho GCC/Clang documentado; o CRT do MSVC não usa essa flag.")
        elif symbol.module == "utilidades":
            notes.append("Gerador próprio com estado global; não é criptográfico nem thread-safe. A interface unsigned acompanha o alvo.")
        if symbol.kind == "macro" and symbol.name not in {"BG_NORETURN", "__USE_MINGW_ANSI_STDIO", "BG_FORMATO"} and not symbol.conditions:
            notes.append("Fora das extensões opcionais, a linguagem exige C11; diferenças de tipo, libc e CRT continuam sendo as do alvo.")
        for note in dict.fromkeys(notes):
            out.append(f"- {protect(note)}\n")
        out.append("\n:::important[Não confunda implementação com certificação]\n"
                   "A matriz histórica distingue executado, compilado, avaliado e não testado. "
                   "Consulte [a matriz](/projeto/11-matriz-compatibilidade), [os compiladores](/projeto/10-compiladores) "
                   "e os logs atuais dos workflows antes de chamar um alvo de suportado.\n:::\n")
        return "".join(out)

    def render_symbol(self, symbol: Symbol) -> None:
        """Monta a página completa de um símbolo com contrato, fonte, testes e portabilidade."""
        contract = self.contract(symbol)
        aliases = sorted(symbol.aliases)
        title = symbol.name + (f" · {' · '.join(aliases)}" if aliases and symbol.kind == "macro" else "")
        summary = contract.get("resumo") or (symbol.comment.splitlines()[0] if symbol.comment else "")
        if not summary and symbol.kind == "macro":
            body = symbol.body.strip()
            category = self.macro_category(symbol)
            group = self.contracts.get("macro_groups", {}).get(category, "")
            summary = (f"Macro {'com argumentos' if symbol.parameters else 'de objeto'} que expande para `{body}`. "
                       + group) if body else "Macro de objeto sem corpo de expansão."
            contract = dict(contract)
            contract.setdefault("mecanica", group or "A expansão literal abaixo é o contrato desta macro.")
        if not summary and symbol.kind == "função C" and symbol.name == "main":
            summary = "Ponto de entrada do transpilador: interpreta a CLI, orquestra leitura/tradução, trata erros de E/S e devolve o código de saída." if symbol.path.startswith("tools/") else "Ponto de entrada do harness: seleciona modos de stdin/falha fatal ou executa os grupos unitários e resume falhas."
        if not summary and symbol.kind == "função C" and symbol.path.startswith("tests/"):
            summary = f"Função do arquivo de teste `{symbol.path}`; o corpo abaixo e os casos associados mostram o cenário e o resultado conferido."
        if not summary:
            summary = f"{symbol.kind} definida em {symbol.path}; contrato extraído da assinatura e do corpo abaixo."
            self.audit["sem_resumo_editorial"].append(symbol.key)
        out = [f":::tip[Página gerada do fonte]\nGerada por `tools/gerar_documentacao.py`. "
               f"Definição: {self.gh_link(symbol.path, symbol.line, label=f'{symbol.path}:{symbol.line}')}.\n:::\n\n",
               protect(summary) + "\n\n"]
        if symbol.kind == "macro":
            out.append(f"**Tipo:** macro {'com argumentos' if symbol.parameters else 'de objeto'} · **Módulo/categoria:** {CAT_LABELS[self.macro_category(symbol)]}.\n\n")
            if aliases:
                out.append("**Nomes equivalentes:** " + ", ".join(f"`{a}`" for a in aliases) +
                           ". Os testes de qualquer um desses nomes são reunidos nesta página.\n\n")
            if symbol.name.startswith("BG_") and not aliases and symbol.path == "include/basicgol.h":
                out.append("Não foi encontrado nome curto equivalente no cabeçalho atual; use a forma prefixada.\n\n")
        else:
            out.append(f"**Tipo:** {symbol.kind} · **Visibilidade:** {symbol.visibility} · **Arquivo:** `{symbol.path}`.\n\n")
        out.append("## Assinatura e expansão\n\n" + fence(symbol.signature, code_language(symbol.path)))
        if symbol.parameters:
            out.append("**Parâmetros formais:** " + ", ".join(f"`{p}`" for p in symbol.parameters) + ".\n\n")
        if symbol.kind == "macro":
            out.append("**Corpo da macro:**\n\n" + fence(symbol.body or "(vazio)", "c"))
            target = self.macro_target(symbol)
            if target:
                out.append(f"Encadeia para {self.link(target)}.\n\n")
        if symbol.prototypes:
            out.append("**Declarações públicas localizadas:**\n\n")
            for proto in symbol.prototypes:
                label = f"{proto['path']}:{proto['line']}"
                out.append(f"- {self.gh_link(proto['path'], proto['line'], label=label)}\n")
                if proto["comment"]:
                    out.append(quote(proto["comment"]))
            out.append("\n")
        out.append("## Como funciona\n\n")
        mechanics = contract.get("mecanica") or symbol.comment
        if mechanics:
            out.append(protect(mechanics) + "\n\n")
        else:
            out.append("O comportamento normativo é o corpo extraído abaixo. Não foi inferida uma garantia adicional sem contrato/documentação no fonte.\n\n")
        if contract.get("retorno"):
            out.append("**Retorno:** " + protect(contract["retorno"]) + "\n\n")
        if contract.get("cuidados"):
            out.append(":::caution[Contrato e armadilhas]\n")
            for note in contract["cuidados"]:
                out.append("- " + protect(note) + "\n")
            out.append(":::\n\n")
        if contract.get("exemplo"):
            out.append("## Exemplo de uso\n\n" + fence(contract["exemplo"], code_language(symbol.path)) + "\n")
        if symbol.comment and symbol.comment != mechanics:
            out.append("## Comentário do fonte\n\n" + quote(symbol.comment) + "\n")
        if symbol.body and symbol.kind != "macro":
            long = len(symbol.body.splitlines()) > 45
            if long:
                out.append(f"<details>\n<summary>Implementação completa · {len(symbol.body.splitlines())} linhas</summary>\n\n")
            else:
                out.append("## Implementação\n\n")
            out.append(fence(symbol.body, code_language(symbol.path)) + "\n")
            if long:
                out.append("</details>\n\n")
        related_macros = [m for m in self.canonical_macros if symbol.name in m.calls]
        if related_macros:
            out.append("## Interface BASICGOL relacionada\n\n" + ", ".join(self.link(m) for m in related_macros) + "\n\n")
        deps = [n for n in symbol.calls if n in self.inv.by_name and n != symbol.name]
        if deps:
            links = []
            for name in deps:
                candidates = self.inv.by_name[name]
                if len(candidates) == 1:
                    links.append(self.link(candidates[0]))
            if links:
                out.append("**Nomes internos referenciados:** " + ", ".join(links) + ".\n\n")
        out.append("## Como este símbolo aparece nos testes\n\n")
        out.append(self.render_refs(symbol))
        out.append(self.render_modes(symbol))
        out.append(self.indirect_refs(symbol))
        if symbol.path == "tools/basicgolc.c":
            match = re.search(r"verifica_w(\d+)", symbol.name)
            if match:
                cases = [c for c in self.inv.cases if c["name"].startswith("w" + match.group(1) + "_")]
                if cases:
                    out.append("**Caso de aviso correspondente:** " + ", ".join(f"[{c['name']}](/testes/casos/{c['suite']}/{c['name']})" for c in cases) + ".\n\n")
            out.append(":::note[Testes de caixa-preta]\nO transpilador é exercitado pela CLI em `tests/run.sh`, "
                       "`tests/run_portabilidade.sh` e `tests/fuzz_basicgolc.py`. Uma ligação no grafo de chamadores "
                       "não prova que cada ramo desta função foi coberto.\n:::\n\n")
        out.append(self.portability(symbol))
        if symbol.kind == "macro":
            folder = f"referencia/linguagem/{self.macro_category(symbol)}"
            label = next((a for a in aliases if not a.startswith("BG_")), symbol.name)
            self.page(self.slug(symbol), title, "".join(out), label=label, description=summary)
        elif symbol.kind == "função C" and symbol.path.startswith(("include/", "src/")):
            self.page(self.slug(symbol), title, "".join(out), label=symbol.name, description=summary)
        else:
            self.page(self.slug(symbol), title, "".join(out), label=symbol.name, description=summary)

    def render_indexes(self) -> None:
        """Gera índices de referência por família de macro e módulo de biblioteca."""
        counts = Counter(s.kind for s in self.inv.symbols.values())
        self.category("referencia", "Referência da API", 20)
        self.category("referencia/linguagem", "Palavras-chave e macros", 10)
        self.category("referencia/funcoes", "Funções da biblioteca", 20)
        body = ["# Referência viva\n\n",
                f"Inventário extraído de **{len(self.inv.symbols)} símbolos**: "
                f"{counts['macro']} macros, {counts['função C']} funções C, "
                f"{counts['função Python']} funções Python e {counts['função Shell']} funções Shell. "
                "Apelidos curtos/prefixados são reunidos na mesma página de macro.\n\n",
                ":::note[O que é automático]\nNomes, assinaturas, expansões, guards, chamadores, arquivos de teste, "
                "linhas, casos dourados e modos fatais vêm do repositório a cada build. Contratos editoriais são "
                "complementados por comentários/docstrings; um símbolo novo aparece mesmo antes de ganhar prosa especial.\n:::\n\n",
                "Use a busca local para procurar qualquer nome, incluindo apelidos. "
                "O [inventário JSON](pathname:///api/inventario.json) é a saída determinística para auditoria.\n\n",
                "## Macros e palavras-chave\n\n"]
        grouped = defaultdict(list)
        for macro in self.canonical_macros:
            grouped[self.macro_category(macro)].append(macro)
        for category, symbols in sorted(grouped.items()):
            label = CAT_LABELS[category]
            folder = f"referencia/linguagem/{category}"
            self.category(folder, label, 10 + len(body))
            rows = ["# " + label + "\n\n", "| Nome(s) | Contrato | Usos diretos em testes | Portabilidade | Fonte |\n",
                    "|---|---|---:|---|---|\n"]
            for symbol in sorted(symbols, key=lambda s: s.name):
                contract = self.contract(symbol)
                summary = protect(plain(contract.get("resumo") or symbol.comment or symbol.body or "Ver definição")[:130]).replace("|", "\\|")
                refs = self.direct_refs(symbol)
                files = len({r.path for r in refs})
                port = "condicional" if symbol.conditions or PLATFORM.search(symbol.body) else "compartilhada"
                names = " / ".join(symbol.aliases)
                rows.append(f"| {self.link(symbol)}{chr(32) + chr(96) + names + chr(96) if names else ''} | {summary} | {files} arquivo(s) | {port} | {self.gh_link(symbol.path, symbol.line, label='fonte')} |\n")
            self.page(folder + "/index", label, "".join(rows), label="Visão geral", position=0)
            body.append(f"- [{label}](/{folder}/) — {len(symbols)} página(s)\n")
        body.append("\n## Funções C da biblioteca\n\n")
        modules = defaultdict(list)
        for function in self.functions:
            if function.path.startswith(("include/", "src/")):
                modules[function.module].append(function)
        for module, symbols in sorted(modules.items()):
            label = MODULE_LABELS.get(module, module)
            folder = f"referencia/funcoes/{module}"
            self.category(folder, label, 10)
            rows = ["# " + label + "\n\n", "| Função | Visibilidade | Resumo | Arquivos de teste | Fonte |\n", "|---|---|---|---:|---|\n"]
            for symbol in sorted(symbols, key=lambda s: s.name):
                summary = protect(plain(self.contract(symbol).get("resumo") or symbol.comment or "Ver corpo")[:150]).replace("|", "\\|")
                files = len({r.path for r in self.direct_refs(symbol)})
                rows.append(f"| {self.link(symbol)} | {symbol.visibility} | {summary} | {files} | {self.gh_link(symbol.path, symbol.line, label='fonte')} |\n")
            self.page(folder + "/index", label, "".join(rows), label="Visão geral", position=0)
            body.append(f"- [{label}](/{folder}/) — {len(symbols)} função(ões)\n")
        body.append("\n## Funções internas\n\nVeja [o inventário interno](/internos/) para transpilador, testes, runners e scripts Python.\n")
        lang_rows = ["# Palavras-chave e macros\n\n"]
        for category, symbols in sorted(grouped.items()):
            lang_rows.append(f"- [{CAT_LABELS[category]}](/referencia/linguagem/{category}/) — {len(symbols)} página(s)\n")
        self.page("referencia/linguagem/index", "Palavras-chave e macros", "".join(lang_rows), label="Visão geral", position=0)
        func_rows = ["# Funções da biblioteca\n\n"]
        for module, symbols in sorted(modules.items()):
            func_rows.append(f"- [{MODULE_LABELS.get(module, module)}](/referencia/funcoes/{module}/) — {len(symbols)} função(ões)\n")
        self.page("referencia/funcoes/index", "Funções da biblioteca", "".join(func_rows), label="Visão geral", position=0)
        self.page("referencia/index", "Referência viva", "".join(body), label="Visão geral", position=0)

    def render_tests(self) -> None:
        """Gera índice de suítes, casos, harness e modos de integração."""
        self.category("testes", "Testes e rastreabilidade", 30)
        suites = defaultdict(list)
        for case in self.inv.cases:
            suites[case["suite"]].append(case)
        body = ["# Testes e rastreabilidade\n\n",
                f"**{len(self.inv.cases)} casos em disco**, {len(suites)} suítes e {len(self.inv.modes)} modos "
                "unitários/integração extraídos do harness. Contagens são estáticas: não são o total impresso por uma execução.\n\n",
                "```bash\nmake test\nmake test-biblioteca\nmake test-portabilidade\nmake fuzz\n```\n\n",
                "## Caminhos de execução\n\n",
                "- **M0:** o compilador C processa diretamente as macros de `basicgol.h`.\n",
                "- **M1:** `basicgolc` traduz blocos/açúcar para C; o compilador C ainda verifica tipos e liga a biblioteca.\n",
                "- Casos de execução comparam stdout, stderr declarado e status com o resultado dourado principal ou alternativo.\n",
                "- Negativos exigem falha e diagnóstico; avisos exigem W-xx sem necessariamente falhar a tradução.\n",
                "- CR é normalizado nas comparações para acomodar o modo texto do CRT Windows.\n\n",
                "| Suíte | Casos | Caminhos | Registro no runner |\n|---|---:|---|---|\n"]
        for suite, cases in sorted(suites.items()):
            folder = f"testes/casos/{suite}"
            self.category(folder, suite, 10)
            paths = sorted({("M0" if c["m0"] else "") + ("+M1" if c["m1"] else "") for c in cases})
            registered = "sim" if all(c["runner_registered"] for c in cases) else "revisar"
            body.append(f"| [{suite}](/{folder}/) | {len(cases)} | {', '.join(paths)} | {registered} |\n")
            rows = [f"# Suíte {suite}\n\n", "| Caso | Tipo | M0 | M1 | Símbolos | Arquivos |\n|---|---|---|---|---:|---:|\n"]
            for case in cases:
                rows.append(f"| [{case['name']}](/{folder}/{case['name']}) | {case['mode']} | {'sim' if case['m0'] else 'não'} | {'sim' if case['m1'] else 'não'} | {len(case['symbols'])} | {len(case['files'])} |\n")
                self.render_case(case)
            self.page(folder + "/index", suite, "".join(rows), label="Casos", position=0)
        body.append("\n:::caution[Descoberta não altera o runner]\nUm diretório novo aparece aqui automaticamente. "
                    "Para ser executado por `make test`, ele também precisa seguir a seleção de suítes de `tests/run.sh`; "
                    "a coluna acima mostra se o nome da suíte aparece nesse runner.\n:::\n\n")
        body.append("## Harness, runners e modos\n\n- [Testes unitários e integração](/testes/harness)\n"
                    "- [Modos fatais e de stdin](/testes/modos/)\n- [Funções dos runners](/internos/)\n"
                    "- [Cobertura versionada](/testes/cobertura-versionada)\n")
        self.page("testes/index", "Testes e rastreabilidade", "".join(body), label="Visão geral", position=0)
        self.render_harness()
        self.category("testes/modos", "Modos de integração", 30)
        mode_rows = ["# Modos de integração e falha fatal\n\n",
                     "Cada modo é uma entrada de linha de comando de `tests/test_biblioteca.c`. "
                     "O runner verifica o código de saída e o diagnóstico; não basta o processo terminar.\n\n",
                     "| Modo | Ramo C | Chamadas | Invocação no runner |\n|---|---|---|---|\n"]
        for mode in self.inv.modes:
            slug = mode["mode"].lstrip("-")
            runner = self.runner_mode_lines(mode["mode"])
            calls = ", ".join(f"`{c}`" for c in mode["calls"])
            mode_rows.append(f"| [`{mode['mode']}`](/testes/modos/{slug}) | {self.gh_link(mode['path'], mode['line'], label='ramo')} | {calls} | {self.gh_link('tests/run_biblioteca.sh', runner[0][0], label='invocação') if runner else 'não localizada'} |\n")
            out = [f"# Modo `{mode['mode']}`\n\n", f"Ramo extraído de {self.gh_link(mode['path'], mode['line'])}.\n\n",
                   "## Código executado\n\n", fence(mode["body"], "c"), "\n"]
            if mode["conditions"]:
                out.append("## Condições de plataforma\n\n")
                for cond in mode["conditions"]:
                    out.append(f"- `{cond['condition']}`\n")
                out.append("\n")
            out.append("## Como o runner usa este modo\n\n")
            if runner:
                for line, text in runner:
                    out.append(f"- {self.gh_link('tests/run_biblioteca.sh', line, label=f'L{line}')}: `{text.strip()}`\n")
                if mode["mode"].startswith("--"):
                    out.append("\nO modo de stdin espera sucesso e a saída literal conferida pelo runner.\n")
                elif "representabilidade" in "\n".join(t for _, t in runner):
                    out.append("\nAceita **status 3 sem stderr** quando o caso é representável na plataforma, ou **status 1 com o trecho esperado** de erro fatal.\n")
                else:
                    out.append("\nExige **status 1** e o trecho de stderr declarado na invocação.\n")
            else:
                out.append("Nenhuma invocação literal foi localizada; revisar o harness.\n")
            self.page(f"testes/modos/{slug}", mode["mode"], "".join(out), label=mode["mode"])
        self.page("testes/modos/index", "Modos de integração", "".join(mode_rows), label="Visão geral", position=0)

    def render_case(self, case: dict) -> None:
        """Renderiza um caso dourado/negativo/aviso com todos os seus arquivos."""
        out = [f"# {case['name']}\n\n", f"Suíte `{case['suite']}` · tipo **{case['mode']}** · ",
               f"M0: **{'sim' if case['m0'] else 'não'}** · M1: **{'sim' if case['m1'] else 'não'}**.\n\n"]
        if not case["runner_registered"]:
            out.append(":::warning[Suíte não localizada no runner]\nO caso foi descoberto em disco, mas o nome da suíte não aparece em tests/run.sh.\n:::\n\n")
        out.append("## Como é verificado\n\n")
        if case["mode"] == "negativo":
            out.append("A compilação/tradução deve falhar e o diagnóstico deve conter o texto esperado. "
                       "Um programa que compila sem erro não satisfaz este caso.\n\n")
        elif case["mode"] == "aviso":
            out.append("O transpilador deve emitir o código W-xx correspondente; aviso não é o mesmo que erro fatal.\n\n")
        else:
            out.append("Compila e executa pelos caminhos indicados, compara stdout normalizado, status e stderr declarado. "
                       "Arquivos alternativos, quando presentes, representam resultados de plataforma explicitamente aceitos.\n\n")
        if case["objetos"]:
            out.append("O marcador `objetos` seleciona compilação separada (`-c`) e ligação posterior.\n\n")
        out.append("## Arquivos do caso\n\n")
        for name, path in case["files"].items():
            source = self.inv.sources.get(path)
            text = source.text if source else (self.inv.root / path).read_text(encoding="utf-8")
            out.append(f"<details>\n<summary>{name} · {self.gh_link(path, label='fonte')}</summary>\n\n")
            out.append(fence(text, code_language(path)) + "\n</details>\n\n")
        out.append("## Símbolos documentados usados neste caso\n\n")
        links = []
        for name in case["symbols"]:
            candidates = self.inv.by_name.get(name, [])
            if len(candidates) == 1:
                links.append(self.link(candidates[0]))
        out.append(", ".join(sorted(set(links))) or "Nenhum símbolo do inventário localizado." )
        out.append("\n\n:::note[Rastreabilidade]\nA lista vem de identificadores no código de teste, não de comentários ou strings. "
                   "Ela não mede cobertura de ramos nem garante que toda chamada foi executada.\n:::\n")
        self.page(f"testes/casos/{case['suite']}/{case['name']}", case["name"], "".join(out), description=f"Caso {case['name']} da suíte {case['suite']}")

    def render_harness(self) -> None:
        """Lista funções C de teste e scripts runners com links de fonte."""
        body = ["# Harness unitário e runners\n\n",
                "`tests/test_biblioteca.c` contém grupos de verificações e modos de subprocesso. "
                "`VERIFICA` conta condições; falhas fatais são isoladas em modos porque `bg_erro` encerra o processo.\n\n",
                "## Grupos e funções de teste\n\n", "| Função | Localização | O que faz |\n|---|---|---|\n"]
        for symbol in sorted(self.inv.symbols.values(), key=lambda s: (s.path, s.line)):
            if symbol.kind == "função C" and symbol.path.startswith("tests/"):
                summary = protect(plain(self.contract(symbol).get("resumo") or symbol.comment or "Ver fonte")).replace("|", "\\|")
                body.append(f"| {self.link(symbol)} | {self.gh_link(symbol.path, symbol.line, label=symbol.path)} | {summary} |\n")
        body.append("\n## Scripts de teste\n\n")
        for path, source in sorted(self.inv.sources.items()):
            if path.startswith("tests/") and source.language in {"shell", "python"} and not source.parent:
                body.append(f"- {self.gh_link(path, label=path)} — {len(source.lines)} linhas; funções indexadas em [internos](/internos/).\n")
        body.append("\n## Cobertura e lacunas\n\nO [JSON de cobertura](pathname:///api/cobertura.json) lista símbolos sem referência direta. "
                    "Guardas/configuração e chamadas indiretas podem explicar uma lacuna; não a apague sem evidência.\n")
        self.page("testes/harness", "Harness unitário e runners", "".join(body))

    def render_internals(self) -> None:
        """Agrupa funções internas por linguagem e arquivo."""
        self.category("internos", "Código interno e ferramentas", 40)
        groups = defaultdict(list)
        for symbol in self.inv.symbols.values():
            if symbol.kind == "função C" and symbol.path.startswith(("include/", "src/")):
                continue
            if symbol.kind == "macro":
                continue
            groups[(symbol.language, Path(symbol.path).with_suffix("").as_posix())].append(symbol)
        body = ["# Inventário interno\n\n",
                "Funções do transpilador, harness C, runners Bash e scripts Python — incluindo o próprio gerador. "
                "Cada página traz assinatura, corpo, chamadores e referências de teste disponíveis.\n\n"]
        for (language, path), symbols in sorted(groups.items()):
            area = {"python": "Python", "shell": "Shell", "c": "C"}.get(language, language)
            folder = f"internos/{area.lower()}/{path.replace('/', '-')}"
            self.category(folder, f"{area} · {path}", 10)
            rows = [f"# {area} · {path}\n\n", f"Fonte: {self.gh_link(path)}.\n\n",
                    "| Função | Linhas | Resumo | Referências diretas |\n|---|---|---|---:|\n"]
            for symbol in sorted(symbols, key=lambda s: s.line):
                summary = protect(plain(self.contract(symbol).get("resumo") or symbol.comment or "Contrato no corpo")[:160]).replace("|", "\\|")
                rows.append(f"| {self.link(symbol)} | {symbol.line}–{symbol.end} | {summary} | {len(self.direct_refs(symbol))} |\n")
            self.page(folder + "/index", f"{area} · {path}", "".join(rows), label="Funções", position=0)
            body.append(f"- [{area} · {path}](/{folder}/) — {len(symbols)} função(ões)\n")
        self.page("internos/index", "Inventário interno", "".join(body), label="Visão geral", position=0)

    def render_examples(self) -> None:
        """Publica exemplos versionados e suas saídas douradas."""
        self.category("exemplos", "Exemplos executáveis", 50)
        body = ["# Exemplos executáveis\n\n", "Fontes de `docs/exemplos`, com saídas douradas quando versionadas. ",
                "`make exemplos` compila os exemplos; o bloco `10_exemplos` de `tests/run.sh` confere os resultados.\n\n"]
        for example in self.inv.examples:
            source = self.inv.sources[example["path"]]
            out = [f"# {example['name']}\n\n", f"Fonte: {self.gh_link(example['path'])}.\n\n", fence(source.text, "c"), "\n"]
            for name, path in example["files"].items():
                if path == example["path"]:
                    continue
                text = (self.inv.root / path).read_text(encoding="utf-8")
                out.append(f"## {name}\n\n" + fence(text, code_language(path)) + "\n")
            links = [self.link(self.inv.by_name[n][0]) for n in example["symbols"] if len(self.inv.by_name.get(n, [])) == 1]
            out.append("## Referência relacionada\n\n" + (", ".join(links) or "Nenhum símbolo inventariado.") + "\n")
            self.page(f"exemplos/{example['name']}", example["name"], "".join(out))
            body.append(f"- [{example['name']}](/exemplos/{example['name']})\n")
        self.page("exemplos/index", "Exemplos executáveis", "".join(body), label="Visão geral", position=0)

    def import_markdown(self, source_path: Path, target: str, title: str | None = None) -> None:
        """Importa documento original ajustando título, links e sintaxe MDX."""
        text = source_path.read_text(encoding="utf-8")
        match = re.match(r"#\s+(.+)\n", text)
        heading = match.group(1).strip() if match else source_path.stem
        if match:
            text = text[match.end():].lstrip()
        text = self.rewrite_links(text, source_path.parent)
        text = self.escape_mdx(text)
        position = int(match.group(1)) if (match := re.match(r"(\d+)-", source_path.stem)) else None
        self.page(target, title or heading, text, description=heading, position=position)

    def rewrite_links(self, text: str, base: Path) -> str:
        """Reescreve links relativos para páginas importadas ou permalinks GitHub."""
        mapping = {}
        for path in (self.inv.root / "docs").glob("*.md"):
            mapping[path.resolve()] = "/projeto/" + path.stem
        mapping[(self.inv.root / "README.md").resolve()] = "/projeto/visao-geral"
        mapping[(self.inv.root / "tests" / "COBERTURA.md").resolve()] = "/testes/cobertura-versionada"
        mapping[(self.inv.root / "tests" / "README.md").resolve()] = "/testes/leiame-original"
        def replace(match: re.Match) -> str:
            """Resolve um link Markdown individual contra o mapa de documentos/fontes."""
            label, dest = match.group(1), match.group(2)
            if re.match(r"[a-z]+:|#|/", dest):
                return match.group(0)
            split = dest.split("#", 1)
            path = (base / split[0]).resolve()
            anchor = "#" + split[1] if len(split) > 1 else ""
            if path in mapping:
                return f"[{label}]({mapping[path]}{anchor})"
            try:
                relative = path.relative_to(self.inv.root).as_posix()
                return f"[{label}]({self.gh(relative)}{anchor})"
            except ValueError:
                return match.group(0)
        return re.sub(r"\[([^\]]+)\]\(([^)\s]+)\)", replace, text)

    def escape_mdx(self, text: str) -> str:
        """Escapa prosa linha a linha sem modificar blocos de código fenced."""
        lines = text.splitlines(keepends=True)
        out = []
        fence_marker = None
        for line in lines:
            marker = re.match(r"^\s*(`{3,}|~{3,})", line)
            if marker:
                token = marker.group(1)[0] * 3
                if fence_marker is None:
                    fence_marker = token
                elif fence_marker == token:
                    fence_marker = None
                out.append(line)
                continue
            if fence_marker:
                out.append(line)
            else:
                out.append(protect(line))
        return "".join(out)

    def render_project(self) -> None:
        """Importa especificação, decisões, relatórios, README e cobertura versionada."""
        self.category("projeto", "Especificação e decisões", 60)
        body = ["# Especificação, decisões e relatórios\n\n",
                "Os documentos originais do repositório são importados a cada build, com links internos ajustados. "
                "Eles preservam o histórico e as ressalvas; a referência gerada mostra o estado atual dos símbolos.\n\n"]
        for path in sorted((self.inv.root / "docs").glob("*.md")):
            target = "projeto/" + path.stem
            self.import_markdown(path, target)
            title = re.match(r"#\s+(.+)", path.read_text(encoding="utf-8"))
            label = title.group(1) if title else path.stem
            body.append(f"- [{label}](/{target})\n")
        self.import_markdown(self.inv.root / "README.md", "projeto/visao-geral", title="Visão geral · README")
        body.append("- [Visão geral · README](/projeto/visao-geral)\n")
        self.page("projeto/index", "Especificação e decisões", "".join(body), label="Documentos do projeto", position=0)
        self.category("testes/documentos", "Documentos originais de testes", 80)
        self.import_markdown(self.inv.root / "tests" / "COBERTURA.md", "testes/cobertura-versionada", title="Cobertura versionada (relatório)")
        self.import_markdown(self.inv.root / "tests" / "README.md", "testes/leiame-original", title="README original dos testes")

    def render_build(self) -> None:
        """Gera página viva de Makefile, workflows e comandos de build."""
        self.category("guias", "Guias", 10)
        body = ["# Build, comandos e integração contínua\n\n",
                "## Makefile vivo\n\nTargets extraídos diretamente do Makefile:\n\n",
                "| Target | Dependências | Linha |\n|---|---|---:|\n"]
        for target in self.inv.make_targets:
            body.append(f"| `{target['name']}` | `{target['dependencies'] or '—'}` | {self.gh_link('Makefile', target['line'], label=str(target['line']))} |\n")
        body.append("\n## Workflows versionados\n\n")
        for path in sorted((self.inv.root / ".github" / "workflows").glob("*.yml")):
            relative = path.relative_to(self.inv.root).as_posix()
            text = path.read_text(encoding="utf-8")
            name = re.search(r"^name:\s*(.+)$", text, re.M)
            label = name.group(1).strip() if name else path.stem
            body.append(f"### {label}\n\n{self.gh_link(relative)}\n\n<details>\n<summary>Ver workflow</summary>\n\n")
            body.append(fence(text, "yaml") + "\n</details>\n\n")
        body.append("## Gerar este site\n\n```bash\npython3 -m venv .venv\n"
                    ".venv/bin/python -m pip install -r tools/documentacao/requirements.txt\n"
                    "cd website\nnpm ci\nnpm run build\n```\n\n"
                    "No Windows, ative o ambiente com `.venv\\Scripts\\activate` e use `python` em vez de `.venv/bin/python`. "
                    "Veja [publicação e manutenção](/guias/publicacao) para GitHub Pages e evolução automática.\n")
        self.page("guias/build", "Build e CI", "".join(body))

    def write(self, output: Path) -> dict:
        """Renderiza todas as páginas, guias e JSONs e escreve a árvore gerenciada."""
        for symbol in sorted(self.inv.symbols.values(), key=lambda s: s.key):
            if symbol.kind == "macro" and self.macro_target(symbol):
                continue
            self.render_symbol(symbol)
        self.render_indexes()
        self.render_tests()
        self.render_internals()
        self.render_examples()
        self.render_project()
        self.render_build()
        # Guias editoriais são fontes pequenas e versionadas; entram no mesmo build.
        manuals = self.inv.root / "website" / "manuais"
        if manuals.exists():
            for path in sorted(manuals.rglob("*")):
                if path.is_file() and path.suffix in {".md", ".mdx"}:
                    relative = path.relative_to(manuals).as_posix()
                    target = "guias/" + str(Path(relative).with_suffix("")) if not relative.startswith("inicio") else relative
                    target = str(Path(target).with_suffix(""))
                    self.pages[target + ".mdx"] = path.read_text(encoding="utf-8")
        for name, label, pos in [("guias", "Guias", 10), ("referencia", "Referência", 20),
                                 ("testes", "Testes", 30), ("internos", "Internos", 40),
                                 ("exemplos", "Exemplos", 50), ("projeto", "Projeto", 60)]:
            self.category(name, label, pos)
        inventory = {
            "schema": 1, "commit": self.commit,
            "counts": dict(Counter(s.kind for s in self.inv.symbols.values())),
            "symbols": [{"name": s.name, "aliases": s.aliases, "kind": s.kind, "language": s.language,
                         "path": s.path, "line": s.line, "end": s.end, "signature": s.signature,
                         "module": s.module, "visibility": s.visibility, "conditions": s.conditions,
                         "calls": s.calls, "url": "/" + self.slug(s), "source": self.gh(s.path, s.line)}
                        for s in sorted(self.inv.symbols.values(), key=lambda s: s.key)],
            "references": [r.__dict__ for r in self.inv.references],
            "cases": self.inv.cases, "modes": self.inv.modes, "warnings": self.inv.warnings,
        }
        no_refs = [s.key for s in sorted(self.inv.symbols.values(), key=lambda s: s.key)
                   if not self.direct_refs(s) and not (s.kind == "macro" and self.macro_target(s))]
        coverage = {"schema": 1, "sem_referencia_direta": no_refs,
                    "sem_resumo_editorial": sorted(set(self.audit["sem_resumo_editorial"])),
                    "parser_warnings": self.inv.warnings,
                    "nota": "Referência direta não é cobertura de execução; ausência não implica defeito."}
        self.pages["../static/api/inventario.json"] = json.dumps(inventory, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
        self.pages["../static/api/cobertura.json"] = json.dumps(coverage, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
        output.mkdir(parents=True, exist_ok=True)
        (output / ".documentacao-gerada").write_text("Não edite: saída de tools/gerar_documentacao.py\n", encoding="utf-8")
        for relative, content in sorted(self.pages.items()):
            path = (output / relative).resolve()
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")
        return {"pages": len(self.pages), "symbols": len(self.inv.symbols), "cases": len(self.inv.cases),
                "references": len(self.inv.references), "warnings": self.inv.warnings,
                "sem_referencia_direta": len(no_refs), "sem_resumo_editorial": len(coverage["sem_resumo_editorial"])}
