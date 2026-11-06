#!/usr/bin/env bash
# run_portabilidade.sh — Fase 6: matriz de compiladores, avisos e sanitizers.
#
#   ./tests/run_portabilidade.sh          roda tudo o que o ambiente permitir
#   ./tests/run_portabilidade.sh -v       mostra cada verificação aprovada
#   ./tests/run_portabilidade.sh -q       só o resumo e as falhas
#
# O que é verificado, por compilador disponível:
#   1. o cabeçalho compila limpo com -pedantic -Wall -Wextra -Wformat=2 -Werror
#      em C11, C17, gnu11 e (quando aceito) no próximo padrão;
#   2. C99 e C89 são recusados com a mensagem da linguagem;
#   3. cada cabeçalho público compila sozinho (autocontido);
#   4. precedência/ninhamento, controle de fluxo e fronteiras da biblioteca
#      compilam e passam (tests/portabilidade/*.c);
#   5. o mesmo fonte de controle de fluxo passa pelos dois caminhos, M0 e M1;
#   6. entrada com falhas produz exatamente a saída dourada;
#   7. três unidades de tradução com inclusão múltipla ligam sem conflito, com
#      BG_SEM_APELIDOS numa delas e BG_USAR_EXTENSOES em outra;
#   8. os cabeçalhos de módulo funcionam de C++ (e basicgol.h falha com
#      mensagem clara, como documentado em D-15);
#   9. a biblioteca e o transpilador não emitem avisos no conjunto que o
#      projeto adota (o conjunto é maior que o do Makefile, não menor);
#  10. sanitizers (ASan + UBSan) quando o compilador os suportar;
#  11. alvos cruzados, quando `zig` está no PATH: i386 (executa), aarch64
#      (compila) e x86_64-windows-gnu (compila e liga).
#
# Nada aqui é obrigatório para o projeto compilar: cada verificação que depende
# de ferramenta ausente é REPORTADA como ignorada, nunca contada como aprovada.
#
# Saída legível: stdout.  Matriz em TSV: build/relatorio/portabilidade.tsv.

set -u

export LC_ALL=C
RAIZ=$(cd "$(dirname "$0")/.." && pwd)
PORT="$RAIZ/tests/portabilidade"
INCLUDE="$RAIZ/include"
BUILD="${T_BUILD:-$RAIZ/build}"
FERRAMENTA="${T_BASICGOLC:-$BUILD/basicgolc}"
BIBLIOTECA="${T_LIBRARY:-$BUILD/libbasicgol.a}"
REL="$BUILD/relatorio"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/basicgol-port.XXXXXX")
trap 'rm -rf "$TMP"' EXIT

VERBOSO=0
QUIETO=0
case "${1:-}" in
    -v|--verboso) VERBOSO=1 ;;
    -q|--quieto)  QUIETO=1 ;;
esac

OK=0
FALHOU=0
IGNORADO=0
FALHAS=""
MATRIZ="$TMP/matriz.tsv"
: > "$MATRIZ"

# ---------------------------------------------------------------- utilidades

ok() {
    OK=$((OK + 1))
    [ "$VERBOSO" = 1 ] && printf '  ok   %s\n' "$1"
    printf '%s\t%s\t%s\n' "${CTAG:-?}" "ok" "$1" >> "$MATRIZ"
    return 0
}

falhou() {
    FALHOU=$((FALHOU + 1))
    printf '  FALHA %s\n' "$1"
    [ -n "${2:-}" ] && printf '        %s\n' "$2"
    FALHAS="$FALHAS
  - ${CTAG:-?}: $1"
    printf '%s\t%s\t%s\n' "${CTAG:-?}" "FALHA" "$1" >> "$MATRIZ"
    return 0
}

ignorado() {
    IGNORADO=$((IGNORADO + 1))
    [ "$QUIETO" = 0 ] && printf '  --   %s\n' "$1"
    printf '%s\t%s\t%s\n' "${CTAG:-?}" "IGNORADO" "$1" >> "$MATRIZ"
    return 0
}

titulo() { [ "$QUIETO" = 0 ] && printf '\n== %s\n' "$1"; return 0; }

primeira_linha() { head -1 "$1" 2>/dev/null | tr -d '\r'; return 0; }

# compila <compilador...> -- <flags...>   (usa "$TMP/e" para o stderr)
tenta() {
    local cc="$1"; shift
    $cc "$@" 2> "$TMP/e"
}

