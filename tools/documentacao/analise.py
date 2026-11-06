"""Inventário estático, determinístico e independente do compilador C.

Tree-sitter lê C sem executar o pré-processador: isso preserva os ramos de
compiladores diferentes e permite ler casos negativos deliberadamente inválidos.
Python usa ast; runners Bash têm suas funções e heredocs C incorporados indexados.
Referências são evidências de uso no código, NÃO uma medida de cobertura dinâmica.
"""
from __future__ import annotations

import ast
import hashlib
import re
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterator

from tree_sitter import Language, Node, Parser
import tree_sitter_c

IDENT = re.compile(r"[A-Za-z_][A-Za-z_0-9]*")
C_SUFFIXES = {".c", ".h", ".cpp", ".cc", ".cxx"}
CODE_SUFFIXES = C_SUFFIXES | {".py", ".sh"}
SKIP_DIRS = {".git", ".venv", "node_modules", "build", "website", "__pycache__"}


@dataclass
class Source:
    path: str
    text: str
    language: str
    tree: object = None
    parse_errors: int = 0
    parent: str | None = None
    offset: int = 0
    data: bytes = field(init=False, repr=False)

    def __post_init__(self) -> None:
        # Tree-sitter usa bytes UTF-8, não índices de caracteres Unicode.
        """Codifica a fonte em bytes UTF-8 para offsets corretos do Tree-sitter."""
        self.data = self.text.encode("utf-8")

    @property
    def lines(self) -> list[str]:
        """Expõe a fonte como linhas para snippets e links."""
        return self.text.splitlines()

    @property
    def digest(self) -> str:
        """Calcula hash SHA-256 determinístico do conteúdo da fonte."""
        return hashlib.sha256(self.text.encode()).hexdigest()


@dataclass
class Symbol:
    name: str
    kind: str
    language: str
    path: str
    line: int
    end: int
    signature: str
    body: str
    module: str
    comment: str = ""
    conditions: list[dict] = field(default_factory=list)
    calls: list[str] = field(default_factory=list)
    parameters: list[str] = field(default_factory=list)
    prototypes: list[dict] = field(default_factory=list)
    aliases: list[str] = field(default_factory=list)
    visibility: str = "pública"

    @property
    def key(self) -> str:
        """Identifica símbolo por tipo/nome ou por arquivo para evitar colisões internas."""
        if self.kind in {"função C", "macro"}:
            return f"{self.kind}:{self.name}"
        return f"{self.kind}:{self.path}:{self.name}"

    @property
    def slug(self) -> str:
        """Gera rota estável por tipo/módulo, reunindo apelidos de macro."""
        name = re.sub(r"[^a-zA-Z0-9_-]+", "-", self.name).strip("-").lower()
        if self.kind == "macro":
            return f"referencia/macros/{name}"
        if self.kind == "função C":
            return f"referencia/funcoes/{self.module}/{name}"
        area = {"python": "python", "shell": "shell"}.get(self.language, "c")
        return f"internos/{area}/{Path(self.path).stem}/{name.replace('.', '-')}"


@dataclass
class Reference:
    symbol: str
    path: str
    line: int
    container: str
    kind: str = "código"
    parent: str | None = None
    offset: int = 0


@dataclass
class Inventory:
    root: Path
    sources: dict[str, Source] = field(default_factory=dict)
    symbols: dict[str, Symbol] = field(default_factory=dict)
    references: list[Reference] = field(default_factory=list)
    cases: list[dict] = field(default_factory=list)
    modes: list[dict] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)
    make_targets: list[dict] = field(default_factory=list)
    examples: list[dict] = field(default_factory=list)
    by_name: dict[str, list[Symbol]] = field(default_factory=dict)

    def finish(self) -> None:
        """Reconstrói índices por nome, resolve apelidos canônicos e ordena saídas."""
        self.by_name = defaultdict(list)
        for symbol in self.symbols.values():
            self.by_name[symbol.name].append(symbol)
        for symbol in self.symbols.values():
            if symbol.kind != "macro" or not symbol.path.startswith("include/") or symbol.parameters or not symbol.body.strip():
                continue
            target = [s for s in self.by_name.get(symbol.body.strip(), [])
                      if s.kind == "macro" and s.path.startswith("include/")]
            if len(target) == 1:
                target[0].aliases = sorted(set(target[0].aliases) | {symbol.name})
                symbol.aliases = sorted(set(symbol.aliases) | {target[0].name})
        for symbol in self.symbols.values():
            symbol.aliases = sorted(set(symbol.aliases) - {symbol.name})
        self.warnings.sort()
        self.references.sort(key=lambda r: (r.path, r.line, r.symbol, r.kind))


