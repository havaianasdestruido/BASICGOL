#!/usr/bin/env bash
# lib.sh — funções do runner de testes BASICGOL.
# Uso: source tests/lib.sh  (run.sh faz isso)
#
# Não é para ser executado sozinho. Todas as variáveis T_* pertencem ao run.sh.

# ------------------------------------------------------------------ infraestrutura

t_ok() {
    T_OK=$((T_OK + 1))
    if [ "${T_VERBOSO:-0}" = "1" ]; then
        printf '  ok   %s\n' "$1"
    fi
}

t_falhou() {
    T_FALHOU=$((T_FALHOU + 1))
    printf '  FALHA %s\n' "$1"
    if [ -n "${2:-}" ]; then
        printf '        %s\n' "$2"
    fi
    T_LISTA_FALHAS="${T_LISTA_FALHAS}
  - $1"
}

t_titulo() {
    printf '\n== %s\n' "$1"
}

# ---------------------------------------------------------------- compilação M0

# compila_m0 <dir> <bin>
# Compila todos os .c do diretório como um programa C11 (a fonte BG é .c mesmo).
# stderr vai para $T_TMP/err.log; devolve 0/1.
compila_m0() {
    local dir="$1" bin="$2"
    local fontes=() f extras=""

    [ -f "$dir/cflags.txt" ] && extras="$(cat "$dir/cflags.txt")"

    for f in "$dir"/*.c; do
        [ -e "$f" ] || continue
        fontes+=("$f")
    done
    : > "$T_TMP/err.log"
    if [ ${#fontes[@]} -eq 0 ]; then
        echo "nenhum fonte em $dir" > "$T_TMP/err.log"
        return 1
    fi
    "$T_CC" $T_CFLAGS $extras -I "$T_INCLUDE" -I "$dir" "${fontes[@]}" \
        -o "$bin" -lm 2> "$T_TMP/err.log"
}

# compila_objetos <dir> <bin>
# Compila cada fonte separadamente (-c) e liga no fim — exercita EXTERNO/ESTATICO
# e o cenário multi-arquivo real.
compila_objetos() {
    local dir="$1" bin="$2"
    local objs=() f base

    : > "$T_TMP/err.log"
    for f in "$dir"/*.c; do
        [ -e "$f" ] || continue
        base="$T_TMP/obj_$(basename "$f").o"
        if ! "$T_CC" $T_CFLAGS -I "$T_INCLUDE" -I "$dir" -c "$f" -o "$base" \
             2>> "$T_TMP/err.log"; then
            return 1
        fi
        objs+=("$base")
    done
    "$T_CC" $T_CFLAGS "${objs[@]}" -o "$bin" -lm 2>> "$T_TMP/err.log"
}

# ---------------------------------------------------------------- compilação M1

# transpila <dir> <outdir>  -> gera o C de cada fonte
transpila() {
    local dir="$1" saida="$2" f base

    mkdir -p "$saida"
    : > "$T_TMP/err.m1"
    for f in "$dir"/*.c; do
        [ -e "$f" ] || continue
        base="$(basename "$f")"
        if ! "$T_BASICGOLC" "$f" -o "$saida/$base" 2>> "$T_TMP/err.m1"; then
            return 1
        fi
    done
    return 0
}

# compila_m1 <dir> <bin>
# Transpila cada .c do caso e compila o C gerado (a fonte BG já é .c).
compila_m1() {
    local dir="$1" bin="$2"
    local gdir="$T_TMP/m1_$(echo "$dir" | tr '/' '_')"
    local fontes=() f extras=""

    [ -f "$dir/cflags.txt" ] && extras="$(cat "$dir/cflags.txt")"

    rm -rf "$gdir"
    if ! transpila "$dir" "$gdir"; then
        return 1
    fi
    # no caminho M1 compila-se SÓ o C gerado: a fonte já foi traduzida
    for f in "$gdir"/*.c; do
        [ -e "$f" ] && fontes+=("$f")
    done
    : > "$T_TMP/err.log"
    "$T_CC" $T_CFLAGS $extras -I "$T_INCLUDE" -I "$dir" "${fontes[@]}" \
        -o "$bin" -lm 2> "$T_TMP/err.log"
}

# ------------------------------------------------------------------- execução

# roda <bin> <entrada|-> <arquivo_saida> <arquivo_err> ; ecoa o código de saída
roda() {
    local bin="$1" entrada="$2" saida="$3" errf="$4" rc

    if [ "$entrada" = "-" ]; then
        "$bin" > "$saida" 2> "$errf" < /dev/null
    else
        "$bin" > "$saida" 2> "$errf" < "$entrada"
    fi
    rc=$?
    echo "$rc"
}

# ------------------------------------------------------------------ comparações

difere() {
    ! diff -q "$1" "$2" > /dev/null 2>&1
}

mostra_diff() {
    diff -u "$1" "$2" | sed -n '1,12p' | sed 's/^/        /'
}

# --------------------------------------------------------------- caso completo

# caso_execucao <caminho-do-caso>
# Convenção de arquivos do caso:
#   *.c, *.h           fontes (a fonte BG é um .c com #include <basicgol.h>)
#   entrada.txt        stdin (opcional)
#   esperado.txt       stdout esperado (obrigatório neste tipo)
#   esperado_rc.txt    código de saída esperado (padrão 0)
#   esperado_stderr.txt  trecho que DEVE aparecer em stderr
#   sem_m0 / sem_m1    marcadores para pular um caminho
#   objetos            marcador: compilar em etapas separadas (-c + link)
caso_execucao() {
    local dir="$1" nome entrada="" esperado_rc="0" bin rc

    nome="$(echo "$dir" | sed "s|^$T_CASOS/||")"
    [ -f "$dir/esperado.txt" ] || { t_falhou "$nome" "caso sem esperado.txt"; return; }
    [ -f "$dir/entrada.txt" ] && entrada="$dir/entrada.txt"
    [ -f "$dir/esperado_rc.txt" ] && esperado_rc="$(cat "$dir/esperado_rc.txt")"

    # ---- caminho M0
    if [ ! -e "$dir/sem_m0" ]; then
        bin="$T_TMP/bin_m0_$(echo "$nome" | tr '/' '_')"
        if [ -e "$dir/objetos" ]; then
            compila_objetos "$dir" "$bin" || { t_falhou "$nome (M0)" "compilacao falhou: $(head -3 "$T_TMP/err.log" | tr '\n' ' ')"; return; }
        else
            compila_m0 "$dir" "$bin" || { t_falhou "$nome (M0)" "compilacao falhou: $(head -3 "$T_TMP/err.log" | tr '\n' ' ')"; return; }
        fi
        rc=$(roda "$bin" "${entrada:--}" "$T_TMP/out0" "$T_TMP/err0")
        if [ "$rc" != "$esperado_rc" ]; then
            t_falhou "$nome (M0)" "codigo de saida $rc, esperado $esperado_rc"
        elif difere "$dir/esperado.txt" "$T_TMP/out0"; then
            t_falhou "$nome (M0)" "saida diferente:"
            mostra_diff "$dir/esperado.txt" "$T_TMP/out0"
        elif [ -f "$dir/esperado_stderr.txt" ] && ! grep -qF "$(cat "$dir/esperado_stderr.txt")" "$T_TMP/err0"; then
            t_falhou "$nome (M0)" "stderr nao contem: $(cat "$dir/esperado_stderr.txt")"
        else
            t_ok "$nome (M0)"
        fi
    fi

    # ---- caminho M1 (equivalência com o esperado, não com o M0)
    if [ ! -e "$dir/sem_m1" ]; then
        bin="$T_TMP/bin_m1_$(echo "$nome" | tr '/' '_')"
        if ! compila_m1 "$dir" "$bin"; then
            t_falhou "$nome (M1)" "transpilacao/compilacao falhou: $(head -3 "$T_TMP/err.m1" "$T_TMP/err.log" 2>/dev/null | tr '\n' ' ')"
            return
        fi
        rc=$(roda "$bin" "${entrada:--}" "$T_TMP/out1" "$T_TMP/err1")
        if [ "$rc" != "$esperado_rc" ]; then
            t_falhou "$nome (M1)" "codigo de saida $rc, esperado $esperado_rc"
        elif difere "$dir/esperado.txt" "$T_TMP/out1"; then
            t_falhou "$nome (M1)" "saida diferente do esperado:"
            mostra_diff "$dir/esperado.txt" "$T_TMP/out1"
        else
            t_ok "$nome (M1)"
        fi
    fi
}

# caso_erro_compilacao <caminho-do-caso>
#   programa.c + esperado_erro.txt (trecho do erro de compilação M0)
caso_erro_compilacao() {
    local dir="$1" nome trecho

    nome="$(echo "$dir" | sed "s|^$T_CASOS/||")"
    trecho="$(cat "$dir/esperado_erro.txt")"
    if compila_m0 "$dir" "$T_TMP/bin_neg"; then
        t_falhou "$nome" "compilou, mas deveria falhar"
        return
    fi
    if grep -qF "$trecho" "$T_TMP/err.log"; then
        t_ok "$nome"
    else
        t_falhou "$nome" "erro nao contem '$trecho': $(head -2 "$T_TMP/err.log" | tr '\n' ' ')"
    fi
}

# caso_aviso <caminho-do-caso> <codigo>
#   caso cujo basicgolc deve emitir um aviso W-xx
caso_aviso() {
    local dir="$1" codigo="$2" nome

    nome="$(echo "$dir" | sed "s|^$T_CASOS/||") [aviso $codigo]"
    if ! "$T_BASICGOLC" "$dir/programa.c" -o /dev/null 2> "$T_TMP/err.aviso"; then
        t_falhou "$nome" "basicgolc falhou: $(head -2 "$T_TMP/err.aviso" | tr '\n' ' ')"
        return
    fi
    if grep -q "\[$codigo\]" "$T_TMP/err.aviso"; then
        t_ok "$nome"
    else
        t_falhou "$nome" "aviso nao emitido"
    fi
}

# caso_erro_transpilacao <caminho-do-caso>
#   programa.c + esperado_erro.txt (trecho que o basicgolc deve reportar)
caso_erro_transpilacao() {
    local dir="$1" nome trecho

    nome="$(echo "$dir" | sed "s|^$T_CASOS/||")"
    trecho="$(cat "$dir/esperado_erro.txt")"
    if "$T_BASICGOLC" "$dir/programa.c" -o "$T_TMP/neg_m1.c" 2> "$T_TMP/err.m1"; then
        t_falhou "$nome" "transpilou, mas deveria falhar"
        return
    fi
    if grep -qF "$trecho" "$T_TMP/err.m1"; then
        t_ok "$nome"
    else
        t_falhou "$nome" "erro nao contem '$trecho': $(head -2 "$T_TMP/err.m1" | tr '\n' ' ')"
    fi
}
