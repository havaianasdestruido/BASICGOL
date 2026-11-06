#!/usr/bin/env bash
# run_msvc.sh — compila os exemplos de docs/exemplos com MSVC (cl.exe) e
# confere cada um contra a saída dourada, nos caminhos M0 e M1.
#
# Roda no Git Bash do Windows (ou em qualquer bash com cl.exe no PATH):
#   ./tests/run_msvc.sh
#
# Requisitos: cl.exe no PATH (Developer Command Prompt, ou o passo
# msvc-dev-cmd do workflow .github/workflows/msvc.yml) e os artefatos
#   build/basicgolc.exe    — cl /nologo /TC /std:c11 /W4 /WX tools\basicgolc.c /Fe:build\basicgolc.exe
#   build/libbasicgol.lib  — ver o workflow msvc.yml (lib.exe sobre src/biblioteca/*.obj)
# sobrescrevíveis via T_BASICGOLC e T_LIBRARY.
#
# As comparações normalizam \r dos dois lados: o MSVC escreve \n como \r\n
# no modo texto e um checkout com autocrlf também deixa \r nas saídas
# douradas. O resto fica byte a byte.
set -u
export LC_ALL=C

# No Git Bash (MSYS), argumentos que comecam com '/' sao convertidos para
# caminhos absolutos ('/nologo' viraria 'C:/Program Files/Git/nologo').
# O cl.exe aceita '-' como prefixo de opcao, e MSYS nunca converte argumentos
# iniciados por '-'; as variaveis abaixo desligam a conversao por completo
# (vale tambem para wrappers de terceiros que esperem '/').
export MSYS2_ARG_CONV_EXCL='*' MSYS_NO_PATHCONV=1

RAIZ=$(cd "$(dirname "$0")/.." && pwd)
cd "$RAIZ"

CL="${CL:-cl}"
T_BASICGOLC="${T_BASICGOLC:-build/basicgolc.exe}"
T_LIBRARY="${T_LIBRARY:-build/libbasicgol.lib}"
BUILD=build/exemplos-msvc
TMP=$(mktemp -d "${TMPDIR:-/tmp}/basicgol-msvc.XXXXXX")
trap 'rm -rf "$TMP"' EXIT

DEF="-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS"
# Núcleo do produto usa /W4 /WX no workflow; os exemplos são código didático e
# verificam comportamento (saída dourada), não diagnósticos — aqui W3 sem WX:
# um aviso do MSVC não deve reprovar um programa que produz a saída correta.
# Flags com '-' (e nao '/') por causa da conversao de caminhos do MSYS.
FLAGS="-nologo -TC -std:c11 -W3 $DEF"

OK=0
FALHAS=0

ok() {
    OK=$((OK + 1))
    printf '  ok   %s\n' "$1"
}

# Em GitHub Actions, despeja o log de falha como annotations (::error::),
# para que o motivo apareça no resumo do run e via API. Fora do Actions,
# é silencioso.
anota() {
    [ "${GITHUB_ACTIONS:-}" = "true" ] || return 0
    [ -n "${1:-}" ] || return 0
    head -12 "$1" | while IFS= read -r linha; do
        printf '::error::%s\n' "$linha"
    done
}

falha() {
    FALHAS=$((FALHAS + 1))
    printf '  FALHA %s\n' "$1"
    if [ -n "${2:-}" ]; then
        printf '        %s\n' "$2"
    fi
    if [ "${GITHUB_ACTIONS:-}" = "true" ]; then
        printf '::error::FALHA %s\n' "$1"
        [ -n "${2:-}" ] && printf '::error::%s\n' "$2"
        return 0
    fi
}