def walk(node: Node) -> Iterator[Node]:
    """Percorre uma árvore Tree-sitter em profundidade, incluindo o nó raiz."""
    yield node
    for child in node.named_children:
        yield from walk(child)


def segment(source: Source, node: Node) -> str:
    """Extrai texto de um nó usando offsets de bytes UTF-8 do Tree-sitter."""
    return source.data[node.start_byte:node.end_byte].decode("utf-8")


def ancestors(node: Node) -> Iterator[Node]:
    """Percorre os pais de um nó até a raiz da árvore."""
    current = node.parent
    while current is not None:
        yield current
        current = current.parent


def previous_comment(source: Source, node: Node) -> str:
    """Reúne comentários imediatamente anteriores a uma definição."""
    comments = []
    sibling = node.prev_named_sibling
    while sibling is not None and sibling.type == "comment":
        comments.append(segment(source, sibling))
        sibling = sibling.prev_named_sibling
    return "\n".join(reversed(comments))


def clean_comment(raw: str) -> str:
    """Remove delimitadores e marcadores de comentário sem alterar o conteúdo."""
    lines = []
    for line in raw.splitlines():
        line = line.strip()
        if line.startswith(("/*", "//", "# ")):
            line = line[2:].strip()
        if line.endswith("*/"):
            line = line[:-2].strip()
        if line.startswith("*"):
            line = line[1:].strip()
        if line and not re.fullmatch(r"[-= ]+", line):
            lines.append(line)
    return "\n".join(lines)


def conditions(source: Source, node: Node) -> list[dict]:
    """Extrai condições de pré-processador ancestrais, incluindo negação de else."""
    result = []
    for parent in ancestors(node):
        if parent.type not in {"preproc_if", "preproc_ifdef", "preproc_elif", "preproc_else"}:
            continue
        if parent.type in {"preproc_else", "preproc_elif"}:
            previous = parent.prev_named_sibling
            predicates = []
            while previous is not None:
                if previous.type in {"preproc_if", "preproc_ifdef"}:
                    cond = previous.child_by_field_name("condition") or previous.child_by_field_name("name")
                    if cond is not None:
                        predicates.append(segment(source, cond))
                elif previous.type == "preproc_elif":
                    cond = previous.child_by_field_name("condition")
                    if cond is not None:
                        predicates.append(segment(source, cond))
                previous = previous.prev_named_sibling
            own = parent.child_by_field_name("condition")
            if own is not None:
                predicates.append(segment(source, own))
            condition = ("NOT (" + " OR ".join(predicates) + ")") if predicates else "ramo alternativo"
            if own is not None:
                condition += " AND " + segment(source, own)
        else:
            child = parent.child_by_field_name("condition") or parent.child_by_field_name("name")
            condition = segment(source, child) if child is not None else segment(source, parent).splitlines()[0]
            if parent.type == "preproc_ifdef":
                condition = f"defined({condition})"
        result.append({"condition": condition, "line": parent.start_point.row + 1 + source.offset,
                       "kind": parent.type})
    return list(reversed(result))


def declarator_name(source: Source, node: Node | None) -> str:
    """Encontra o identificador em declaradores C aninhados/ponteiro."""
    if node is None:
        return ""
    if node.type in {"identifier", "field_identifier"}:
        return segment(source, node)
    if node.type == "pointer_declarator" and "*" in segment(source, node):
        # Preserva o '*' da declaração sem perder o nome do identificador.
        pass
    for field_name in ("declarator", "function_declarator"):
        child = node.child_by_field_name(field_name)
        if child is not None:
            return declarator_name(source, child)
    for child in node.named_children:
        name = declarator_name(source, child)
        if name:
            return name
    return ""


