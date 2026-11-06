#!/usr/bin/env bash
# Testes de API da biblioteca portátil BASICGOL.
set -u

if [ "$#" -ne 1 ] || [ ! -x "$1" ]; then
    echo "uso: $0 build/test-biblioteca" >&2
    exit 2
fi

TESTE="$1"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/basicgol-biblioteca.XXXXXX")
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

printf '%s\n' '== biblioteca padrão: módulos e limites'
if "$TESTE" > "$TMP/out" 2> "$TMP/err" \
   && grep -Eq '^biblioteca: [0-9]+ verificacoes aprovadas$' "$TMP/out" \
   && [ ! -s "$TMP/err" ]; then
    cat "$TMP/out"
    OK=$((OK + 1))
else
    falha "testes unitarios" "$(head -3 "$TMP/err" "$TMP/out" | tr '\n' ' ')"
fi

if printf '42\nlinha de texto\n-7\n3.5\n' | "$TESTE" --stdin > "$TMP/out" 2> "$TMP/err" \
   && [ "$(cat "$TMP/out")" = 'stdin: ok' ] && [ ! -s "$TMP/err" ]; then
    printf '  ok   wrappers de stdin + LEIA_TEXTO depois de scanf\n'
    OK=$((OK + 1))
else
    falha "wrappers de stdin" "$(head -3 "$TMP/err" "$TMP/out" | tr '\n' ' ')"
fi

if printf 'abc\n1e9999\n' | "$TESTE" --stdin-invalido > "$TMP/out" 2> "$TMP/err" \
   && [ "$(cat "$TMP/out")" = 'stdin invalido: ok' ] && [ ! -s "$TMP/err" ]; then
    printf '  ok   falhas numéricas preservam o destino\n'
    OK=$((OK + 1))
else
    falha "wrappers numericos invalidos" "$(head -3 "$TMP/err" "$TMP/out" | tr '\n' ' ')"
fi

if printf 'linha-longa\nok\n' | "$TESTE" --stdin-texto-longo > "$TMP/out" 2> "$TMP/err" \
   && [ "$(cat "$TMP/out")" = 'linha longa: ok' ] && [ ! -s "$TMP/err" ]; then
    printf '  ok   linha longa é descartada sem perder a próxima\n'
    OK=$((OK + 1))
else
    falha "linha longa" "$(head -3 "$TMP/err" "$TMP/out" | tr '\n' ' ')"
fi

fatal_esperado() {
    local modo="$1" trecho="$2" rc
    "$TESTE" "$modo" > "$TMP/fatal.out" 2> "$TMP/fatal.err"
    rc=$?
    if [ "$rc" -eq 1 ] && grep -qF -- "$trecho" "$TMP/fatal.err"; then
        printf '  ok   erro esperado: %s\n' "$modo"
        OK=$((OK + 1))
    else
        falha "erro esperado: $modo" "rc=$rc; $(head -2 "$TMP/fatal.err" | tr '\n' ' ')"
    fi
}

representabilidade_esperada() {
    local modo="$1" trecho="$2" descricao="$3" rc
    "$TESTE" "$modo" > "$TMP/fatal.out" 2> "$TMP/fatal.err"
    rc=$?
    if [ "$rc" -eq 3 ] && [ ! -s "$TMP/fatal.err" ]; then
        printf '  ok   caso representável nesta plataforma: %s\n' "$modo"
        OK=$((OK + 1))
    elif [ "$rc" -eq 1 ] && grep -qF -- "$trecho" "$TMP/fatal.err"; then
        printf '  ok   erro esperado: %s\n' "$modo"
        OK=$((OK + 1))
    else
        falha "$descricao: $modo" "rc=$rc; $(head -2 "$TMP/fatal.err" | tr '\n' ' ')"
    fi
}

fatal_esperado erro 'teste.c:37: erro: falha esperada'
fatal_esperado passo 'teste.c:38: erro: PARA_PASSO: passo zero'
fatal_esperado divide-zero 'teste.c:39: erro: divisao por zero'
representabilidade_esperada divide-estouro 'estouro em divisao inteira (INT_MIN / -1)' 'divisao por -1'
fatal_esperado divide-zero-l 'teste.c:43: erro: divisao por zero'
fatal_esperado divide-zero-ll 'teste.c:44: erro: divisao por zero'
fatal_esperado divide-zero-u 'teste.c:45: erro: divisao por zero'
fatal_esperado divide-zero-ul 'teste.c:46: erro: divisao por zero'
fatal_esperado divide-zero-ull 'teste.c:47: erro: divisao por zero'
fatal_esperado divide-zero-f 'teste.c:48: erro: divisao por zero (real)'
fatal_esperado divide-zero-d 'teste.c:49: erro: divisao por zero (real)'
fatal_esperado divide-zero-ld 'teste.c:50: erro: divisao por zero (real)'
representabilidade_esperada divide-estouro-l 'estouro em divisao inteira (LONG_MIN / -1)' 'divisao por -1'
representabilidade_esperada divide-estouro-ll 'estouro em divisao inteira (LLONG_MIN / -1)' 'divisao por -1'
fatal_esperado resto-zero 'teste.c:41: erro: resto por zero'
representabilidade_esperada resto-estouro 'estouro em resto inteiro (INT_MIN % -1)' 'resto por -1'
fatal_esperado resto-zero-l 'teste.c:51: erro: resto por zero'
fatal_esperado resto-zero-ll 'teste.c:52: erro: resto por zero'
fatal_esperado resto-zero-u 'teste.c:53: erro: resto por zero'
fatal_esperado resto-zero-ul 'teste.c:54: erro: resto por zero'
fatal_esperado resto-zero-ull 'teste.c:55: erro: resto por zero'
representabilidade_esperada resto-estouro-l 'estouro em resto inteiro (LONG_MIN % -1)' 'resto por -1'
representabilidade_esperada resto-estouro-ll 'estouro em resto inteiro (LLONG_MIN % -1)' 'resto por -1'
representabilidade_esperada abs-minimo 'ABS: valor absoluto de INT_MIN nao cabe em int' 'ABS mínimo'
representabilidade_esperada abs-minimo-l 'ABS: valor absoluto de LONG_MIN nao cabe em long' 'ABS mínimo'
representabilidade_esperada abs-minimo-ll 'ABS: valor absoluto de LLONG_MIN nao cabe em long long' 'ABS mínimo'

printf '\n'
if [ "$FALHAS" -eq 0 ]; then
    printf 'TUDO OK: %d verificacoes da biblioteca aprovadas\n' "$OK"
    exit 0
fi
printf 'FALHAS: %d de %d verificacoes da biblioteca\n' "$FALHAS" "$((OK + FALHAS))"
exit 1
