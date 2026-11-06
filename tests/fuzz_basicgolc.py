#!/usr/bin/env python3
"""fuzz_basicgolc.py — teste de robustez do transpilador M1 (Fase 6).

Gera mutações DETERMINÍSTICAS dos fontes do repositório e verifica que o
transpilador nunca trava, nunca aborta e sempre termina dentro do tempo
limite. Não julga a saída: o contrato verificado aqui é só

    basicgolc --checar <arquivo mutado>   ->  termina com 0 ou 1, sem sinal

porque um fonte mutado pode ser inválido (erro de estrutura, saída 1) mas não
pode derrubar a ferramenta nem deixá-la em laço infinito.

Uso:
    python3 tests/fuzz_basicgolc.py                     # 300 mutações, semente fixa
    python3 tests/fuzz_basicgolc.py --iteracoes 2000
    python3 tests/fuzz_basicgolc.py --basicgolc build/basicgolc
    python3 tests/fuzz_basicgolc.py --seed 7 --verboso

Sai com 0 se nenhuma mutação derrubou o transpilador; 1 caso contrário; 2 se o
transpilador não for encontrado.
"""

from __future__ import annotations

import argparse
import random
import subprocess
import sys
import tempfile
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent

# Entradas que estressam o léxico: chaves, literais, comentários, diretivas,
# acentos e tokens do M1 (PARA i DE … / ATE …;).
FRAGMENTOS = [
    b'"', b"'", b"\\", b"/*", b"*/", b"//", b"\n", b"\r\n", b"\r", b"\t", b"\0",
    b"#define X", b"#include <basicgol.h>\n", b"SE", b"ENTAO", b"FIM_SE",
    b"PARA i DE 1 ATE 10 FACA", b"FIM_PARA", b"ATE", b";", b"REPITA",
    b"ESCOLHA", b"CASO", b"CONTRARIO", b"PARE", b"PROSSIGA",
    "ATÉ".encode(), "ENTÃO".encode(), "SE".encode(), "FIM".encode(),
    b"{", b"}", b"(", b")", b"[", b"]", b",", b"0", b"2147483648",
    b'"texto com \\" aspas"', b"'\\0'", b"MOD", b"DIVIDE", b"LEIA", b"LEIA_TEXTO",
]


def fontes() -> list[Path]:
    """Corpus: os fontes reais do repositório (casos + exemplos)."""
    achados = sorted(RAIZ.glob("tests/casos/**/programa.c"))
    achados += sorted(RAIZ.glob("tests/casos/**/*.c"))
    achados += sorted(RAIZ.glob("docs/exemplos/*.c"))
    vistos: dict[Path, None] = {}
    for caminho in achados:
        vistos.setdefault(caminho, None)
    return list(vistos)


def muta(rng: random.Random, base: bytes) -> tuple[bytes, str]:
    """Aplica uma mutação e devolve (bytes, nome da mutação)."""
    escolha = rng.randrange(9)

    if escolha == 0:  # truncar
        corte = rng.randrange(0, len(base) + 1)
        return base[:corte], "truncar"

    if escolha == 1:  # apagar um intervalo
        if not base:
            return base, "apagar(no-op)"
        ini = rng.randrange(0, len(base))
        fim = min(len(base), ini + rng.randrange(2, 64))
        return base[:ini] + base[fim:], "apagar"

    if escolha == 2:  # inserir fragmento
        pos = rng.randrange(0, len(base) + 1)
        frag = rng.choice(FRAGMENTOS)
        return base[:pos] + frag + base[pos:], "inserir"

    if escolha == 3:  # trocar um byte
        if not base:
            return base, "trocar(no-op)"
        pos = rng.randrange(0, len(base))
        return base[:pos] + bytes([rng.randrange(0, 256)]) + base[pos + 1:], "trocar"

    if escolha == 4:  # duplicar uma linha (aninhamento/quebra de bloco)
        linhas = base.split(b"\n")
        if not linhas:
            return base, "duplicar(no-op)"
        i = rng.randrange(0, len(linhas))
        linhas.insert(i, linhas[i])
        return b"\n".join(linhas), "duplicar"

    if escolha == 5:  # CRLF
        return base.replace(b"\n", b"\r\n"), "crlf"

    if escolha == 6:  # BOM + CRLF
        return b"\xef\xbb\xbf" + base.replace(b"\n", b"\r\n"), "bom+crlf"

    if escolha == 7:  # remover o último fechamento de bloco
        for fecha in (b"FIM\n", b"FIM_SE", b"FIM_PARA", b"FIM_ENQUANTO"):
            if fecha in base:
                return base.replace(fecha, b"", 1), "sem-fechamento"
        return base, "sem-fechamento(no-op)"

    # escolha == 8: muitas chaves de abertura (estouro de pilha)
    return base + b"\n" + b"SE 1 ENTAO\n" * 300, "aninhamento-300"