def called_names(source: Source, node: Node) -> list[str]:
    """Lista chamadas diretas por identificador dentro de um nó C."""
    result = []
    for item in walk(node):
        if item.type == "call_expression":
            function = item.child_by_field_name("function")
            if function is not None and function.type == "identifier":
                result.append(segment(source, function))
    return sorted(set(result))


def module_for(path: str) -> str:
    """Deriva o módulo de documentação a partir do caminho do fonte."""
    p = Path(path)
    if p.parts[0] == "tools":
        return "transpilador"
    if p.parts[0] == "tests":
        return "testes"
    return p.stem


def mask_code(text: str, language: str) -> str:
    """Substitui comentários e literais por espaços, preservando linhas/colunas.

    Não interpreta o pré-processador. Strings não são tratadas como chamadas;
    nomes de modos de teste são extraídos separadamente pela AST C.
    """
    out = list(text)
    i = 0
    n = len(text)
    line_comment = "//" if language in {"c", "cpp"} else "#"
    block = language in {"c", "cpp"}
    while i < n:
        if block and text.startswith("/*", i):
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            for j in range(i, end):
                if out[j] != "\n":
                    out[j] = " "
            i = end
            continue
        if text.startswith(line_comment, i):
            end = text.find("\n", i)
            end = n if end < 0 else end
            out[i:end] = " " * (end - i)
            i = end
            continue
        if language == "python" and text[i:i+3] in {"'''", '"""'}:
            quote = text[i:i+3]
            end = text.find(quote, i + 3)
            end = n if end < 0 else end + 3
            for j in range(i, end):
                if out[j] != "\n":
                    out[j] = " "
            i = end
            continue
        if text[i] in {"'", '"', "`"}:
            quote = text[i]
            j = i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == quote or (language == "shell" and text[j] == "\n" and quote != "`"):
                    break
                j += 1
            end = min(n, j + 1)
            for k in range(i, end):
                if out[k] != "\n":
                    out[k] = " "
            i = end
            continue
        i += 1
    return "".join(out)


def identifier_spans(source: Source) -> list[tuple[str, int]]:
    """Lista identificadores e linhas, incluindo corpos de macros C."""
    if source.language in {"c", "cpp"} and source.tree is not None:
        spans = []
        for node in walk(source.tree.root_node):
            if node.type == "identifier":
                spans.append((segment(source, node), node.start_point.row + 1))
            elif node.type == "preproc_arg":
                line = node.start_point.row + 1
                for match in IDENT.finditer(mask_code(segment(source, node), "c")):
                    spans.append((match.group(), line))
        return spans
    return [(m.group(), source.text.count("\n", 0, m.start()) + 1)
            for m in IDENT.finditer(mask_code(source.text, source.language))]


def add_symbol(inv: Inventory, symbol: Symbol) -> None:
    """Insere um símbolo ou mescla protótipo/definição sem duplicar a página."""
    existing = inv.symbols.get(symbol.key)
    if existing is None:
        inv.symbols[symbol.key] = symbol
        return
    if symbol.body:
        existing.body = symbol.body
        existing.line = symbol.line
        existing.end = symbol.end
        existing.signature = symbol.signature
        existing.calls = symbol.calls
        existing.conditions = symbol.conditions
        existing.path = symbol.path
    if symbol.comment and symbol.comment not in existing.comment:
        existing.comment = (existing.comment + "\n" + symbol.comment).strip()
    if symbol.prototypes:
        existing.prototypes.extend(symbol.prototypes)


