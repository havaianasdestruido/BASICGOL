#!/usr/bin/env bash
# run.sh — suíte de testes BASICGOL (Fase 2).
#
#   ./tests/run.sh            roda tudo
#   ./tests/run.sh -v         mostra cada caso aprovado
#   ./tests/run.sh 03_controle  roda só uma suíte (prefixo)
#
# Cada caso de execução é verificado DUAS vezes: no caminho M0 (macros) e no
# caminho M1 (transpilador). Um caso só passa se os dois caminhos produzirem
# exatamente a saída esperada — é assim que a equivalência M0×M1 (AC-32) é
# testada em todo o conjunto, e não em um teste solto.

set -u

# Locale fixo: os casos negativos comparam o texto do diagnóstico, que é do
# compilador (em inglês) e não da linguagem. Sem isto, um ambiente com locale
# UTF-8 faz o GCC trocar ' por ‘ (e um locale traduzido muda a mensagem
# inteira), então a suíte passaria numa máquina e falharia em outra. A
# biblioteca não chama setlocale (D-17); aqui só se fixa o ambiente do runner.
export LC_ALL=C

RAIZ=$(cd "$(dirname "$0")/.." && pwd)
T_CASOS="$RAIZ/tests/casos"
T_INCLUDE="$RAIZ/include"
T_BASICGOLC="${T_BASICGOLC:-$RAIZ/build/basicgolc}"
T_LIBRARY="${T_LIBRARY:-$RAIZ/build/libbasicgol.a}"
T_CC="${CC:-cc}"
T_CFLAGS="${T_CFLAGS:--std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror}"
T_TMP=$(mktemp -d "${TMPDIR:-/tmp}/basicgol-teste.XXXXXX")
T_OK=0
T_FALHOU=0
T_LISTA_FALHAS=""
T_VERBOSO=0
FILTRO="${1:-}"
[ "${1:-}" = "-v" ] && { T_VERBOSO=1; FILTRO="${2:-}"; }

trap 'rm -rf "$T_TMP"' EXIT

# shellcheck source=tests/lib.sh
. "$RAIZ/tests/lib.sh"

if [ ! -x "$T_BASICGOLC" ]; then
    echo "run.sh: basicgolc nao encontrado em $T_BASICGOLC" >&2
    echo "        rode  make  antes dos testes." >&2
    exit 2
fi
if [ ! -f "$T_LIBRARY" ]; then
    echo "run.sh: biblioteca nao encontrada em $T_LIBRARY" >&2
    echo "        rode  make biblioteca  antes dos testes." >&2
    exit 2
fi

selecionado() { [ -z "$FILTRO" ] || case "$1" in "$FILTRO"*) return 0 ;; *) return 1 ;; esac; }

