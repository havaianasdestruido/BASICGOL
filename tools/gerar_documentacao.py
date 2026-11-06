#!/usr/bin/env python3
"""Gera ou verifica a documentação Docusaurus do BASICGOL.

Uso:
    python3 tools/gerar_documentacao.py
    python3 tools/gerar_documentacao.py --check
    python3 tools/gerar_documentacao.py --strict

Não compila nem executa C: a extração é estática e preserva ramos de compilador.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from tools.documentacao.analise import collect
from tools.documentacao.render import Renderer


def current_commit(root: Path) -> str:
    """Resolve o permalink do fonte sem exigir rede ou autenticação."""
    explicit = os.environ.get("DOCS_COMMIT") or os.environ.get("GITHUB_SHA")
    if explicit:
        return explicit
    try:
        return subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"],
                                       text=True, stderr=subprocess.DEVNULL).strip()
    except (OSError, subprocess.CalledProcessError):
        return "main"


def tree_digest(directory: Path) -> dict[str, str]:
    """Indexa arquivos por caminho e hash para comparação determinística."""
    if not directory.exists():
        return {}
    return {p.relative_to(directory).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(directory.rglob("*")) if p.is_file()}


def check_outputs(expected: Path, docs: Path, static_api: Path) -> list[str]:
    """Compara a saída recém-gerada com os artefatos locais, sem modificá-los."""
    problems = []
    for want, actual in ((expected / "docs", docs), (expected / "static" / "api", static_api)):
        a, b = tree_digest(want), tree_digest(actual)
        for path in sorted(set(a) | set(b)):
            if path not in a:
                problems.append(f"obsoleto: {actual / path}")
            elif path not in b:
                problems.append(f"ausente: {actual / path}")
            elif a[path] != b[path]:
                problems.append(f"diferente: {actual / path}")
    return problems


def clean_managed_output(docs: Path, static_api: Path) -> None:
    """Remove apenas a árvore marcada como gerada e os dois JSON conhecidos."""
    if docs.exists():
        if not (docs / ".documentacao-gerada").exists():
            raise RuntimeError(f"Recusando apagar diretório não gerenciado: {docs}")
        shutil.rmtree(docs)
    for name in ("inventario.json", "cobertura.json"):
        path = static_api / name
        if path.exists():
            path.unlink()


def build(root: Path, output: Path, repo_url: str, commit: str) -> dict:
    """Extrai o inventário e escreve páginas MDX e dados JSON públicos."""
    inventory = collect(root)
    contracts = json.loads((root / "tools" / "documentacao" / "contratos.json").read_text(encoding="utf-8"))
    renderer = Renderer(inventory, contracts, repo_url, commit)
    return renderer.write(output)


def main(argv: list[str] | None = None) -> int:
    """CLI: gerar, verificar determinismo e opcionalmente falhar em avisos do parser."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--output", type=Path, help="Saída docs; padrão website/docs")
    parser.add_argument("--check", action="store_true", help="Não escreve; compara com a saída existente")
    parser.add_argument("--strict", action="store_true", help="Falha se o parser reportar sintaxe não reconhecida")
    parser.add_argument("--repo-url", default=os.environ.get("DOCS_REPOSITORY_URL", "https://github.com/havaianasdestruido/BASICGOL"))
    parser.add_argument("--commit", default=None)
    args = parser.parse_args(argv)
    root = args.root.resolve()
    output = (args.output or root / "website" / "docs").resolve()
    static_api = output.parent / "static" / "api"
    commit = args.commit or current_commit(root)
    try:
        if args.check:
            with tempfile.TemporaryDirectory(prefix="basicgol-docs-") as tmp:
                summary = build(root, Path(tmp) / "docs", args.repo_url, commit)
                problems = check_outputs(Path(tmp), output, static_api)
            if problems:
                print("Documentação desatualizada ou diferente:", file=sys.stderr)
                print("\n".join(problems[:40]), file=sys.stderr)
                return 1
        else:
            clean_managed_output(output, static_api)
            summary = build(root, output, args.repo_url, commit)
        print(json.dumps(summary, ensure_ascii=False, indent=2))
        return 1 if args.strict and summary["warnings"] else 0
    except (RuntimeError, ValueError, OSError) as exc:
        print(f"gerar_documentacao: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