def extract_c(inv: Inventory, source: Source, production: bool) -> None:
    """Extrai funções, protótipos e macros de uma árvore C sem pré-processar alvos."""
    assert source.tree is not None
    for node in walk(source.tree.root_node):
        if node.type in {"function_definition", "declaration"}:
            # Fixtures BG não são C puro: sem expandir macros, o parser pode
            # interpretar uma chamada como definição. Só o harness unitário é
            # inventariado como função C dentro de tests/.
            if source.path.startswith("tests/") and source.path != "tests/test_biblioteca.c":
                continue
            declarator = node.child_by_field_name("declarator")
            function = declarator
            while function is not None and function.type not in {"function_declarator", "identifier"}:
                function = function.child_by_field_name("declarator")
            if function is None or function.type != "function_declarator":
                continue
            name = declarator_name(source, function.child_by_field_name("declarator"))
            if not name:
                continue
            body = node.child_by_field_name("body")
            signature = source.data[node.start_byte:body.start_byte].decode("utf-8").strip() if body else segment(source, node)
            params = function.child_by_field_name("parameters")
            parameters = [segment(source, child) for child in params.named_children
                          if child.type == "parameter_declaration"] if params else []
            storage = [segment(source, child) for child in node.named_children
                       if child.type == "storage_class_specifier"]
            symbol = Symbol(name, "função C", "c", source.path,
                            node.start_point.row + 1, node.end_point.row + 1,
                            signature, segment(source, body) if body else "", module_for(source.path),
                            clean_comment(previous_comment(source, node)), conditions(source, node),
                            called_names(source, node), parameters,
                            visibility="interna (static)" if "static" in storage else "pública")
            if body is None:
                # Chamadas/declarações BG em fixtures não são definições de API.
                if source.path.startswith("tests/"):
                    continue
                symbol.prototypes = [{"path": source.path, "line": symbol.line,
                                      "signature": signature, "comment": symbol.comment,
                                      "conditions": symbol.conditions}]
                symbol.comment = ""
            add_symbol(inv, symbol)
        elif node.type in {"preproc_def", "preproc_function_def"}:
            name_node = node.child_by_field_name("name")
            value = node.child_by_field_name("value")
            params = node.child_by_field_name("parameters")
            name = segment(source, name_node)
            parameters = [segment(source, child) for child in params.named_children
                          if child.type == "identifier"] if params else []
            body = segment(source, value) if value else ""
            symbol = Symbol(name, "macro", "c", source.path, node.start_point.row + 1,
                            node.end_point.row, segment(source, node).rstrip(), body,
                            module_for(source.path), clean_comment(previous_comment(source, node)),
                            conditions(source, node),
                            sorted(set(IDENT.findall(mask_code(body, "c")))), parameters)
            add_symbol(inv, symbol)
    if production and source.tree.root_node.has_error:
        unexpected = [n for n in walk(source.tree.root_node)
                      if n.type == "ERROR" and 'extern "C"' not in segment(source, n)
                      and "__attribute__" not in segment(source, n)
                      and not (segment(source, n).startswith("#") and "BG_FORMATO" in source.text)]
        if unexpected:
            inv.warnings.append(f"Tree-sitter encontrou sintaxe não reconhecida em {source.path}; revisar extração.")


def extract_python(inv: Inventory, source: Source) -> None:
    """Extrai funções/métodos Python com ast, docstrings e chamadas."""
    try:
        tree = ast.parse(source.text, filename=source.path)
    except SyntaxError as exc:
        inv.warnings.append(f"Python inválido em {source.path}: {exc}")
        return
    lines = source.lines

    def visit(node: ast.AST, prefix: str = "") -> None:
        """Visita recursivamente definições Python preservando qualificação de classe/função."""
        for child in ast.iter_child_nodes(node):
            if isinstance(child, (ast.FunctionDef, ast.AsyncFunctionDef)):
                name = prefix + child.name
                end = child.end_lineno or child.lineno
                body = "\n".join(lines[child.lineno - 1:end])
                args = [ast.get_source_segment(source.text, a) or a.arg for a in child.args.posonlyargs + child.args.args]
                if child.args.vararg:
                    args.append("*" + child.args.vararg.arg)
                args += [ast.get_source_segment(source.text, a) or a.arg for a in child.args.kwonlyargs]
                if child.args.kwarg:
                    args.append("**" + child.args.kwarg.arg)
                signature = body.split(":", 1)[0] + ":"
                calls = sorted({n.func.id for n in ast.walk(child)
                                if isinstance(n, ast.Call) and isinstance(n.func, ast.Name)} |
                               {n.func.attr for n in ast.walk(child)
                                if isinstance(n, ast.Call) and isinstance(n.func, ast.Attribute)})
                add_symbol(inv, Symbol(name, "função Python", "python", source.path,
                                       child.lineno, end, signature, body, Path(source.path).stem,
                                       ast.get_docstring(child) or "", [], calls, args,
                                       visibility="interna" if child.name.startswith("_") else "pública"))
                visit(child, name + ".")
            elif isinstance(child, ast.ClassDef):
                visit(child, prefix + child.name + ".")
            else:
                visit(child, prefix)
    visit(tree)