# ===============================================================
# 01 — sanidade do cabeçalho e do transpilador
# ===============================================================
if selecionado 01_sanidade; then
    t_titulo "01 — sanidade (cabeçalho, C11, BG_SEM_APELIDOS, avaliação única)"

    # o cabeçalho compila sozinho, sem avisos
    printf '#include <basicgol.h>\nint main(void) { return 0; }\n' > "$T_TMP/h1.c"
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h1" "$T_TMP/h1.c" 2> "$T_TMP/e"; then
        t_ok "cabecalho compila sozinho"
    else
        t_falhou "cabecalho compila sozinho" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # idempotente
    printf '#include <basicgol.h>\n#include <basicgol.h>\nint main(void){return 0;}\n' > "$T_TMP/h2.c"
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h2" "$T_TMP/h2.c" 2> "$T_TMP/e"; then
        t_ok "cabecalho e idempotente"
    else
        t_falhou "cabecalho e idempotente" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # -pedantic também limpo
    printf '#include <basicgol.h>\nINICIO\n  ESCREVAL("x");\nFIM\n' > "$T_TMP/h3.c"
    if $T_CC -std=c11 -Wall -Wextra -Wpedantic -Werror -I "$T_INCLUDE" \
         -o "$T_TMP/h3" "$T_TMP/h3.c" -lm 2> "$T_TMP/e"; then
        t_ok "compila limpo com -Wpedantic"
    else
        t_falhou "compila limpo com -Wpedantic" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # C99 deve falhar com mensagem clara
    printf '#include <basicgol.h>\nint main(void){return 0;}\n' > "$T_TMP/h4.c"
    if $T_CC -std=c99 -I "$T_INCLUDE" -o "$T_TMP/h4" "$T_TMP/h4.c" 2> "$T_TMP/e"; then
        t_falhou "C99 e rejeitado" "compilou em C99"
    elif grep -q "exige C11" "$T_TMP/e"; then
        t_ok "C99 e rejeitado com mensagem"
    else
        t_falhou "C99 e rejeitado com mensagem" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # AC-22 — as expansões do cabeçalho são exatamente as documentadas em 03 §3.
    # -dM -E normaliza espaços, então a comparação é textual e estável.
    printf '#include <basicgol.h>\n' > "$T_TMP/dm.c"
    if $T_CC -std=c11 -I "$T_INCLUDE" -dM -E "$T_TMP/dm.c" > "$T_TMP/dm.txt" 2> "$T_TMP/e"; then
        divergentes=""
        while IFS= read -r linha; do
            case "$linha" in ''|\#*) continue ;; esac
            nome="${linha%%|*}"
            corpo="${linha#*|}"
            if ! grep -qxF "#define $nome $corpo" "$T_TMP/dm.txt"; then
                divergentes="$divergentes $nome"
            fi
        done < "$RAIZ/tests/expansoes.txt"
        if [ -z "$divergentes" ]; then
            t_ok "expansoes do cabecalho batem com a tabela documentada (AC-22)"
        else
            t_falhou "expansoes do cabecalho batem com a tabela documentada (AC-22)" \
                     "divergentes:$divergentes"
        fi
    else
        t_falhou "expansoes do cabecalho batem com a tabela documentada (AC-22)" \
                 "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # AC-26 — também em C17 (e em clang, quando existir no sistema)
    if $T_CC -std=c17 $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h9" "$T_TMP/h3.c" -lm \
       2> "$T_TMP/e"; then
        t_ok "compila limpo em -std=c17"
    else
        t_falhou "compila limpo em -std=c17" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    if command -v clang > /dev/null 2>&1; then
        if clang $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h10" "$T_TMP/h3.c" -lm \
           2> "$T_TMP/e"; then
            t_ok "compila limpo com clang"
        else
            t_falhou "compila limpo com clang" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
        fi
    else
        t_ok "clang ausente neste ambiente (verificacao ignorada)"
    fi

    # AC-29 — compilar um exemplo pequeno leva menos de 1 s
    tempo_ini=$(date +%s%N)
    if $T_CC -std=c11 -I "$T_INCLUDE" -o "$T_TMP/h11" \
         "$RAIZ/docs/exemplos/ola_mundo.c" -lm 2> "$T_TMP/e"; then
        tempo_fim=$(date +%s%N)
        ms=$(( (tempo_fim - tempo_ini) / 1000000 ))
        if [ "$ms" -lt 1000 ]; then
            t_ok "compilacao de um exemplo abaixo de 1 s (${ms} ms)"
        else
            t_falhou "compilacao de um exemplo abaixo de 1 s" "${ms} ms"
        fi
    else
        t_falhou "compilacao de um exemplo abaixo de 1 s" \
                 "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # BG_SEM_APELIDOS: forma prefixada funciona, nome curto desaparece
    cat > "$T_TMP/h5.c" <<'EOF'
#define BG_SEM_APELIDOS
#include <basicgol.h>
BG_INICIO
    BG_ESCREVAL("prefixado ok");
    BG_SE 1 BG_ENTAO
        BG_ESCREVAL("se ok");
    BG_FIM_SE
BG_FIM
EOF
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h5" "$T_TMP/h5.c" -lm 2> "$T_TMP/e" \
       && [ "$("$T_TMP/h5" | tr '\n' '|')" = "prefixado ok|se ok|" ]; then
        t_ok "BG_SEM_APELIDOS: forma prefixada funciona"
    else
        t_falhou "BG_SEM_APELIDOS: forma prefixada funciona" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    cat > "$T_TMP/h6.c" <<'EOF'
