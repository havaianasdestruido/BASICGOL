#!/usr/bin/env python3
"""Audita todos os fontes de exemplos, inclusive os fora de docs/exemplos.

Exemplo: python3 tests/auditar_exemplos.py --cc clang --basicgolc build/basicgolc \
    --library build/libbasicgol.a
--object-only é para compiladores que não oferecem ligação, como 8cc. Para
cproc/8cc pode-se usar basicgolc e biblioteca previamente construídos com GCC;
o resultado de cada exemplo não implica que esses componentes foram construídos
pelo compilador sob teste. Os três exemplos de plataforma específica são apenas
sondados como código C no host, nunca executados como Windows/boot/Wasm.
"""
import argparse
import pathlib
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS = ROOT / 'docs/exemplos'
EXTRAS = ROOT / 'examples'
COPIAS = {
    'examples/beginner/01_ola_mundo.c': 'ola_mundo',
    'examples/beginner/02_tabuada.c': 'tabuada',
    'examples/intermediate/01_fatorial_fibonacci.c': 'fatorial_recursivo',
    'examples/advanced/01_cadastro.c': 'cadastro',
}
SPECIAL = {
    'examples/advanced/basicz-gui/basicz_gui.c': 'Windows GUI',
    'examples/advanced/bootloader/bootloader.c': 'boot freestanding',
    'examples/advanced/wasm/wasm_exemplo.c': 'WebAssembly/Emscripten',
}


def call(args, *, stdin=b'', timeout=20):
    try:
        return subprocess.run(args, input=stdin, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, cwd=ROOT, timeout=timeout)
    except subprocess.TimeoutExpired:
        return None


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cc', required=True, help='executável do compilador')
    p.add_argument('--basicgolc', required=True)
    p.add_argument('--library', required=True)
    p.add_argument('--flag', action='append', default=[], help='flag adicional; use --flag=-std=c11')
    p.add_argument('--object-only', action='store_true', help='compila -c sem ligar nem executar')
    p.add_argument('--linker', help='compila -c e liga com este driver (ex.: gcc para 8cc)')
    args = p.parse_args()
    if args.object_only and args.linker:
        p.error('--object-only e --linker sao mutuamente exclusivos')
    compiler = args.cc
    basicgolc = str(pathlib.Path(args.basicgolc).resolve())
    library = str(pathlib.Path(args.library).resolve())
    sources = sorted(DOCS.glob('*.c')) + sorted(EXTRAS.rglob('*.c'))
    passed = failed = probe_ok = probe_failed = 0
    with tempfile.TemporaryDirectory(prefix='bg-exemplos-') as temp:
        temp = pathlib.Path(temp)
        for source in sources:
            rel = source.relative_to(ROOT).as_posix()
            special = SPECIAL.get(rel)
            fixture = source.stem if source.parent == DOCS else COPIAS.get(rel)
            profiles = ['M0', 'M1']
            if source.parent == DOCS and source.stem in ('para_contado', 'controle_para_acucar'):
                profiles.remove('M0')
            for profile in profiles:
                generated = temp / 'generated.c'
                inp = source
                if profile == 'M1':
                    proc = call([basicgolc, str(source), '-o', str(generated)])
                    if proc is None or proc.returncode:
                        lines = [] if proc is None else proc.stderr.decode(errors='replace').strip().splitlines()
                        message = ('timeout' if proc is None else
                                   next((line for line in lines if ': erro:' in line), lines[0] if lines else 'sem diagnostico'))
                        status = 'SONDAGEM FALHOU' if special else 'FALHA'
                        print(f'{status}\t{rel}\t{profile}\ttranspilacao: {message}')
                        if special: probe_failed += 1
                        else: failed += 1
                        continue
                    inp = generated
                binary = temp / ('out.o' if args.object_only or args.linker or special else 'out')
                cmd = [compiler, *args.flag, '-I', str(ROOT / 'include')]
                if special == 'boot freestanding':
                    cmd.append('-ffreestanding')
                cmd += [str(inp)]
                if args.object_only or args.linker or special:
                    cmd += ['-c', '-o', str(binary)]
                else:
                    cmd += ['-o', str(binary), library, '-lm']
                proc = call(cmd)
                if proc is None or proc.returncode:
                    message = 'timeout' if proc is None else ' '.join(proc.stderr.decode(errors='replace').strip().splitlines()[:2])
                    status = 'SONDAGEM FALHOU' if special else 'FALHA'
                    print(f'{status}\t{rel}\t{profile}\tcompilacao: {message}')
                    if special: probe_failed += 1
                    else: failed += 1
                    continue
                if special:
                    print(f'SONDAGEM COMPILOU\t{rel}\t{profile}\tobjeto host; nao testou {special}')
                    probe_ok += 1
                    continue
                if args.object_only:
                    print(f'COMPILOU\t{rel}\t{profile}\tobjeto; nao ligado nem executado')
                    passed += 1
                    continue
                if args.linker:
                    exe = temp / 'linked'
                    proc = call([args.linker, str(binary), library, '-lm', '-o', str(exe)])
                    if proc is None or proc.returncode:
                        message = 'timeout' if proc is None else ' '.join(proc.stderr.decode(errors='replace').strip().splitlines()[:2])
                        print(f'FALHA\t{rel}\t{profile}\tligacao: {message}')
                        failed += 1
                        continue
                    binary = exe
                entrada = DOCS / (str(fixture) + '.entrada.txt') if fixture else None
                proc = call([str(binary)], stdin=entrada.read_bytes() if entrada and entrada.exists() else b'')
                error = None
                if proc is None:
                    error = 'execucao: timeout'
                elif proc.returncode:
                    error = f'execucao: codigo {proc.returncode}'
                elif fixture:
                    expected = DOCS / (fixture + '.esperado.txt')
                    contains = DOCS / (fixture + '.contem.txt')
                    stderr = DOCS / (fixture + '.esperado_stderr.txt')
                    if not expected.exists() and not contains.exists():
                        error = 'sem contrato de stdout para este exemplo'
                    elif expected.exists() and proc.stdout.replace(b'\r', b'') != expected.read_bytes().replace(b'\r', b''):
                        error = 'stdout diverge do dourado'
                    elif contains.exists() and any(line not in proc.stdout for line in contains.read_bytes().splitlines() if line):
                        error = 'stdout nao contem trecho esperado'
                    elif stderr.exists() and stderr.read_bytes().strip() not in proc.stderr:
                        error = 'stderr nao contem trecho esperado'
                elif rel == 'examples/advanced/basicz/basicz.c':
                    if b'BASIC-Z' not in proc.stdout or b'Concluido.' not in proc.stdout:
                        error = 'stdout nao contem marcadores BASIC-Z e Concluido.'
                else:
                    error = 'sem contrato de stdout para este exemplo'
                if error:
                    print(f'FALHA\t{rel}\t{profile}\t{error}')
                    failed += 1
                else:
                    print(f'OK\t{rel}\t{profile}\texecutado e saida conferida')
                    passed += 1
    print(f'RESUMO\tfontes={len(sources)}\taprovados={passed}\tfalhas={failed}\tsondagens_compiladas={probe_ok}\tsondagens_falharam={probe_failed}')
    return 1 if failed or probe_failed else 0


if __name__ == '__main__':
    sys.exit(main())