def extract_shell(inv: Inventory, source: Source) -> None:
    """Extrai funções Bash por nome, corpo balanceado e comentário anterior."""
    masked = mask_code(source.text, "shell")
    starts = list(re.finditer(r"^([A-Za-z_][A-Za-z_0-9]*)\s*\(\)\s*\{", masked, re.MULTILINE))
    for match in starts:
        start = match.end() - 1
        depth = 0
        end = len(masked)
        for index in range(start, len(masked)):
            if masked[index] == "{":
                depth += 1
            elif masked[index] == "}":
                depth -= 1
                if depth == 0:
                    end = index + 1
                    break
        line = source.text.count("\n", 0, match.start()) + 1
        body = source.text[match.start():end]
        comment = ""
        prior = line - 2
        comments = []
        while prior >= 0 and source.lines[prior].lstrip().startswith("#"):
            comments.append(source.lines[prior])
            prior -= 1
        comment = clean_comment("\n".join(reversed(comments)))
        add_symbol(inv, Symbol(match.group(1), "função Shell", "shell", source.path,
                               line, source.text.count("\n", 0, end) + 1,
                               match.group(1) + "()", body, Path(source.path).stem,
                               comment, [], sorted(set(IDENT.findall(mask_code(body, "shell"))))))


def extract_heredocs(inv: Inventory, source: Source, parser: Parser) -> None:
    """Indexa heredocs de runners como fontes virtuais com linhas originais."""
    lines = source.lines
    i = 0
    while i < len(lines):
        match = re.search(r"<<-?\s*['\"]?([A-Za-z_][A-Za-z_0-9]*)['\"]?", lines[i])
        if not match or lines[i].lstrip().startswith("#"):
            i += 1
            continue
        delimiter = match.group(1)
        start = i + 1
        end = start
    # O fechamento pode ser indentado com <<-; o conteúdo não passa por expansão
    # quando o delimitador está entre aspas. Variáveis são preservadas como texto.
        while end < len(lines) and lines[end].lstrip() != delimiter:
            end += 1
        if end < len(lines) and end > start:
            content = "\n".join(lines[start:end]) + "\n"
            virtual = f"{source.path}::heredoc@{start + 1}"
            language = "c" if re.search(r"\.(c|cpp|cc)\b", lines[i]) else "texto"
            child = Source(virtual, content, language, parent=source.path, offset=start)
            if language == "c":
                child.tree = parser.parse(content.encode())
            inv.sources[virtual] = child
        i = end + 1


def string_value(source: Source, node: Node) -> str:
    """Remove aspas de um literal string simples extraído da AST C."""
    raw = segment(source, node)
    return raw[1:-1] if len(raw) >= 2 else raw


def extract_modes(inv: Inventory, source: Source) -> None:
    """Liga strcmp(caso, 'modo') e strcmp(argv[1], '--modo') ao ramo C real."""
    assert source.tree is not None
    for node in walk(source.tree.root_node):
        if node.type != "call_expression":
            continue
        name = node.child_by_field_name("function")
        args = node.child_by_field_name("arguments")
        if name is None or segment(source, name) != "strcmp" or args is None:
            continue
        values = args.named_children
        if len(values) != 2 or values[1].type != "string_literal":
            continue
        first = segment(source, values[0])
        if first not in {"caso", "argv[1]"}:
            continue
        branch = next((a for a in ancestors(node) if a.type == "if_statement"), None)
        if branch is None:
            continue
        body = branch.child_by_field_name("consequence")
        if body is None:
            continue
        mode = string_value(source, values[1])
        inv.modes.append({"mode": mode, "path": source.path,
                          "line": body.start_point.row + 1, "end": body.end_point.row + 1,
                          "body": segment(source, body), "calls": called_names(source, body),
                          "conditions": conditions(source, body)})