#define BG_SEM_APELIDOS
#include <basicgol.h>
#ifdef SE
#error "nome curto SE nao deveria existir"
#endif
#ifdef ESCREVAL
#error "nome curto ESCREVAL nao deveria existir"
#endif
#ifdef DIVIDE
#error "nome curto DIVIDE nao deveria existir"
#endif
#ifdef LEIA_INTEIRO
#error "nome curto LEIA_INTEIRO nao deveria existir"
#endif
#ifdef NATURAL_CURTO
#error "nome curto NATURAL_CURTO nao deveria existir"
#endif
int main(void) { return 0; }
EOF
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h6" "$T_TMP/h6.c" 2> "$T_TMP/e"; then
        printf '' > "$T_TMP/vazio.txt"
        t_ok "BG_SEM_APELIDOS: nenhum nome curto definido"
    else
        t_falhou "BG_SEM_APELIDOS: nenhum nome curto definido" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # as extensões de compilador ficam desligadas, e os nomes delas somem
    cat > "$T_TMP/h6b.c" <<'EOF'
#include <basicgol.h>
#ifdef BG_TEM_EXTENSOES
#error "BG_TEM_EXTENSOES nao deveria existir sem BG_USAR_EXTENSOES"
#endif
#ifdef SOMA_SEGURA
#error "SOMA_SEGURA nao deveria existir sem BG_USAR_EXTENSOES"
#endif
int main(void) { return 0; }
EOF
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h6b" "$T_TMP/h6b.c" 2> "$T_TMP/e"; then
        t_ok "extensoes ficam desligadas por padrao (D-22)"
    else
        t_falhou "extensoes ficam desligadas por padrao (D-22)" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # versão
    cat > "$T_TMP/h7.c" <<'EOF'
#include <basicgol.h>
_Static_assert(BG_VERSAO_MINIMA(1, 0), "exige 1.0");
INICIO
    ESCREVA(BG_VERSAO_MAIOR); ESCREVA("."); ESCREVA(BG_VERSAO_MENOR);
    ESCREVA("."); ESCREVAL(BG_VERSAO_CORRECAO);
FIM
EOF
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h7" "$T_TMP/h7.c" -lm 2> "$T_TMP/e" \
       && [ "$("$T_TMP/h7")" = "1.1.0" ]; then
        t_ok "macros de versao consistentes"
    else
        t_falhou "macros de versao consistentes" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # avaliação única (P-11): argumentos com efeito colateral
    cat > "$T_TMP/h8.c" <<'EOF'
#include <basicgol.h>
INICIO
    INTEIRO i = 0;
    INTEIRO a;
    INTEIRO b;
    a = ABS(++i);            /* uma avaliacao: i=1, a=1 */
    b = MINIMO(++i, 100);    /* uma avaliacao: i=2, b=2 */
    ESCREVA(i); ESCREVA(" "); ESCREVA(a); ESCREVA(" "); ESCREVAL(b);
    i = 0;
    ESCREVA(i++); ESCREVA(" "); ESCREVAL(i);   /* imprime 0 e depois 1 */
FIM
EOF
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h8" "$T_TMP/h8.c" "$T_LIBRARY" -lm 2> "$T_TMP/e" \
       && [ "$("$T_TMP/h8" | tr '\n' '|')" = "2 1 2|0 1|" ]; then
        t_ok "avaliacao unica de argumentos (P-11)"
    else
        t_falhou "avaliacao unica de argumentos (P-11)" "saida: $("$T_TMP/h8" 2>/dev/null | tr '\n' '|') $(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # avaliação única das macros da Fase 4 (P-11 / RG-2)
    cat > "$T_TMP/h8b.c" <<'EOF'
#include <basicgol.h>
INICIO
    INTEIRO i = 0;
    INTEIRO q;
    LOGICO x;
    q = DIVIDE(++i, 1);            /* uma avaliacao: i=1 */
    x = OU_EXCLUSIVO(++i, 0);      /* uma avaliacao: i=2 */
    ESCREVA(i); ESCREVA(" "); ESCREVA(q); ESCREVA(" ");
    ESCREVAL(x);
