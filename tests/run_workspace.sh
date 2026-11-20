#!/usr/bin/env bash
# Testes da workspace falsa (BG_USAR_WORKSPACE) — módulo opcional da biblioteca.
#
#   ./tests/run_workspace.sh build/test-workspace
#
# Também confirma que, sem -DBG_USAR_WORKSPACE, o cabeçalho não declara API
# (tests/workspace_desligado.c compila com #error se declarar).
set -u

if [ "$#" -ne 1 ] || [ ! -x "$1" ]; then
    echo "uso: $0 build/test-workspace" >&2
    exit 2
fi

TESTE="$1"
RAIZ=$(cd "$(dirname "$0")/.." && pwd)
CC="${CC:-cc}"
CFLAGS="${T_CFLAGS:--std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror}"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/basicgol-workspace.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
OK=0
FALHAS=0

falha() {
    FALHAS=$((FALHAS + 1))
    printf '  FALHA %s\n' "$1"
    if [ -n "${2:-}" ]; then
        printf '        %s\n' "$2"
    fi
}

printf '%s\n' '== workspace falsa (BG_USAR_WORKSPACE)'

if "$TESTE" > "$TMP/out" 2> "$TMP/err" \
   && grep -Eq '^workspace: [0-9]+ verificacoes aprovadas$' "$TMP/out" \
   && [ ! -s "$TMP/err" ]; then
    cat "$TMP/out"
    OK=$((OK + 1))
else
    falha "testes unitarios" "$(head -3 "$TMP/err" "$TMP/out" | tr '\n' ' ')"
fi

if "$CC" $CFLAGS -I "$RAIZ/include" -o "$TMP/desligado" \
        "$RAIZ/tests/workspace_desligado.c" 2> "$TMP/e"; then
    printf '  ok   sem BG_USAR_WORKSPACE o cabeçalho não declara API\n'
    OK=$((OK + 1))
else
    falha "cabeçalho desligado por padrão" "$(head -3 "$TMP/e" | tr '\n' ' ')"
fi

printf '\n'
if [ "$FALHAS" -eq 0 ]; then
    printf 'TUDO OK: %d verificacoes da workspace aprovadas\n' "$OK"
    exit 0
fi
printf 'FALHAS: %d de %d verificacoes da workspace\n' "$FALHAS" "$((OK + FALHAS))"
exit 1