def container_for(source: Source, line: int, inv: Inventory) -> str:
    """Resolve o contexto mais específico de uma referência de teste."""
    if source.parent:
        return f"C incorporado no runner (linha {source.offset + 1})"
    candidates = [s for s in inv.symbols.values() if s.path == source.path and s.body
                  and s.line <= line <= s.end]
    if candidates:
        return min(candidates, key=lambda s: s.end - s.line).name
    if source.path.startswith("tests/casos/"):
        return Path(source.path).parent.name
    return Path(source.path).name


def discover_cases(inv: Inventory) -> None:
    """Descobre suítes/casos em disco, arquivos esperados, marcadores e registro no runner."""
    cases_root = inv.root / "tests" / "casos"
    if not cases_root.exists():
        return
    for suite in sorted(p for p in cases_root.iterdir() if p.is_dir()):
        for directory in sorted(p for p in suite.iterdir() if p.is_dir()):
            programs = sorted(p for p in directory.iterdir()
                              if p.suffix in C_SUFFIXES and p.name.startswith("programa"))
            if not programs:
                #.Multiarquivo: principal.c + módulos; aplicação: todos os .c.
                programs = sorted(p for p in directory.glob("*.c"))
            if not programs:
                inv.warnings.append(f"Caso sem programa reconhecido: {directory.relative_to(inv.root)}")
                continue
            negative = "negativ" in suite.name
            warning = "avisos" in suite.name
            m1_only = (directory / "sem_m0").exists() or "m1" in suite.name
            item = {"suite": suite.name, "name": directory.name,
                    "path": directory.relative_to(inv.root).as_posix(),
                    "programs": [p.relative_to(inv.root).as_posix() for p in programs],
                    "files": {p.name: p.relative_to(inv.root).as_posix()
                              for p in sorted(directory.iterdir()) if p.is_file()},
                    "mode": "negativo" if negative else "aviso" if warning else "execução",
                    "m0": not (negative or warning or m1_only),
                    "m1": not ("06_negativos" in suite.name or (directory / "sem_m1").exists()),
                    "objetos": (directory / "objetos").exists(),
                    "runner_registered": suite.name in (inv.root / "tests" / "run.sh").read_text(encoding="utf-8"),
                    "markers": sorted(p.name for p in directory.iterdir() if p.is_file() and p.suffix == ""),
                    "symbols": []}
            inv.cases.append(item)


def discover_make(inv: Inventory) -> None:
    """Extrai targets, dependências e linhas do Makefile."""
    path = inv.root / "Makefile"
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8")
    for match in re.finditer(r"^([a-zA-Z][a-zA-Z0-9_-]*):([^=\n]*)\n((?:\t.*\n?)*)", text, re.M):
        inv.make_targets.append({"name": match.group(1), "dependencies": match.group(2).strip(),
                                 "recipe": match.group(3).rstrip(), "line": text.count("\n", 0, match.start()) + 1})


