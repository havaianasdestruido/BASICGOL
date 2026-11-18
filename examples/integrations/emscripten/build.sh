#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/../../.." && pwd)
cd "$HERE"
mkdir -p build

compile_one() {
    sample=$1
    symbol="_$2"
    exports="[\"${symbol}\"]"
    emcc -std=c11 -O2 -Wall -Wextra -I "$ROOT/include" --no-entry \
        -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web \
        "-sEXPORTED_FUNCTIONS=${exports}" \
        "$sample/basicgol.c" -o "build/$sample.js"
}

compile_one 01_soma bg_soma
compile_one 02_fatorial bg_fatorial
compile_one 03_media bg_media3
compile_one 04_maior bg_maior
compile_one 05_primo bg_e_primo
compile_one 06_fahrenheit bg_fahrenheit_para_celsius
compile_one 07_limita bg_limita
printf 'Sete modulos WebAssembly criados em %s/build\n' "$HERE"