# ----------------------------------------------------- descoberta de compiladores
#
# Cada compilador é um comando (pode ter espaços, como "zig cc -target ...").
# A matriz é o produto compilador × padrão; o resto dos testes usa o primeiro.

COMPILADORES=()
NOMES=()
VERSOES=()

registra_compilador() {
    local cmd="$1" nome="$2" versao

    [ -n "$cmd" ] || return 0
    command -v "${cmd%% *}" > /dev/null 2>&1 || return 0
    versao=$($cmd --version 2>/dev/null | head -1)
    [ -n "$versao" ] || versao=$($cmd -v 2>&1 | head -1)
    # cc e gcc costumam ser o MESMO binário: deduplica pelo caminho resolvido,
    # não pelo nome (senão "cc" e "gcc" contam como dois compiladores).
    real=$(command -v "${cmd%% *}" 2>/dev/null)
    real=$(readlink -f "$real" 2>/dev/null || printf '%s' "$real")
    case " ${VERSOES[*]-} " in
        *"|$real|"*) return 0 ;;
    esac
    VERSOES+=("|$real|")
    COMPILADORES+=("|$cmd|")
    NOMES+=("$nome: $versao")
}

CC_BASE="${CC:-cc}"
registra_compilador "$CC_BASE" "CC=$CC_BASE"
registra_compilador gcc "gcc"
registra_compilador clang "clang"

# zig cc: compilador com frontend Clang empacotado; não é o driver do LLVM.
# Ver docs/10-compiladores.md §2. Vira um wrapper porque o alvo é fixo.
ZIG_BIN=""
if command -v zig > /dev/null 2>&1; then
    ZIG_BIN=$(command -v zig)
elif [ -x /tmp/zig/ziglang/zig ]; then
    ZIG_BIN=/tmp/zig/ziglang/zig
fi
if [ -n "$ZIG_BIN" ]; then
    cat > "$TMP/zigcc" <<EOF
#!/bin/sh
export ZIG_GLOBAL_CACHE_DIR="\${ZIG_GLOBAL_CACHE_DIR:-$TMP/zigcache}"
export ZIG_LOCAL_CACHE_DIR="\${ZIG_LOCAL_CACHE_DIR:-$TMP/zigcache}"
exec "$ZIG_BIN" cc -target x86_64-linux-gnu "\$@"
EOF
    chmod +x "$TMP/zigcc"
    registra_compilador "$TMP/zigcc" "zig cc (frontend Clang)"
fi
[ -n "${T_CC_ALT:-}" ] && registra_compilador "$T_CC_ALT" "T_CC_ALT"

if [ "${#COMPILADORES[@]}" -eq 0 ]; then
    echo "run_portabilidade.sh: nenhum compilador C encontrado" >&2
    exit 2
fi

if [ ! -x "$FERRAMENTA" ]; then
    echo "run_portabilidade.sh: basicgolc nao encontrado em $FERRAMENTA" >&2
    echo "                      rode  make  antes." >&2
    exit 2
fi
if [ ! -f "$BIBLIOTECA" ]; then
    echo "run_portabilidade.sh: biblioteca nao encontrada em $BIBLIOTECA" >&2
    echo "                      rode  make biblioteca  antes." >&2
    exit 2
fi

FLAGS_BASE="-Wall -Wextra -Wformat=2 -Wformat-security -Wvla"
FLAGS_ESTRITAS="$FLAGS_BASE -Wpedantic -Werror"
# Conjunto ADOTADO pelo projeto para código próprio (biblioteca e transpilador).
# É maior que o do Makefile: se algum destes disparar, é para consertar o código.
AVISOS_PROJETO="-std=c11 -Wall -Wextra -Wformat=2 -Wformat-security -Wvla -Wpedantic
 -Wcast-qual -Wshadow -Wstrict-prototypes -Wmissing-prototypes
 -Wmissing-declarations -Wold-style-definition -Wundef -Wwrite-strings
 -Wpointer-arith -Wredundant-decls -Wswitch-enum -Wvla -Wnull-dereference
 -Wstrict-overflow=2 -Wmissing-field-initializers -Werror"