FIM
EOF
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/h8b" "$T_TMP/h8b.c" "$T_LIBRARY" -lm 2> "$T_TMP/e" \
       && [ "$("$T_TMP/h8b" | tr '\n' '|')" = "2 1 1|" ]; then
        t_ok "avaliacao unica nas macros novas (DIVIDE, OU_EXCLUSIVO)"
    else
        t_falhou "avaliacao unica nas macros novas (DIVIDE, OU_EXCLUSIVO)" \
                 "saida: $("$T_TMP/h8b" 2>/dev/null | tr '\n' '|') $(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # basicgolc: CLI
    "$T_BASICGOLC" --ajuda > /dev/null 2>&1 && t_ok "basicgolc --ajuda sai 0" \
        || t_falhou "basicgolc --ajuda sai 0"
    "$T_BASICGOLC" --versao | grep -q "1.1.0" && t_ok "basicgolc --versao" \
        || t_falhou "basicgolc --versao"
    "$T_BASICGOLC" > /dev/null 2>&1
    [ $? -eq 2 ] && t_ok "basicgolc sem argumento sai 2" || t_falhou "basicgolc sem argumento sai 2"
    "$T_BASICGOLC" --opcao-que-nao-existe x > /dev/null 2>&1
    [ $? -eq 2 ] && t_ok "basicgolc opcao invalida sai 2" || t_falhou "basicgolc opcao invalida sai 2"
    "$T_BASICGOLC" /caminho/inexistente.c > /dev/null 2>&1
    [ $? -eq 70 ] && t_ok "basicgolc arquivo inexistente falha" || t_falhou "basicgolc arquivo inexistente falha"
fi

