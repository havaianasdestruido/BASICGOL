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
        -o "$bin" "$T_LIBRARY" -lm 2> "$T_TMP/err.log"
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
    "$T_CC" $T_CFLAGS "${objs[@]}" -o "$bin" "$T_LIBRARY" -lm 2>> "$T_TMP/err.log"
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
        -o "$bin" "$T_LIBRARY" -lm 2> "$T_TMP/err.log"
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

# Windows CRT writes stdout in text mode (LF becomes CRLF). Compare console
# lines, not the platform's newline spelling; keep the golden files LF-only.
# Use real temporary files: MSYS2 process-substitution paths are not portable.
difere() {
    local esperado="$T_TMP/esperado.norm" obtido="$T_TMP/obtido.norm"
    tr -d '\r' < "$1" > "$esperado" || return 1
    tr -d '\r' < "$2" > "$obtido" || return 1
    local resultado=0
    diff -q "$esperado" "$obtido" > /dev/null 2>&1 || resultado=$?
    if [ "${T_DEBUG_COMPARISON:-0}" = 1 ]; then
        {
            printf 'files: %s %s\n' "$1" "$2"
            od -An -tx1 "$esperado"
            od -An -tx1 "$obtido"
            diff -u "$esperado" "$obtido"
            printf 'diff rc=%s\n' "$resultado"
        } | base64 -w 0 > "$PWD/build/comparison-debug.txt"
    fi
    [ "$resultado" -ne 0 ]
}

# Compara com o resultado principal ou, para casos dependentes de C11/plataforma,
# com o conjunto opcional alternativo (alternativo.txt, alternativo_rc.txt, etc.).
resultado_confere() {
    local dir="$1" rc="$2" saida="$3" erros="$4"
    local rc_esperado="0" alt_rc="0"

    [ -f "$dir/esperado_rc.txt" ] && rc_esperado="$(cat "$dir/esperado_rc.txt")"
    if [ "$rc" = "$rc_esperado" ] && ! difere "$dir/esperado.txt" "$saida"; then
        if [ ! -f "$dir/esperado_stderr.txt" ] \
           || grep -qF -- "$(cat "$dir/esperado_stderr.txt")" "$erros"; then
            return 0
        fi
    fi

    [ -f "$dir/alternativo.txt" ] || return 1
    [ -f "$dir/alternativo_rc.txt" ] && alt_rc="$(cat "$dir/alternativo_rc.txt")"
    [ "$rc" = "$alt_rc" ] || return 1
    difere "$dir/alternativo.txt" "$saida" && return 1
    if [ -f "$dir/alternativo_stderr.txt" ]; then
        grep -qF -- "$(cat "$dir/alternativo_stderr.txt")" "$erros" || return 1
    elif [ -s "$erros" ]; then
        return 1
    fi
    return 0
}

mostra_diff() {
    if ! difere "$1" "$2"; then
        return
    fi
    diff -u "$T_TMP/esperado.norm" "$T_TMP/obtido.norm" | sed -n '1,12p' | sed 's/^/        /'
}

# --------------------------------------------------------------- caso completo

# caso_execucao <caminho-do-caso>
# Convenção de arquivos do caso:
#   *.c, *.h           fontes (a fonte BG é um .c com #include <basicgol.h>)
#   entrada.txt        stdin (opcional)
#   esperado.txt       stdout esperado (obrigatório neste tipo)
#   esperado_rc.txt    código de saída esperado (padrão 0)
#   esperado_stderr.txt  trecho que DEVE aparecer em stderr
#   alternativo.txt + arquivos rc/stderr opcionais: alternativa; sem stderr declarado, exige stderr vazio
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
        if resultado_confere "$dir" "$rc" "$T_TMP/out0" "$T_TMP/err0"; then
            t_ok "$nome (M0)"
        else
            t_falhou "$nome (M0)" "resultado diferente (codigo $rc)"
            if [ "$rc" != "$esperado_rc" ]; then
                :
            elif difere "$dir/esperado.txt" "$T_TMP/out0"; then
                mostra_diff "$dir/esperado.txt" "$T_TMP/out0"
            elif [ -f "$dir/esperado_stderr.txt" ]; then
                printf '        stderr esperado: %s\n' "$(cat "$dir/esperado_stderr.txt")"
            fi
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
        if resultado_confere "$dir" "$rc" "$T_TMP/out1" "$T_TMP/err1"; then
            t_ok "$nome (M1)"
        else
            t_falhou "$nome (M1)" "resultado diferente (codigo $rc)"
            if [ "$rc" != "$esperado_rc" ]; then
                :
            elif difere "$dir/esperado.txt" "$T_TMP/out1"; then
                mostra_diff "$dir/esperado.txt" "$T_TMP/out1"
            elif [ -f "$dir/esperado_stderr.txt" ]; then
                printf '        stderr esperado: %s\n' "$(cat "$dir/esperado_stderr.txt")"
            fi
        fi
    fi
}