def roda(basicgolc: Path, dados: bytes, tmp: Path, limite: float = 10.0) -> tuple[int, str]:
    """Executa uma mutação sob timeout e classifica crash, sinal ou sanitizer."""
    alvo = tmp / "mutado.c"
    alvo.write_bytes(dados)
    try:
        proc = subprocess.run(
            [str(basicgolc), "--checar", str(alvo)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            timeout=limite,
            env={"ASAN_OPTIONS": "detect_leaks=0", "LC_ALL": "C",
                 "PATH": "/usr/bin:/bin"},
        )
    except subprocess.TimeoutExpired:
        return 124, "tempo limite"
    err = proc.stderr.decode("utf-8", "replace")
    if proc.returncode < 0:
        return proc.returncode, f"sinal {-proc.returncode}"
    if "Sanitizer" in err or "runtime error" in err:
        return 99, err.strip().splitlines()[0][:200] if err.strip() else "sanitizer"
    return proc.returncode, err[:300]


def main() -> int:
    """CLI do fuzz determinístico: muta o corpus, preserva falhas e resume resultados."""
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--basicgolc", default=str(RAIZ / "build" / "basicgolc"))
    ap.add_argument("--iteracoes", type=int, default=300)
    ap.add_argument("--seed", type=int, default=20261106)
    ap.add_argument("--verboso", action="store_true")
    args = ap.parse_args()

    basicgolc = Path(args.basicgolc)
    if not (basicgolc.is_file() and basicgolc.stat().st_mode & 0o111):
        print(f"fuzz_basicgolc: transpilador nao encontrado em {basicgolc}", file=sys.stderr)
        print("                 rode  make  antes.", file=sys.stderr)
        return 2

    rng = random.Random(args.seed)
    corpus = fontes()
    if not corpus:
        print("fuzz_basicgolc: corpus vazio", file=sys.stderr)
        return 2

    falhas = 0
    ok = 0
    por_mutacao: dict[str, int] = {}

    with tempfile.TemporaryDirectory(prefix="basicgol-fuzz.") as tmpdir:
        tmp = Path(tmpdir)
        for n in range(args.iteracoes):
            base = rng.choice(corpus).read_bytes()
            dados, nome = muta(rng, base)
            por_mutacao[nome] = por_mutacao.get(nome, 0) + 1
            rc, detalhe = roda(basicgolc, dados, tmp)
            if rc not in (0, 1):
                falhas += 1
                print(f"  FALHA iteracao {n} ({nome}): rc={rc} {detalhe}")
                (tmp / f"falha_{n}.c").write_bytes(dados)
                destino = RAIZ / "build" / f"fuzz-falha-{n}.c"
                destino.parent.mkdir(exist_ok=True)
                destino.write_bytes(dados)
                print(f"        entrada preservada em {destino.relative_to(RAIZ)}")
            else:
                ok += 1
                if args.verboso:
                    print(f"  ok   {n} ({nome}) rc={rc}")

    print(f"\nfuzz_basicgolc: {ok} mutacoes sem crash, {falhas} falha(s) "
          f"[seed={args.seed}, iteracoes={args.iteracoes}]")
    print("  por mutacao: " + ", ".join(f"{k}={v}" for k, v in sorted(por_mutacao.items())))
    return 0 if falhas == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
