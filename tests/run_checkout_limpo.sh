#!/usr/bin/env bash
# run_checkout_limpo.sh — os comandos documentados funcionam a partir de um
# checkout limpo? (Fase 6)
#
#   ./tests/run_checkout_limpo.sh
#   ./tests/run_checkout_limpo.sh -v
#
# O que este script faz:
#   1. monta um "checkout limpo" em um diretório temporário copiando só o que o
#      Git versionaria (arquivos rastreados + não ignorados) — sem build/,
#      build-sanitize/, build-matriz/ nem lixo de execução anterior;
#   2. roda, nessa cópia, exatamente os comandos do README: make, make
#      biblioteca, make test, make test-biblioteca, make exemplos, make
#      cobertura, o comando M0 de uma linha e o fluxo M1 de duas etapas;
#   3. confere que `make cobertura` não altera tests/COBERTURA.md (é o que o CI
#      verifica) e que o programa mínimo imprime o esperado.
#
# Sai 0 se todos os comandos funcionarem. Qualquer falha é reportada com a
# saída do comando.

set -u

RAIZ=$(cd "$(dirname "$0")/.." && pwd)
VERBOSO=0
[ "${1:-}" = "-v" ] && VERBOSO=1
TMP=$(mktemp -d "${TMPDIR:-/tmp}/basicgol-checkout.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
DEST="$TMP/checkout"

OK=0
FALHOU=0
FALHAS=""

ok() {
    OK=$((OK + 1))
    [ "$VERBOSO" = 1 ] && printf '  ok   %s\n' "$1"
    return 0
}
falhou() {
    FALHOU=$((FALHOU + 1))
    printf '  FALHA %s\n' "$1"
    [ -n "${2:-}" ] && printf '        %s\n' "$2"
    FALHAS="$FALHAS
  - $1"
    return 0
}

# ------------------------------------------------------------- 1. cópia limpa
mkdir -p "$DEST"
( cd "$RAIZ" && {
      git ls-files
      git ls-files --others --exclude-standard
  } | sort -u | tar -cf - -T - ) | tar -xf - -C "$DEST" 2> "$TMP/tar.err"

if [ -f "$DEST/Makefile" ] && [ -f "$DEST/include/basicgol.h" ]; then
    ok "checkout limpo montado ($(find "$DEST" -type f | wc -l) arquivos, sem build/)"
else
    echo "run_checkout_limpo.sh: não consegui montar o checkout em $DEST" >&2
    cat "$TMP/tar.err" >&2
    exit 2
fi

if [ -d "$DEST/build" ] || [ -d "$DEST/build-sanitize" ]; then
    falhou "o checkout limpo não deveria conter diretórios de build"
else
    ok "nenhum artefato de build veio junto"
fi

roda() {
    local rotulo="$1"; shift
    if "$@" > "$TMP/saida.log" 2>&1; then
        ok "$rotulo"
    else
        falhou "$rotulo" "$(tail -5 "$TMP/saida.log" | tr '\n' ' ')"
    fi
}

# ------------------------------------------------- 2. comandos do README
printf '\n== comandos documentados no README\n'
roda "make (transpilador + biblioteca)" make -C "$DEST" --no-print-directory
roda "make biblioteca" make -C "$DEST" --no-print-directory biblioteca
roda "make test" make -C "$DEST" --no-print-directory test
roda "make test-biblioteca" make -C "$DEST" --no-print-directory test-biblioteca
roda "make exemplos" make -C "$DEST" --no-print-directory exemplos
cp "$DEST/tests/COBERTURA.md" "$TMP/COBERTURA.antes.md"
roda "make cobertura" make -C "$DEST" --no-print-directory cobertura

# Equivalente à checagem do CI (`git diff --exit-code`), sem depender de .git:
# o arquivo gerado tem de ser idêntico ao versionado.
if cmp -s "$TMP/COBERTURA.antes.md" "$DEST/tests/COBERTURA.md"; then
    ok "make cobertura não altera tests/COBERTURA.md (checagem do CI)"
else
    falhou "make cobertura alterou tests/COBERTURA.md" \
           "$(diff -u "$TMP/COBERTURA.antes.md" "$DEST/tests/COBERTURA.md" | head -6 | tr '\n' ' ')"
fi

# comando M0 de uma linha, como no README
if ( cd "$DEST" && ${CC:-cc} -std=c11 -Wall -Wextra -I include \
        docs/exemplos/ola_mundo.c build/libbasicgol.a -lm -o ola_m0 \
        > "$TMP/m0.log" 2>&1 && [ "$(./ola_m0)" = "Ola, mundo!" ] ); then
    ok "comando M0 de uma linha do README compila e imprime 'Ola, mundo!'"
else
    falhou "comando M0 de uma linha" "$(tail -4 "$TMP/m0.log" | tr '\n' ' ')"
fi

# fluxo M1 de duas etapas
if ( cd "$DEST" && build/basicgolc docs/exemplos/controle_lacos.c -o /tmp/gerado.c \
        > "$TMP/m1.log" 2>&1 \
     && ${CC:-cc} -std=c11 -Wall -Wextra -I include /tmp/gerado.c \
        build/libbasicgol.a -lm -o /tmp/gerado_m1 >> "$TMP/m1.log" 2>&1 ); then
    ok "fluxo M1 de duas etapas do README (transpila e compila o C gerado)"
else
    falhou "fluxo M1 de duas etapas" "$(tail -4 "$TMP/m1.log" | tr '\n' ' ')"
fi

# make test-portabilidade também precisa funcionar do zero
roda "make test-portabilidade" make -C "$DEST" --no-print-directory test-portabilidade

# e o fuzz, que depende de python3
if command -v python3 > /dev/null 2>&1; then
    roda "make fuzz (100 mutações)" make -C "$DEST" --no-print-directory fuzz
else
    printf '  --   python3 ausente: make fuzz ignorado\n'
fi

printf '\n=============================================\n'
if [ "$FALHOU" -eq 0 ]; then
    printf 'CHECKOUT LIMPO OK: %d comandos/verificações\n' "$OK"
    exit 0
fi
printf 'FALHAS: %d de %d%s\n' "$FALHOU" "$((OK + FALHOU))" "$FALHAS"
exit 1