# normaliza_aspas < arquivo > arquivo
# GCC e Clang usam aspas tipográficas (‘x’, “x”) quando o locale é UTF-8 e
# aspas retas quando é "C". O diagnóstico é do compilador, não da linguagem:
# a comparação não pode depender disso.
normaliza_aspas() {
    sed -e "s/‘/'/g" -e "s/’/'/g" -e 's/“/"/g' -e 's/”/"/g'
}

# confere_diagnostico <saida-do-compilador> <esperado_erro.txt>
#   esperado_erro.txt lista, uma por linha, as mensagens ACEITAS para o caso;
#   linha vazia é ignorada e linha começando com '#' é comentário. Basta uma
#   casar (texto fixo). Assim cada caso fixa o defeito que quer provar sem
#   depender da redação de um compilador ou de uma versão dele — a redação
#   muda entre GCC e Clang e entre versões do mesmo GCC.
#   Deixa em DIAG_DETALHE a razão da falha, quando falha.
confere_diagnostico() {
    local saida="$1" esperados="$2" trecho tentados=""

    DIAG_DETALHE=""
    normaliza_aspas < "$saida" > "$T_TMP/diag.norm"
    while IFS= read -r trecho || [ -n "$trecho" ]; do
        case "$trecho" in ''|'#'*) continue ;; esac
        tentados="$tentados [$trecho]"
        if grep -qF -- "$trecho" "$T_TMP/diag.norm"; then
            return 0
        fi
    done < "$esperados"
    if [ -z "$tentados" ]; then
        DIAG_DETALHE="esperado_erro.txt nao declara nenhum trecho comparavel"
    else
        DIAG_DETALHE="diagnostico nao casa com nenhum de:$tentados"
    fi
    return 1
}

# caso_erro_compilacao <caminho-do-caso>
#   programa.c + esperado_erro.txt (mensagens aceitas do erro de compilação M0)
caso_erro_compilacao() {
    local dir="$1" nome

    nome="$(echo "$dir" | sed "s|^$T_CASOS/||")"
    if compila_m0 "$dir" "$T_TMP/bin_neg"; then
        t_falhou "$nome" "compilou, mas deveria falhar"
        return
    fi
    if confere_diagnostico "$T_TMP/err.log" "$dir/esperado_erro.txt"; then
        t_ok "$nome"
    else
        t_falhou "$nome" "$DIAG_DETALHE: $(head -2 "$T_TMP/err.log" | tr '\n' ' ')"
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
#   programa.c + esperado_erro.txt (mensagens aceitas do basicgolc)
caso_erro_transpilacao() {
    local dir="$1" nome

    nome="$(echo "$dir" | sed "s|^$T_CASOS/||")"
    if "$T_BASICGOLC" "$dir/programa.c" -o "$T_TMP/neg_m1.c" 2> "$T_TMP/err.m1"; then
        t_falhou "$nome" "transpilou, mas deveria falhar"
        return
    fi
    if confere_diagnostico "$T_TMP/err.m1" "$dir/esperado_erro.txt"; then
        t_ok "$nome"
    else
        t_falhou "$nome" "$DIAG_DETALHE: $(head -2 "$T_TMP/err.m1" | tr '\n' ' ')"
    fi
}