confere_saida() {
    local nome="$1" rotulo="$2" bin="$3" entrada="$4" rc linha falhou=0

    "$bin" < "$entrada" > "$TMP/out.raw" 2> "$TMP/err.raw"
    rc=$?
    # Normaliza CRLF: o CRT no Windows escreve \r\n; o restante é byte a byte.
    tr -d '\r' < "$TMP/out.raw" > "$TMP/out"
    tr -d '\r' < "$TMP/err.raw" > "$TMP/err"
    if [ "$rc" != "0" ]; then
        falha "$nome ($rotulo)" "codigo de saida $rc"
        return
    fi
    if [ -f "docs/exemplos/$nome.esperado.txt" ]; then
        if ! diff -q <(tr -d '\r' < "docs/exemplos/$nome.esperado.txt") \
                     "$TMP/out" > /dev/null; then
            falha "$nome ($rotulo)" \
                  "saida difere do dourado (docs/exemplos/$nome.esperado.txt)"
            diff -u <(tr -d '\r' < "docs/exemplos/$nome.esperado.txt") \
                     "$TMP/out" > "$TMP/diff" || true
            sed -n '1,10p' "$TMP/diff" | sed 's/^/        /'
            if [ "${GITHUB_ACTIONS:-}" = "true" ]; then
                # so as linhas que mudaram: GitHub exibe ~10 annotations por
                # passo; um dump inteiro esconde o diff de verdade
                grep -E '^[+-]' "$TMP/diff" | grep -v -E '^(\+\+\+|---)' \
                    | while IFS= read -r linha; do
                        printf '::error::%s\n' "$linha"
                    done
            fi
            return
        fi
    fi
    if [ -f "docs/exemplos/$nome.contem.txt" ]; then
        while IFS= read -r linha; do
            linha="${linha%$'\r'}"
            [ -z "$linha" ] && continue
            if ! grep -qF -- "$linha" "$TMP/out"; then
                falha "$nome ($rotulo)" "saida nao contem: $linha"
                if [ "${GITHUB_ACTIONS:-}" = "true" ]; then
                    printf '::error::saida produzida:\n'
                    sed -n '1,20p' "$TMP/out" | while IFS= read -r linha_out; do
                        printf '::error::| %s\n' "$linha_out"
                    done
                fi
                falhou=1
            fi
        done < "docs/exemplos/$nome.contem.txt"
        [ "$falhou" = "1" ] && return
    fi
    if [ -f "docs/exemplos/$nome.esperado_stderr.txt" ]; then
        if ! grep -qF -- "$(tr -d '\r' < "docs/exemplos/$nome.esperado_stderr.txt")" "$TMP/err"; then
            falha "$nome ($rotulo)" "stderr nao contem a linha esperada"
            return
        fi
    fi
    ok "$nome ($rotulo)"
}

if ! command -v "$CL" > /dev/null 2>&1; then
    echo "run_msvc.sh: cl.exe nao encontrado no PATH." >&2
    echo "        abre um Developer Command Prompt (ou rode via o workflow" >&2
    echo "        .github/workflows/msvc.yml) e tente de novo." >&2
    exit 2
fi
if [ ! -x "$T_BASICGOLC" ]; then
    echo "run_msvc.sh: basicgolc nao encontrado em $T_BASICGOLC" >&2
    echo "        compile-o antes (ver .github/workflows/msvc.yml)." >&2
    exit 2
fi
if [ ! -f "$T_LIBRARY" ]; then
    echo "run_msvc.sh: biblioteca nao encontrada em $T_LIBRARY" >&2
    echo "        empacote-a antes com lib.exe (ver .github/workflows/msvc.yml)." >&2
    exit 2
fi
mkdir -p "$BUILD"

printf '%s\n' '== exemplos BASICGOL construidos com MSVC (cl.exe)'
for fonte in docs/exemplos/*.c; do
    nome=$(basename "$fonte" .c)
    entrada="docs/exemplos/$nome.entrada.txt"
    [ -f "$entrada" ] || entrada=/dev/null
    exe="$BUILD/$nome.exe"

    # ---- caminho M0 (macros); dois exemplos so existem no M1
    if [ "$nome" != "para_contado" ] && [ "$nome" != "controle_para_acucar" ]; then
        if "$CL" $FLAGS -I include "$fonte" \
                -Fo:"$BUILD/$nome.obj" -Fe:"$exe" -link "$T_LIBRARY" > "$TMP/log" 2>&1; then
            confere_saida "$nome" M0 "./$exe" "$entrada"
        else
            anota "$TMP/log"
            falha "$nome (M0)" "compilacao falhou: $(head -3 "$TMP/log" | tr '\n' ' ')"
        fi
    fi

    # ---- caminho M1 (transpilador)
    gerado="$BUILD/$nome.gerado.c"
    if ! "$T_BASICGOLC" "$fonte" -o "$gerado" > "$TMP/log" 2>&1; then
        anota "$TMP/log"
        falha "$nome (M1)" "basicgolc falhou: $(head -3 "$TMP/log" | tr '\n' ' ')"
        continue
    fi
    if "$CL" $FLAGS -I include "$gerado" \
            -Fo:"$BUILD/$nome-m1.obj" -Fe:"$exe" -link "$T_LIBRARY" > "$TMP/log" 2>&1; then
        confere_saida "$nome" M1 "./$exe" "$entrada"
    else
        anota "$TMP/log"
        falha "$nome (M1)" "compilacao falhou: $(head -3 "$TMP/log" | tr '\n' ' ')"
    fi
done

printf '\n'
if [ "$FALHAS" -eq 0 ]; then
    printf 'TUDO OK: %d resultados de exemplos aprovados (MSVC)\n' "$OK"
    exit 0
fi
printf 'FALHAS: %d de %d verificacoes (MSVC)\n' "$FALHAS" "$((OK + FALHAS))"
exit 1
