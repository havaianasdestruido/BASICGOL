#!/usr/bin/env bash
# run_matriz.sh — roda a suíte inteira com cada compilador C desta máquina.
#
#   ./tests/run_matriz.sh
#   ./tests/run_matriz.sh -v        mostra cada verificação aprovada
#
# Cada compilador ganha o seu próprio diretório de build (build-matriz/<nome>),
# porque o Makefile não rastreia mudança de flags: compilar metade com gcc e
# metade com clang no mesmo diretório produz um link com objetos incompatíveis
# (ou pior, silenciosamente misto). Isto é uma observação da Fase 6, não um
# bug: `make limpar` sempre funcionou, mas `make CC=... test` depois de um build
# com outro compilador NÃO recompila.
#
# Sai 0 se todos os compiladores encontrados passarem; 2 se nenhum existir.

set -u

RAIZ=$(cd "$(dirname "$0")/.." && pwd)
VERBOSO=0
[ "${1:-}" = "-v" ] && VERBOSO=1
BASE="$RAIZ/build-matriz"

candidatos="${CC:-} cc gcc clang clang-20 clang-19 clang-18 clang-17 tcc ${T_CC_ALT:-}"

encontrados=""
vistos=""
for c in $candidatos; do
    [ -n "$c" ] || continue
    bin=$(command -v "$c" 2>/dev/null) || continue
    real=$(readlink -f "$bin" 2>/dev/null || printf '%s' "$bin")
    case " $vistos " in
        *" $real "*) continue ;;
    esac
    vistos="$vistos $real"
    encontrados="$encontrados $c"
done

if [ -z "$encontrados" ]; then
    echo "run_matriz.sh: nenhum compilador C encontrado" >&2
    exit 2
fi

rm -rf "$BASE"
mkdir -p "$BASE"
falhas=""
total=0
for c in $encontrados; do
    total=$((total + 1))
    nome=$(echo "$c" | tr '/' '_')
    dir="build-matriz/$nome"
    log="$BASE/$nome.build.log"
    printf '\n===== %s (%s)\n' "$c" "$($c --version 2>/dev/null | head -1)"
    if ! make -C "$RAIZ" --no-print-directory CC="$c" BUILD="$dir" \
             all "$dir/test-biblioteca" > "$log" 2>&1; then
        printf '  FALHA compilar com %s\n' "$c"
        sed -n '1,5p' "$log" | sed 's/^/        /'
        falhas="$falhas $c"
        continue
    fi
    recado=""
    [ "$VERBOSO" = 1 ] && recado="-v"

    if T_BASICGOLC="$RAIZ/$dir/basicgolc" T_LIBRARY="$RAIZ/$dir/libbasicgol.a" \
       CC="$c" "$RAIZ/tests/run.sh" $recado; then
        :
    else
        falhas="$falhas $c(linguagem)"
    fi

    if "$RAIZ/tests/run_biblioteca.sh" "$RAIZ/$dir/test-biblioteca"; then
        :
    else
        falhas="$falhas $c(biblioteca)"
    fi
done

printf '\n=============================================\n'
if [ -z "$falhas" ]; then
    printf 'MATRIZ OK: %d compilador(es):%s\n' "$total" "$encontrados"
    exit 0
fi
printf 'MATRIZ COM FALHAS:%s\n' "$falhas"
exit 1