if [ "$QUIETO" = 0 ]; then
    printf 'BASICGOL — portabilidade e compiladores\n'
    printf 'compiladores encontrados:\n'
    for i in "${!NOMES[@]}"; do printf '  [%d] %s\n' "$i" "${NOMES[$i]}"; done
    printf 'basicgolc: %s\nbiblioteca: %s\n' "$FERRAMENTA" "$BIBLIOTECA"
fi

# =============================================================== 1. cabeçalho
cat > "$TMP/h.c" <<'EOF'
#include <basicgol.h>

/* uso real: uma macro de cada família, para o compilador analisar de verdade */
INICIO
    INTEIRO i = 0;
    LOGICO b = VERDADEIRO;
    TEXTO_FIXO(buf, 16);

    SE b E (i < 1) ENTAO
        ESCREVAL("x");
    FIM_SE
    PARA (i, 0, 1) {
        ESCREVA(i);
    }
    ESCREVAL(ABS(-1) + MINIMO(1, 2) + DIVIDE(4, 2));
    ESCREVAL(LEIA_TEXTO(buf, TAMANHO_DE(buf)));
FIM
EOF

titulo "1 — cabeçalho e padrões do C"
for i in "${!COMPILADORES[@]}"; do
    cc="${COMPILADORES[$i]%|}"
    cc="${cc#|}"
    CTAG="c${i}"
    for std in c11 c17 gnu11 c23; do
        if [ "$std" = "c23" ]; then
            # c23/c2x: só entra na matriz se o compilador aceitar o nome do padrão
            if ! tenta "$cc" -std=c23 -fsyntax-only -x c /dev/null > /dev/null; then
                if tenta "$cc" -std=c2x -fsyntax-only -x c /dev/null > /dev/null; then
                    std_efetivo=c2x
                else
                    ignorado "[$i] -std=c23 (padrão não suportado por este compilador)"
                    continue
                fi
            else
                std_efetivo=c23
            fi
        else
            std_efetivo=$std
        fi
        if tenta "$cc" -std=$std_efetivo $FLAGS_ESTRITAS -I "$INCLUDE" \
                 -o "$TMP/h_${i}_$std" "$TMP/h.c" "$BIBLIOTECA" -lm; then
            ok "[$i] cabeçalho e biblioteca compilam limpos em -std=$std_efetivo (-Wpedantic -Werror)"
        else
            falhou "[$i] compilar em -std=$std_efetivo" "$(head -4 "$TMP/e" | tr '\n' ' ')"
        fi
    done

    # C99 e C89 recusados, com a mensagem da linguagem
    printf '#include <basicgol.h>\nint main(void){return 0;}\n' > "$TMP/h99.c"
    if tenta "$cc" -std=c99 -I "$INCLUDE" -o "$TMP/h99" "$TMP/h99.c"; then
        falhou "[$i] C99 deveria ser recusado" "compilou em -std=c99"
    elif grep -q "exige C11" "$TMP/e"; then
        ok "[$i] -std=c99 recusado com mensagem da linguagem"
    else
        falhou "[$i] C99 recusado sem a mensagem esperada" "$(head -2 "$TMP/e" | tr '\n' ' ')"
    fi
    printf '#include <basicgol.h>\nint main(void){return 0;}\n' > "$TMP/h89.c"
    if tenta "$cc" -std=c89 -I "$INCLUDE" -o "$TMP/h89" "$TMP/h89.c"; then
        falhou "[$i] C89 deveria ser recusado" "compilou em -std=c89"
    else
        ok "[$i] -std=c89 recusado"
    fi

    # cada cabeçalho público sozinho (autocontido)
    for h in basicgol.h basicgol/biblioteca.h basicgol/consola.h basicgol/texto.h \
             basicgol/matematica.h basicgol/caractere.h basicgol/utilidades.h; do
        printf '#include <%s>\nint main(void){return 0;}\n' "$h" > "$TMP/so.c"
        if tenta "$cc" -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" -o "$TMP/so" "$TMP/so.c" -lm; then
            ok "[$i] cabeçalho autocontido: <$h>"
        else
            falhou "[$i] cabeçalho não é autocontido: <$h>" "$(head -3 "$TMP/e" | tr '\n' ' ')"
        fi
    done
done