def collect(root: Path) -> Inventory:
    """Coleta fontes, símbolos, referências, casos, modos e exemplos do repositório."""
    root = root.resolve()
    inv = Inventory(root)
    parser = Parser(Language(tree_sitter_c.language()))
    paths = []
    for top in ("include", "src", "tools", "tests"):
        base = root / top
        if base.exists():
            paths.extend(p for p in base.rglob("*") if p.is_file()
                         and not any(part in SKIP_DIRS or part.startswith("build-") for part in p.relative_to(root).parts))
    for path in sorted(set(paths)):
        relative = path.relative_to(root).as_posix()
        if path.suffix not in CODE_SUFFIXES:
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            inv.warnings.append(f"Arquivo de código não UTF-8 ignorado: {relative}")
            continue
        language = {".py": "python", ".sh": "shell", ".cpp": "cpp", ".cc": "cpp"}.get(path.suffix, "c")
        source = Source(relative, text, language)
        if language in {"c", "cpp"}:
            # O atributo de diagnóstico é um protocolo do projeto, não sintaxe C.
            # Só a cópia analisada omite essa anotação; posições e fonte permanecem.
            parse_text = re.sub(r"(BG_FORMATO\([^)]*\)|BG_NORETURN(?=\s+void))",
                                lambda m: " " * len(m.group()), text)
            source.tree = parser.parse(parse_text.encode("utf-8"))
        inv.sources[relative] = source
        production = relative.startswith(("include/", "src/", "tools/"))
        if language in {"c", "cpp"}:
            extract_c(inv, source, production)
            if relative == "tests/test_biblioteca.c":
                extract_modes(inv, source)
        elif language == "python":
            extract_python(inv, source)
        elif language == "shell":
            extract_shell(inv, source)
            extract_heredocs(inv, source, parser)
    # Funções primeiro: a resolução de contexto não deve escolher o caso inteiro
    # quando existe uma função de teste mais específica.
    inv.finish()
    names = set(inv.by_name)
    for source in list(inv.sources.values()):
        if not source.path.startswith("tests/"):
            continue
        spans = identifier_spans(source)
        if source.language == "shell" and not source.parent:
            # Heredocs já têm fonte e linhas próprias; não contar duas vezes.
            masked = mask_code(source.text, "shell")
            for match in re.finditer(r"<<-?\s*['\"]?([A-Za-z_][A-Za-z_0-9]*)['\"]?", masked):
                line_start = masked.rfind("\n", 0, match.start()) + 1
                if masked[line_start:match.start()].lstrip().startswith("#"):
                    continue
                delimiter = match.group(1)
                end = masked.find("\n" + delimiter, match.end())
                if end >= 0:
                    stop = masked.find("\n", end + 1)
                    stop = len(masked) if stop < 0 else stop
                    masked = masked[:match.end()] + " " * (stop - match.end()) + masked[stop:]
            spans = [(m.group(), masked.count("\n", 0, m.start()) + 1)
                     for m in IDENT.finditer(masked)]
        for name, line in spans:
            if name not in names:
                continue
            if not source.parent:
                definition = next((s for s in inv.by_name[name] if s.path == source.path
                                   and line == s.line), None)
                if definition:
                    continue
            kind = "condicional" if source.language in {"c", "cpp"} and re.match(
                r"\s*#\s*(if|ifdef|ifndef|elif|else|endif)", source.lines[line - 1]) else "código"
            inv.references.append(Reference(name, source.path, line + source.offset,
                                            container_for(source, line, inv), kind,
                                            source.parent, source.offset if source.parent else 0))
    expansions = root / "tests" / "expansoes.txt"
    if expansions.exists():
        for line, text in enumerate(expansions.read_text(encoding="utf-8").splitlines(), 1):
            match = re.match(r"([A-Za-z_]\w*)\|", text)
            if match and match.group(1) in names:
                inv.references.append(Reference(match.group(1), "tests/expansoes.txt", line,
                                                "expansão esperada após cc -E -dM", "tabela de expansão"))
    example_dir = root / "docs" / "exemplos"
    if example_dir.exists():
        for path in sorted(example_dir.glob("*.c")):
            relative = path.relative_to(root).as_posix()
            text = path.read_text(encoding="utf-8")
            source = Source(relative, text, "c")
            source.tree = parser.parse(source.data)
            inv.sources[relative] = source
            names = sorted({name for name, _ in identifier_spans(source) if name in names})
            inv.examples.append({"name": path.stem, "path": relative, "symbols": names,
                                 "files": {p.name: p.relative_to(root).as_posix()
                                           for p in sorted(example_dir.glob(path.stem + ".*"))}})
    discover_cases(inv)
    for case in inv.cases:
        case["symbols"] = sorted({r.symbol for r in inv.references if r.path.startswith(case["path"] + "/")})
    discover_make(inv)
    inv.finish()
    return inv