# ===============================================================
# 02..08 — casos de execução por diretório
# ===============================================================
for suite in 02_basico 03_controle 04_estruturas 05_entrada_saida 07_multiarquivo 08_aplicacao 09_tipos_expr; do
    selecionado "$suite" || continue
    t_titulo "casos: $suite"
    for dir in "$T_CASOS/$suite"/*/; do
        [ -d "$dir" ] || continue
        dir="${dir%/}"
        caso_execucao "$dir"
    done
done

# ===============================================================
# 06 — casos negativos (devem falhar na compilação, com mensagem prevista)
# ===============================================================
if selecionado 06_negativos; then
    t_titulo "06 — negativos: erros do compilador C (caminho M0)"
    for dir in "$T_CASOS/06_negativos"/*/; do
        [ -d "$dir" ] || continue
        caso_erro_compilacao "${dir%/}"
    done

    t_titulo "06b — negativos: diagnosticos do transpilador (caminho M1)"
    for dir in "$T_CASOS/12_negativos_m1"/*/; do
        [ -d "$dir" ] || continue
        caso_erro_transpilacao "${dir%/}"
    done
fi

# ===============================================================
# 06c — avisos do transpilador (um caso por codigo W-xx)
# ===============================================================
if selecionado 06_avisos; then
    t_titulo "06c — avisos W-01..W-07 (cada um com o seu caso)"
    A="$T_CASOS/13_avisos"
    caso_aviso "$A/w01_atribuicao_em_condicao" W-01
    caso_aviso "$A/w02_sem_inicializador"     W-02
    caso_aviso "$A/w03_condicao_literal"      W-03
    caso_aviso "$A/w04_controle_modificado"   W-04
    caso_aviso "$A/w05_nome_todo_maiusculo"   W-05
    caso_aviso "$A/w06_tamanho_de_parametro"  W-06
    caso_aviso "$A/w07_tamanho_constante"     W-07
fi

# ===============================================================
# 09 — transpilador M1 (tradução, açúcar, acentos, avisos, --checar)
# ===============================================================
if selecionado 09_m1; then
    t_titulo "09 — transpilador M1"

    # 9.1 tradução de cada construtor de bloco
    cat > "$T_TMP/g1.gerado.c" <<'EOF'
#include <basicgol.h>
INICIO
    SE 1 ENTAO
    SENAO_SE 2 ENTAO
    SENAO
    FIM_SE
    ENQUANTO 0 FACA
    FIM_ENQUANTO
    REPITA
    ATE 1;
    ESCOLHA (1)
        CASO 1:
        CONTRARIO:
    FIM_ESCOLHA
    PARA i DE 1 ATE 3 FACA
    FIM_PARA
FIM
EOF
    if "$T_BASICGOLC" "$T_TMP/g1.gerado.c" -o "$T_TMP/g1.gerado.c" -w 2> "$T_TMP/e"; then
        esperados=("if (" ") {" "} else if (" "} else {" "while (" "do {" "} while (!(1));" \
                   "switch (1) {" "case " "default" "int main(void) {" \
                   "for (i = (1); (1) > 0 ? (i <= (3)) : ((1) == 0 ? bg_passo_zero(__FILE__, __LINE__) : (i >= (3))); i += (1)) {")
        faltando=""
        for trecho in "${esperados[@]}"; do
            grep -qF -- "$trecho" "$T_TMP/g1.gerado.c" || faltando="$faltando [$trecho]"
        done
        if [ -z "$faltando" ]; then
            t_ok "M1 traduz todos os construtores de bloco"
        else
            t_falhou "M1 traduz todos os construtores de bloco" "faltando:$faltando"
        fi
    else
        t_falhou "M1 traduz todos os construtores de bloco" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # 9.2 nenhuma palavra-chave de bloco sobra no C gerado (RG-10 / AC-31)
    if grep -Eq '\b(SE|ENTAO|SENAO_SE|SENAO|FIM_SE|ENQUANTO|FACA|FIM_ENQUANTO|REPITA|ESCOLHA|CASO|CONTRARIO|FIM_ESCOLHA|PARE|PROSSIGA|INICIO|FIM|FIM_PARA|ATE)\b' "$T_TMP/g1.gerado.c"; then
        t_falhou "C gerado nao contem palavras-chave de bloco" "$(grep -Eo '\b(SE|ENTAO|SENAO|FIM_SE|ENQUANTO|FACA|REPITA|ESCOLHA|CASO|CONTRARIO|PARE|PROSSIGA|INICIO|FIM|FIM_PARA|ATE)\b' "$T_TMP/g1.gerado.c" | sort -u | tr '\n' ' ')"
    else
        t_ok "C gerado nao contem palavras-chave de bloco"
    fi

    # 9.3 apelidos acentuados (D-03)
    cat > "$T_TMP/g2.c" <<'EOF'
#include <basicgol.h>
INÍCIO
    LÓGICO pronto = FALSO;
    INTEIRO n = 0;
    ENQUANTO NÃO pronto FAÇA
        n = n + 1;
        SE n >= 2 ENTÃO
            pronto = VERDADEIRO;
        SENÃO
            pronto = FALSO;
        FIM_SE
    FIM_ENQUANTO
    ESCREVAL(n);
FIM
EOF
    if "$T_BASICGOLC" "$T_TMP/g2.c" -o "$T_TMP/g2.gerado.c" -w 2> "$T_TMP/e" \
       && $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/g2" "$T_TMP/g2.gerado.c" -lm 2>> "$T_TMP/e" \
       && [ "$("$T_TMP/g2")" = "2" ]; then
        t_ok "M1 aceita palavras-chave acentuadas"
    else
        t_falhou "M1 aceita palavras-chave acentuadas" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # 9.4 --checar: arquivo bom e arquivo com bloco sem fechamento
    cat > "$T_TMP/g3.c" <<'EOF'
#include <basicgol.h>
INICIO
    SE 1 ENTAO
    FIM_SE
    ENQUANTO 0 FACA
    FIM_ENQUANTO
FIM
EOF
    "$T_BASICGOLC" --checar "$T_TMP/g3.c" > /dev/null 2>&1 \
        && t_ok "M1 --checar aprova arquivo bem formado" \
        || t_falhou "M1 --checar aprova arquivo bem formado"

    cat > "$T_TMP/g4.c" <<'EOF'
#include <basicgol.h>
INICIO
    SE 1 ENTAO
        ESCREVAL("sem fim");
EOF
    if "$T_BASICGOLC" --checar "$T_TMP/g4.c" > /dev/null 2> "$T_TMP/e"; then
        t_falhou "M1 --checar reprova bloco sem fechamento" "saiu 0"
    elif grep -q "nao foi fechado" "$T_TMP/e"; then
        t_ok "M1 --checar reprova bloco sem fechamento"
    else
        t_falhou "M1 --checar reprova bloco sem fechamento" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # 9.5 fechamento errado
    cat > "$T_TMP/g5.c" <<'EOF'
#include <basicgol.h>
INICIO
    ENQUANTO 0 FACA
    FIM_SE
FIM
EOF
    if "$T_BASICGOLC" --checar "$T_TMP/g5.c" > /dev/null 2> "$T_TMP/e"; then
        t_falhou "M1 detecta fechamento errado" "saiu 0"
    elif grep -q "fecha o bloco errado" "$T_TMP/e"; then
        t_ok "M1 detecta fechamento errado"
    else
        t_falhou "M1 detecta fechamento errado" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # 9.6 ATE sem REPITA
    cat > "$T_TMP/g6.c" <<'EOF'
#include <basicgol.h>
INICIO
    ATE 1;
FIM
EOF
    if "$T_BASICGOLC" "$T_TMP/g6.c" -o /dev/null > /dev/null 2> "$T_TMP/e"; then
        t_falhou "M1 rejeita ATE sem REPITA" "saiu 0"
    elif grep -q "sem 'REPITA'" "$T_TMP/e"; then
        t_ok "M1 rejeita ATE sem REPITA"
    else
        t_falhou "M1 rejeita ATE sem REPITA" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # 9.7 cabecalho de PARA incompleto
    printf '#include <basicgol.h>\nINICIO\n  PARA i DE 1 FACA\n  FIM_PARA\nFIM\n' > "$T_TMP/g7.c"
    "$T_BASICGOLC" "$T_TMP/g7.c" -o /dev/null > /dev/null 2> "$T_TMP/e" \
        && t_falhou "M1 rejeita PARA sem ATE" "saiu 0" \
        || { grep -q "faltou 'ATE'" "$T_TMP/e" && t_ok "M1 rejeita PARA sem ATE" \
             || t_falhou "M1 rejeita PARA sem ATE" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"; }

    printf '#include <basicgol.h>\nINICIO\n  PARA i 1 ATE 2 FACA\n  FIM_PARA\nFIM\n' > "$T_TMP/g8.c"
    "$T_BASICGOLC" "$T_TMP/g8.c" -o /dev/null > /dev/null 2> "$T_TMP/e" \
        && t_falhou "M1 rejeita PARA sem DE" "saiu 0" \
        || { grep -q "esperado 'DE'" "$T_TMP/e" && t_ok "M1 rejeita PARA sem DE" \
             || t_falhou "M1 rejeita PARA sem DE" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"; }

    # 9.8 avisos
    cat > "$T_TMP/w1.c" <<'EOF'
#include <basicgol.h>
INICIO
    INTEIRO x = 0;
    SE x = 1 ENTAO
        ESCREVAL("a");
    FIM_SE
FIM
EOF
    "$T_BASICGOLC" "$T_TMP/w1.c" -o /dev/null 2> "$T_TMP/e"
    grep -q "\[W-01\]" "$T_TMP/e" && t_ok "M1 emite W-01 (= em condicao)" \
        || t_falhou "M1 emite W-01 (= em condicao)"

    printf '#include <basicgol.h>\nINICIO\n  INTEIRO y;\n  y = 1;\n  ESCREVAL(y);\nFIM\n' > "$T_TMP/w2.c"
    "$T_BASICGOLC" "$T_TMP/w2.c" -o /dev/null 2> "$T_TMP/e"
    grep -q "\[W-02\]" "$T_TMP/e" && t_ok "M1 emite W-02 (sem inicializador)" \
        || t_falhou "M1 emite W-02 (sem inicializador)"

    printf '#include <basicgol.h>\nINICIO\n  INTEIRO TOTAL = 0;\n  ESCREVAL(TOTAL);\nFIM\n' > "$T_TMP/w5.c"
    "$T_BASICGOLC" "$T_TMP/w5.c" -o /dev/null 2> "$T_TMP/e"
    grep -q "\[W-05\]" "$T_TMP/e" && t_ok "M1 emite W-05 (identificador maiusculo)" \
        || t_falhou "M1 emite W-05 (identificador maiusculo)"

    cat > "$T_TMP/w7.c" <<'EOF'
#include <basicgol.h>
CONSTANTE INTEIRO k_n = 4;
INICIO
    VETOR(INTEIRO, v, k_n);
    v[0] = 1;
    ESCREVAL(v[0]);
FIM
EOF
    "$T_BASICGOLC" "$T_TMP/w7.c" -o /dev/null 2> "$T_TMP/e"
    grep -q "\[W-07\]" "$T_TMP/e" && t_ok "M1 emite W-07 (tamanho de vetor com CONSTANTE)" \
        || t_falhou "M1 emite W-07 (tamanho de vetor com CONSTANTE)"

    # -w suprime
    "$T_BASICGOLC" "$T_TMP/w1.c" -o /dev/null -w 2> "$T_TMP/e"
    if [ -s "$T_TMP/e" ]; then
        t_falhou "M1 -w suprime avisos" "$(head -1 "$T_TMP/e")"
    else
        t_ok "M1 -w suprime avisos"
    fi

    # 9.9 avisos não são erros: sai 0
    "$T_BASICGOLC" "$T_TMP/w1.c" -o /dev/null 2> /dev/null
    [ $? -eq 0 ] && t_ok "M1 avisos nao mudam o codigo de saida" \
        || t_falhou "M1 avisos nao mudam o codigo de saida"
fi

# ===============================================================
# 10 — equivalência M0 × M1 nos exemplos do repositório
# ===============================================================
if selecionado 10_exemplos; then
    t_titulo "10 — exemplos do repositorio (M0 e M1 contra a saida dourada)"
    EX="$RAIZ/docs/exemplos"

    for arq in "$EX"/*.c; do
        nome=$(basename "$arq" .c)
        entrada="/dev/null"
        [ -f "$EX/$nome.entrada.txt" ] && entrada="$EX/$nome.entrada.txt"
        bin0="$T_TMP/ex_m0_$nome"
        bin1="$T_TMP/ex_m1_$nome"
        ok_m0=0
        ok_m1=0

        # caminho M0 (todos, menos os exemplos que so existem em M1: acucar PARA i DE)
        if [ "$nome" != "para_contado" ] && [ "$nome" != "controle_para_acucar" ]; then
            if $T_CC $T_CFLAGS -I "$T_INCLUDE" "$arq" -o "$bin0" "$T_LIBRARY" -lm 2> "$T_TMP/e"; then
                ok_m0=1
            else
                t_falhou "exemplo $nome (M0)" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
            fi
        fi

        # caminho M1 (todos)
        if "$T_BASICGOLC" "$arq" -o "$T_TMP/$nome.gerado.c" 2> "$T_TMP/e" \
           && $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$bin1" "$T_TMP/$nome.gerado.c" "$T_LIBRARY" -lm 2>> "$T_TMP/e"; then
            ok_m1=1
        else
            t_falhou "exemplo $nome (M1)" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
        fi

        # confere um binario contra a saida dourada do exemplo
        confere() {
            local bin="$1" rotulo="$2"
            local saida="$T_TMP/o_$2" erros="$T_TMP/e_$2"
            local rc linha falhou=0

            "$bin" < "$entrada" > "$saida" 2> "$erros"
            rc=$?
            if [ "$rc" != "0" ]; then
                t_falhou "exemplo $nome ($rotulo)" "codigo de saida $rc"
                return
            fi
            if [ -f "$EX/$nome.esperado.txt" ] && difere "$EX/$nome.esperado.txt" "$saida"; then
                t_falhou "exemplo $nome ($rotulo)" "saida difere do dourado:"
                diff -u "$EX/$nome.esperado.txt" "$saida" | sed -n '1,10p' | sed 's/^/        /'
                falhou=1
            fi
            if [ -f "$EX/$nome.contem.txt" ]; then
                while IFS= read -r linha; do
                    [ -z "$linha" ] && continue
                    grep -qF "$linha" "$saida" || {
                        t_falhou "exemplo $nome ($rotulo)" "saida nao contem: $linha"
                        falhou=1
                    }
                done < "$EX/$nome.contem.txt"
            fi
            if [ -f "$EX/$nome.esperado_stderr.txt" ]; then
                grep -qF "$(cat "$EX/$nome.esperado_stderr.txt")" "$erros" || {
                    t_falhou "exemplo $nome ($rotulo)" "stderr nao contem a linha esperada"
                    falhou=1
                }
            fi
            [ "$falhou" = 0 ] || return
            t_ok "exemplo $nome ($rotulo)"
        }

        [ "$ok_m0" = 1 ] && confere "$bin0" "M0"
        [ "$ok_m1" = 1 ] && confere "$bin1" "M1"
    done
fi

# ===============================================================
# 11 — estresse (gerado aqui mesmo, determinístico)
# ===============================================================
if selecionado 11_estresse; then
    t_titulo "11 — estresse"

    # 11.1 aninhamento profundo de SE (40 níveis)
    {
        echo '#include <basicgol.h>'
        echo 'INICIO'
        echo '    INTEIRO n = 0;'
        i=1
        while [ $i -le 40 ]; do
            printf '    SE n >= 0 ENTAO\n'
            i=$((i + 1))
        done
        echo '    n = n + 1;'
        i=1
        while [ $i -le 40 ]; do
            printf '    FIM_SE\n'
            i=$((i + 1))
        done
        echo '    ESCREVAL(n);'
        echo 'FIM'
    } > "$T_TMP/deep.c"
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" "$T_TMP/deep.c" -o "$T_TMP/deep0" -lm 2> "$T_TMP/e" \
       && "$T_BASICGOLC" "$T_TMP/deep.c" -o "$T_TMP/deep.gerado.c" 2>> "$T_TMP/e" \
       && $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/deep1" "$T_TMP/deep.c" -lm 2>> "$T_TMP/e" \
       && [ "$("$T_TMP/deep0")" = "1" ] && [ "$("$T_TMP/deep1")" = "1" ]; then
        t_ok "40 niveis de SE aninhados (M0 e M1)"
    else
        t_falhou "40 niveis de SE aninhados (M0 e M1)" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # 11.2 20 níveis de ENQUANTO + PARE
    {
        echo '#include <basicgol.h>'
        echo 'INICIO'
        echo '    INTEIRO i = 0;'
        i=1
        while [ $i -le 20 ]; do printf '    ENQUANTO i < 100 FACA\n'; i=$((i + 1)); done
        echo '    i = i + 1;'
        i=1
        while [ $i -le 20 ]; do printf '    FIM_ENQUANTO\n'; i=$((i + 1)); done
        echo '    ESCREVAL(i);'
        echo 'FIM'
    } > "$T_TMP/deepw.c"
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" "$T_TMP/deepw.c" -o "$T_TMP/deepw0" -lm 2> "$T_TMP/e" \
       && [ "$("$T_TMP/deepw0")" = "100" ]; then
        t_ok "20 niveis de ENQUANTO aninhados"
    else
        t_falhou "20 niveis de ENQUANTO aninhados" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # 11.3 300 funções + 5.000 iterações
    {
        echo '#include <basicgol.h>'
        i=1
        while [ $i -le 300 ]; do
            printf 'FUNCAO INTEIRO f%d(INTEIRO x) { RETORNE x + %d; }\n' "$i" "$i"
            i=$((i + 1))
        done
        echo 'INICIO'
        echo '    INTEIRO64 total = 0;'
        echo '    INTEIRO i = 0;'
        echo '    PARA (i, 0, 4999) {'
        echo '        total = total + f1(i);'
        echo '    }'
        echo '    ESCREVAL(total);'
        echo '    ESCREVAL(f300(0));'
        echo 'FIM'
    } > "$T_TMP/many.c"
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" "$T_TMP/many.c" -o "$T_TMP/many0" -lm 2> "$T_TMP/e" \
       && "$T_BASICGOLC" "$T_TMP/many.c" -o "$T_TMP/many.gerado.c" 2>> "$T_TMP/e" \
       && $T_CC $T_CFLAGS -I "$T_INCLUDE" -o "$T_TMP/many1" "$T_TMP/many.c" -lm 2>> "$T_TMP/e" \
       && [ "$("$T_TMP/many0")" = "$("$T_TMP/many1")" ]; then
        t_ok "300 funcoes + 5000 iteracoes (M0 == M1)"
    else
        t_falhou "300 funcoes + 5000 iteracoes" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi

    # 11.4 literal de texto grande e muitos ESCREVA
    {
        echo '#include <basicgol.h>'
        echo 'INICIO'
        printf '    ESCREVA("'
        i=0
        while [ $i -lt 400 ]; do printf 'abcdefghij'; i=$((i + 1)); done
        printf '");'
        echo ''
        echo '    ESCREVA_LINHA();'
        echo 'FIM'
    } > "$T_TMP/grande.c"
    if $T_CC $T_CFLAGS -I "$T_INCLUDE" "$T_TMP/grande.c" -o "$T_TMP/grande0" -lm 2> "$T_TMP/e" \
       && [ "$("$T_TMP/grande0" | wc -c)" -eq 4001 ]; then
        t_ok "literal de 4000 caracteres"
    else
        t_falhou "literal de 4000 caracteres" "$(head -2 "$T_TMP/e" | tr '\n' ' ')"
    fi
fi

# ===============================================================
# Resumo
# ===============================================================
TOTAL=$((T_OK + T_FALHOU))
printf '\n=============================================\n'
if [ "$T_FALHOU" -eq 0 ]; then
    printf 'TUDO OK: %d verificacoes aprovadas\n' "$T_OK"
    exit 0
fi
printf 'FALHAS: %d de %d verificacoes%s\n' "$T_FALHOU" "$TOTAL" "$T_LISTA_FALHAS"
exit 1
