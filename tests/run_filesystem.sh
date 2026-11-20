#!/usr/bin/env bash
# Isola os arquivos de teste e permite execução concorrente sem colisões.
set -eu
if [ "$#" -ne 1 ] || [ ! -x "$1" ]; then
    echo "uso: $0 build/test-filesystem" >&2
    exit 2
fi
TESTE="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/basicgol-filesystem.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
(cd "$TMP" && "$TESTE")