# ==================================== 2. programas de portabilidade, por compilador
titulo "2 — precedência, controle de fluxo e fronteiras, por compilador"
for i in "${!COMPILADORES[@]}"; do
    cc="${COMPILADORES[$i]%|}"
    cc="${cc#|}"
    CTAG="c${i}"

    for prog in precedencia controle fronteira; do
        if ! tenta "$cc" -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" \
                 -o "$TMP/$prog.$i" "$PORT/$prog.c" "$BIBLIOTECA" -lm; then
            falhou "[$i] compilar $prog.c" "$(head -4 "$TMP/e" | tr '\n' ' ')"
            continue
        fi
        if "$TMP/$prog.$i" > "$TMP/$prog.$i.out" 2> "$TMP/$prog.$i.err"; then
            ok "[$i] $prog.c executa e passa"
        else
            falhou "[$i] $prog.c falhou" "$(head -2 "$TMP/$prog.$i.err" | tr '\n' ' ')"
        fi
    done

    # M0 × M1 no mesmo fonte de controle de fluxo
    if "$FERRAMENTA" "$PORT/controle.c" -o "$TMP/controle.gerado.$i.c" 2> "$TMP/e" \
       && tenta "$cc" -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" \
                    -o "$TMP/controle.m1.$i" "$TMP/controle.gerado.$i.c" "$BIBLIOTECA" -lm; then
        "$TMP/controle.m1.$i" > "$TMP/controle.m1.$i.out" 2>&1
        if [ "$(cat "$TMP/controle.m1.$i.out")" = "$(cat "$TMP/controle.$i.out")" ]; then
            ok "[$i] controle.c: M0 e M1 com a mesma saída"
        else
            falhou "[$i] controle.c: M0 e M1 divergem" "M0=$(cat "$TMP/controle.$i.out") M1=$(cat "$TMP/controle.m1.$i.out")"
        fi
    else
        falhou "[$i] controle.c pelo caminho M1" "$(head -3 "$TMP/e" | tr '\n' ' ')"
    fi

    # entrada: contrato de leitura e falhas
    if tenta "$cc" -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" \
             -o "$TMP/entrada.$i" "$PORT/entrada.c" "$BIBLIOTECA" -lm; then
        "$TMP/entrada.$i" < "$PORT/entrada.entrada.txt" > "$TMP/entrada.$i.out" 2> "$TMP/entrada.$i.err"
        if diff -u "$PORT/entrada.esperado.txt" "$TMP/entrada.$i.out" > "$TMP/entrada.$i.diff" 2>&1; then
            ok "[$i] entrada.c: contrato de leitura e falhas de entrada"
        else
            falhou "[$i] entrada.c diverge do dourado" "$(sed -n '1,6p' "$TMP/entrada.$i.diff" | tr '\n' ' ')"
        fi
    else
        falhou "[$i] compilar entrada.c" "$(head -3 "$TMP/e" | tr '\n' ' ')"
    fi

    # duas UOs + BG_SEM_APELIDOS + extensões, ligadas juntas
    if tenta "$cc" -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" -o "$TMP/cab.$i" \
             "$PORT/cabecalhos_tu1.c" "$PORT/cabecalhos_tu2.c" \
             "$PORT/cabecalhos_tu3.c" "$PORT/cabecalhos_principal.c" \
             "$BIBLIOTECA" -lm; then
        if [ "$("$TMP/cab.$i")" = "cabecalhos: ok (7 8 7)" ]; then
            ok "[$i] 4 UOs com inclusão múltipla, BG_SEM_APELIDOS e extensões"
        else
            falhou "[$i] programa multi-UO com saída inesperada" "$("$TMP/cab.$i" 2>&1 | head -2 | tr '\n' ' ')"
        fi
    else
        falhou "[$i] ligar 4 UOs" "$(head -4 "$TMP/e" | tr '\n' ' ')"
    fi

    # símbolos globais definidos pelas UOs do teste: nada além do esperado
    if command -v nm > /dev/null 2>&1; then
        definidos=""
        for uo in tu1 tu2 tu3; do
            tenta "$cc" -std=c11 -I "$INCLUDE" -c -o "$TMP/$uo.o" "$PORT/cabecalhos_$uo.c" || continue
            definidos="$definidos $(nm -g --defined-only "$TMP/$uo.o" 2>/dev/null \
                | awk '$2 == "T" { print $3 }' | tr '\n' ' ')"
        done
        inesperados=""
        for s in $definidos; do
            case "$s" in
                bg_tu1_resultado|bg_tu2_resultado|bg_tu3_resultado) ;;
                *) inesperados="$inesperados $s" ;;
            esac
        done
        if [ -z "$inesperados" ]; then
            ok "[$i] nenhum símbolo global extra definido pelas unidades de teste"
        else
            falhou "[$i] símbolos globais inesperados nos cabeçalhos" "$inesperados"
        fi
    else
        ignorado "[$i] nm ausente: não foi possível auditar símbolos globais"
    fi
done

# ============================================== 3. avisos do código do projeto
titulo "3 — avisos no código do projeto (conjunto maior que o do Makefile)"
for i in "${!COMPILADORES[@]}"; do
    cc="${COMPILADORES[$i]%|}"
    cc="${cc#|}"
    CTAG="c${i}"

    for fonte in "$RAIZ"/src/biblioteca/*.c "$RAIZ/tools/basicgolc.c"; do
        base=$(basename "$fonte")
        if tenta "$cc" $AVISOS_PROJETO -I "$INCLUDE" -c -o "$TMP/aviso.o" "$fonte"; then
            ok "[$i] $base sem avisos no conjunto adotado"
        else
            falhou "[$i] $base emite avisos" "$(grep -c 'warning:' "$TMP/e") aviso(s): $(grep 'warning:' "$TMP/e" | head -2 | sed 's/.*warning: //' | tr '\n' ' ')"
        fi
    done

    # -pedantic nos exemplos do repositório. A especificação (PT-H) recomendava
    # NÃO usar -Wpedantic no caminho M0 "porque pode gerar ruído"; a Fase 6
    # mediu e não gera. Exemplos com açúcar (PARA i DE …) vão pelo caminho M1.
    if true; then
        ruins=""
        total_ex=0
        for ex in "$RAIZ"/docs/exemplos/*.c; do
            total_ex=$((total_ex + 1))
            if tenta "$cc" -std=c11 -pedantic $FLAGS_BASE -Werror -I "$INCLUDE" \
                     -o "$TMP/ex" "$ex" "$BIBLIOTECA" -lm; then
                continue
            fi
            if tenta "$FERRAMENTA" "$ex" -o "$TMP/ex.gerado.c" \
               && tenta "$cc" -std=c11 -pedantic $FLAGS_BASE -Werror -I "$INCLUDE" \
                        -o "$TMP/ex" "$TMP/ex.gerado.c" "$BIBLIOTECA" -lm; then
                continue
            fi
            ruins="$ruins $(basename "$ex" .c)"
        done
        if [ -z "$ruins" ]; then
            ok "[$i] os $total_ex exemplos do repositório compilam limpos com -pedantic -Werror"
        else
            falhou "[$i] exemplos com aviso/erro sob -pedantic" "$ruins"
        fi
    fi
done

# ==================================================== 4. sanitizers
titulo "4 — sanitizers (ASan + UBSan), quando suportados"
SAN_FLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all"
for i in "${!COMPILADORES[@]}"; do
    cc="${COMPILADORES[$i]%|}"
    cc="${cc#|}"
    CTAG="c${i}"

    if ! printf 'int main(void){return 0;}\n' > "$TMP/san.c" \
       || ! tenta "$cc" $SAN_FLAGS -o "$TMP/san" "$TMP/san.c"; then
        ignorado "[$i] sanitizers não suportados por este compilador"
        continue
    fi

    # biblioteca + programas sob sanitizer
    if ! tenta "$cc" -std=c11 $SAN_FLAGS -I "$INCLUDE" -o "$TMP/fronteira.san" \
             "$PORT/fronteira.c" "$RAIZ"/src/biblioteca/*.c -lm; then
        falhou "[$i] compilar fronteira.c com sanitizers" "$(head -3 "$TMP/e" | tr '\n' ' ')"
    else
        if ASAN_OPTIONS=detect_leaks=1 "$TMP/fronteira.san" > "$TMP/san.out" 2> "$TMP/san.err"; then
            ok "[$i] fronteira.c limpa sob ASan+UBSan (com LeakSanitizer)"
        else
            falhou "[$i] fronteira.c sob sanitizer" "$(head -6 "$TMP/san.err" | tr '\n' ' ')"
        fi
    fi

    for prog in precedencia controle entrada; do
        if ! tenta "$cc" -std=c11 $SAN_FLAGS -I "$INCLUDE" -o "$TMP/$prog.san" \
                 "$PORT/$prog.c" "$RAIZ"/src/biblioteca/*.c -lm; then
            falhou "[$i] compilar $prog.c com sanitizers" "$(head -3 "$TMP/e" | tr '\n' ' ')"
            continue
        fi
        if [ "$prog" = "entrada" ]; then
            ASAN_OPTIONS=detect_leaks=1 "$TMP/$prog.san" < "$PORT/entrada.entrada.txt" \
                > "$TMP/$prog.san.out" 2> "$TMP/$prog.san.err"
        else
            ASAN_OPTIONS=detect_leaks=1 "$TMP/$prog.san" > "$TMP/$prog.san.out" 2> "$TMP/$prog.san.err"
        fi
        if [ -s "$TMP/$prog.san.err" ]; then
            falhou "[$i] $prog.c sob sanitizer" "$(head -4 "$TMP/$prog.san.err" | tr '\n' ' ')"
        else
            ok "[$i] $prog.c limpa sob ASan+UBSan"
        fi
    done

    # o transpilador também: ele é C puro e o LeakSanitizer já achou vazamento aqui
    if tenta "$cc" -std=c11 $SAN_FLAGS -o "$TMP/basicgolc.san" "$RAIZ/tools/basicgolc.c"; then
        ASAN_OPTIONS=detect_leaks=1 "$TMP/basicgolc.san" -w "$PORT/controle.c" -o /dev/null \
            > /dev/null 2> "$TMP/bg.san.err"
        if grep -qi "sanitizer\|runtime error" "$TMP/bg.san.err"; then
            falhou "[$i] basicgolc sob sanitizer" "$(head -5 "$TMP/bg.san.err" | tr '\n' ' ')"
        else
            ok "[$i] basicgolc limpo sob ASan+UBSan (sem vazamento)"
        fi
    else
        falhou "[$i] compilar basicgolc com sanitizers" "$(head -3 "$TMP/e" | tr '\n' ' ')"
    fi
done

# ==================================================== 5. C++
titulo "5 — C++ (cabeçalhos de módulo)"
CXX=""
for candidato in "${CXX:-}" g++ clang++ c++; do
    [ -n "$candidato" ] || continue
    command -v "$candidato" > /dev/null 2>&1 && { CXX=$candidato; break; }
done
CTAG="c++"
if [ -z "$CXX" ]; then
    ignorado "nenhum compilador C++ encontrado: teste de C++ ignorado"
else
    if tenta "$CXX" -std=c++17 -Wall -Wextra -Wformat=2 -Werror -I "$INCLUDE" \
             -o "$TMP/cpp" "$PORT/cpp_inclusao.cpp" "$BIBLIOTECA" -lm; then
        if [ "$("$TMP/cpp")" = "cpp: ok" ]; then
            ok "$CXX: cabeçalhos de módulo usáveis de C++ (extern \"C\")"
        else
            falhou "$CXX: programa C++ com saída inesperada" "$("$TMP/cpp" 2>&1 | head -2)"
        fi
    else
        falhou "$CXX: incluindo os cabeçalhos de módulo" "$(head -4 "$TMP/e" | tr '\n' ' ')"
    fi

    # basicgol.h é só C (D-15): tem de falhar, e com a mensagem da linguagem
    printf '#include <basicgol.h>\nint main(){return 0;}\n' > "$TMP/lang.cpp"
    if tenta "$CXX" -std=c++17 -I "$INCLUDE" -c -o "$TMP/lang.o" "$TMP/lang.cpp"; then
        falhou "$CXX: basicgol.h não deveria compilar como C++" "compilou"
    elif grep -q "exige C11" "$TMP/e"; then
        ok "$CXX: basicgol.h recusado em C++ com a mensagem de C11 (D-15)"
    else
        falhou "$CXX: basicgol.h falhou sem a mensagem esperada" "$(head -2 "$TMP/e" | tr '\n' ' ')"
    fi
fi

# ====================================== 6. alvos cruzados (só com zig presente)
titulo "6 — alvos cruzados (zig cc)"
CTAG="cross"
if [ -z "$ZIG_BIN" ]; then
    ignorado "zig ausente: alvos i386/aarch64/windows não testados"
else
    export ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-$TMP/zigcache}"
    export ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-$TMP/zigcache}"
    PORT_C="$PORT/precedencia.c"
    PORT_F="$PORT/fronteira.c"

    # i386 (ILP32): compila E executa (estático, musl) — long/tamanho de 32 bits
    if tenta "$ZIG_BIN" cc -target x86-linux-musl -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" \
             -o "$TMP/fronteira32" "$PORT_F" "$RAIZ"/src/biblioteca/*.c -lm \
       && "$TMP/fronteira32" > "$TMP/f32.out" 2>&1; then
        ok "i386 (x86-linux-musl): fronteira.c compila e passa ($(cat "$TMP/f32.out"))"
    else
        falhou "i386 (x86-linux-musl)" "$(head -4 "$TMP/e" | tr '\n' ' ')"
    fi
    if ! tenta "$ZIG_BIN" cc -target x86-linux-musl -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" \
               -o "$TMP/prec32" "$PORT_C" "$RAIZ"/src/biblioteca/*.c -lm; then
        falhou "i386: compilar precedencia.c" "$(head -4 "$TMP/e" | tr '\n' ' ')"
    elif "$TMP/prec32" > "$TMP/p32.out" 2>&1; then
        ok "i386: precedencia.c compila e passa"
    else
        falhou "i386: precedencia.c falhou ao executar" "$(tail -2 "$TMP/p32.out" | tr '\n' ' ')"
    fi

    # aarch64 (LP64, char não assinado em AArch64 Linux): compila
    for fonte in "$PORT_F" "$RAIZ"/src/biblioteca/*.c; do
        if ! tenta "$ZIG_BIN" cc -target aarch64-linux-gnu -std=c11 $FLAGS_ESTRITAS \
                 -I "$INCLUDE" -c -o "$TMP/a64.o" "$fonte"; then
            falhou "aarch64: compilar $(basename "$fonte")" "$(head -3 "$TMP/e" | tr '\n' ' ')"
            continue 2
        fi
    done
    ok "aarch64 (aarch64-linux-gnu): biblioteca + fronteira.c compilam (sem execução)"

    # x86_64-windows-gnu (MinGW-w64): compila E liga .exe (sem execução)
    if tenta "$ZIG_BIN" cc -target x86_64-windows-gnu -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" \
             -o "$TMP/fronteira.exe" "$PORT_F" "$RAIZ"/src/biblioteca/*.c; then
        ok "x86_64-windows-gnu (MinGW-w64): fronteira.exe compila e liga (sem execução)"
    else
        falhou "x86_64-windows-gnu" "$(head -4 "$TMP/e" | tr '\n' ' ')"
    fi
    if tenta "$ZIG_BIN" cc -target x86_64-windows-gnu -std=c11 $FLAGS_ESTRITAS -I "$INCLUDE" \
             -o "$TMP/ola.exe" "$RAIZ/docs/exemplos/ola_mundo.c" "$RAIZ"/src/biblioteca/*.c; then
        ok "x86_64-windows-gnu: exemplo ola_mundo.exe compila e liga"
    else
        falhou "x86_64-windows-gnu: exemplo" "$(head -4 "$TMP/e" | tr '\n' ' ')"
    fi
    if command -v wine > /dev/null 2>&1 || command -v wine64 > /dev/null 2>&1; then
        ok "wine presente (execução de binários Windows não é feita por este script)"
    else
        ignorado "wine ausente: binários Windows não executados (compilação verificada)"
    fi
fi

# =============================================================== matriz e resumo
mkdir -p "$REL"
{
    printf '# compilador\tresultado\tverificacao\n'
    cat "$MATRIZ"
} > "$REL/portabilidade.tsv"

{
    printf '# BASICGOL — matriz de portabilidade\n'
    printf '# gerado por tests/run_portabilidade.sh em %s\n' "$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
    printf '# máquina: %s\n' "$(uname -srm 2>/dev/null || echo '?')"
    for n in "${NOMES[@]}"; do printf '# %s\n' "$n"; done
    printf '#\n# compilador\tresultado\tverificacao\n'
    cat "$MATRIZ"
} > "$REL/portabilidade.tsv"

printf '\n=============================================\n'
printf 'matriz: %s\n' "$REL/portabilidade.tsv"
if [ "$FALHOU" -eq 0 ]; then
    printf 'PORTABILIDADE OK: %d verificacoes aprovadas, %d ignoradas\n' "$OK" "$IGNORADO"
    exit 0
fi
printf 'FALHAS: %d de %d verificacoes (%d ignoradas)%s\n' \
    "$FALHOU" "$((OK + FALHOU))" "$IGNORADO" "$FALHAS"
exit 1
