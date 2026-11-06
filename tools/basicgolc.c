/*
 * basicgolc — transpilador de tokens BASICGOL -> C (caminho M1).
 *
 * Não é um compilador: não analisa tipos, não resolve nomes, não valida
 * semântica. Ele faz exatamente quatro coisas, definidas na especificação
 * (docs/01-especificacao.md §4.2):
 *
 *   1. troca palavras-chave pelos tokens C correspondentes;
 *   2. insere { } onde as palavras-chave de bloco os exigem;
 *   3. converte a forma açucarada do laço contado (PARA i DE a ATE b [PASSO p]
 *      FACA) em um for C equivalente, e ATE cond; em while(!(cond));
 *   4. emite diagnósticos (W-xx) e erros de estrutura de bloco.
 *
 * Construtores de VALOR (E/S, matemática, texto) são emitidos na forma
 * prefixada (BG_ESCREVAL, ...), definida pelo cabeçalho: assim a tabela de
 * formatos existe em um único lugar. Construtores de BLOCO viram C puro.
 *
 * Uso:
 *   basicgolc programa.c -o programa.transpilado.c
 *   basicgolc --checar programa.c
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BG_VERSAO "1.1.0"

/* Sem o atributo de formato, -Wformat=2 (logo -Wformat-nonliteral) faz o
 * clang recusar o vfprintf das funções de diagnóstico com -Werror. O
 * atributo é opcional no padrão, então fica atrás de #if. */
#if defined(__GNUC__) || defined(__clang__)
#  define BG_FORMATO(f, a) __attribute__((format(printf, f, a)))
#else
#  define BG_FORMATO(f, a)
#endif

/* =====================================================================
 * Buffer de texto dinâmico
 * ===================================================================== */

typedef struct {
    char  *dados;
    size_t tam;
    size_t cap;
} Buf;

static void bg_morte(const char *fmt, ...) BG_FORMATO(1, 2);

static void bg_morte(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fputs("basicgolc: erro fatal: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(70);
}

static void buf_ini(Buf *b)
{
    b->cap = 256;
    b->tam = 0;
    b->dados = (char *) malloc(b->cap);
    if (b->dados == NULL) {
        bg_morte("sem memoria");
    }
    b->dados[0] = '\0';
}

static void buf_adiciona(Buf *b, const char *s)
{
    size_t n = strlen(s);
    if (b->tam + n + 1 > b->cap) {
        while (b->tam + n + 1 > b->cap) {
            b->cap *= 2;
        }
        b->dados = (char *) realloc(b->dados, b->cap);
        if (b->dados == NULL) {
            bg_morte("sem memoria");
        }
    }
    memcpy(b->dados + b->tam, s, n);
    b->tam += n;
    b->dados[b->tam] = '\0';
}

/* Emite texto de traducao garantindo que ele nao cole no token seguinte
 * (ex.: "MOD" seguido de "=" nao pode virar "%="). */
static void emite_seguro(Buf *b, const char *txt);

static void buf_limpa(Buf *b)
{
    b->tam = 0;
    b->dados[0] = '\0';
}

static void buf_libera(Buf *b)
{
    free(b->dados);
    b->dados = NULL;
    b->tam = 0;
    b->cap = 0;
}

static void emite_seguro(Buf *b, const char *txt)
{
    size_t n = strlen(txt);
    if (n == 0) {
        return;
    }
    buf_adiciona(b, txt);
    if (txt[n - 1] != ' ' && txt[n - 1] != '\n' && txt[n - 1] != '\t') {
        buf_adiciona(b, " ");
    }
}

/* =====================================================================
 * Tokens
 * ===================================================================== */

typedef enum {
    TK_IDENT,
    TK_NUM,
    TK_TEXTO,      /* literal de texto "..."   */
    TK_CARACTERE,  /* literal de caractere '...' */
    TK_PONT,       /* pontuação / operador      */
    TK_ESPACO,     /* espaços e quebras de linha */
    TK_COMENTARIO,
    TK_DIRETIVA    /* linha de pre-processador  */
} TipoToken;

typedef struct {
    TipoToken tipo;
    char     *texto;
    int       linha;
} Token;

typedef struct {
    Token *itens;
    int    qtd;
    int    cap;
} Lista;

static Token *lista_novo(Lista *l)
{
    if (l->qtd == l->cap) {
        l->cap = (l->cap == 0) ? 256 : l->cap * 2;
        l->itens = (Token *) realloc(l->itens, (size_t) l->cap * sizeof(Token));
        if (l->itens == NULL) {
            bg_morte("sem memoria");
        }
    }
    return &l->itens[l->qtd++];
}

static void lista_adiciona(Lista *l, TipoToken tipo, const char *ini, size_t n, int linha)
{
    Token *t = lista_novo(l);
    t->tipo = tipo;
    t->texto = (char *) malloc(n + 1);
    if (t->texto == NULL) {
        bg_morte("sem memoria");
    }
    memcpy(t->texto, ini, n);
    t->texto[n] = '\0';
    t->linha = linha;
}

/* Libera os tokens e o vetor. O transpilador é um processo curto, mas sem isto
 * o LeakSanitizer acusa vazamento em toda execução e deixa de servir como
 * sinal: a suíte inteira roda limpa em `make test-sanitize`. */
static void lista_libera(Lista *l)
{
    int i;
    for (i = 0; i < l->qtd; i++) {
        free(l->itens[i].texto);
    }
    free(l->itens);
    l->itens = NULL;
    l->qtd = 0;
    l->cap = 0;
}

/* =====================================================================
 * Leitura do arquivo
 * ===================================================================== */

static char *le_arquivo(const char *caminho)
{
    FILE *f = fopen(caminho, "rb");
    char *dados;
    long  n;

    if (f == NULL) {
        bg_morte("nao consegui abrir '%s'", caminho);
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        bg_morte("nao consegui medir '%s'", caminho);
    }
    n = ftell(f);
    if (n < 0) {
        bg_morte("nao consegui medir '%s'", caminho);
    }
    rewind(f);

    dados = (char *) malloc((size_t) n + 1);
    if (dados == NULL) {
        bg_morte("sem memoria para '%s'", caminho);
    }
    if (fread(dados, 1, (size_t) n, f) != (size_t) n) {
        bg_morte("nao consegui ler '%s'", caminho);
    }
    dados[n] = '\0';
    fclose(f);
    return dados;
}

/* =====================================================================
 * Analisador léxico
 *
 * Regras: comentários, literais e diretivas são copiados sem interpretação;
 * identificadores podem conter bytes UTF-8 (para os apelidos acentuados).
 * ===================================================================== */

static bool byte_ident(unsigned char c)
{
    return (c >= 0x80u) || isalnum(c) || c == '_';
}

static bool inicio_ident(unsigned char c)
{
    return (c >= 0x80u) || isalpha(c) || c == '_';
}

static const char *OPERADORES[] = {
    "<<=", ">>=",
    "==", "!=", "<=", ">=", "&&", "||", "++", "--", "+=", "-=", "*=", "/=",
    "%=", "&=", "|=", "^=", "->", "<<", ">>", "##",
    NULL
};

static void tokeniza(const char *src, Lista *l)
{
    size_t i = 0;
    int linha = 1;

    while (src[i] != '\0') {
        unsigned char c = (unsigned char) src[i];

        /* espaços */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            size_t ini = i;
            while (src[i] == ' ' || src[i] == '\t' || src[i] == '\r' || src[i] == '\n') {
                if (src[i] == '\n') {
                    linha++;
                }
                i++;
            }
            lista_adiciona(l, TK_ESPACO, src + ini, i - ini, linha);
            continue;
        }

        /* comentário de linha */
        if (src[i] == '/' && src[i + 1] == '/') {
            size_t ini = i;
            while (src[i] != '\0' && src[i] != '\n') {
                i++;
            }
            lista_adiciona(l, TK_COMENTARIO, src + ini, i - ini, linha);
            continue;
        }

        /* comentário de bloco */
        if (src[i] == '/' && src[i + 1] == '*') {
            size_t ini = i;
            int linha_ini = linha;
            i += 2;
            while (src[i] != '\0' && !(src[i] == '*' && src[i + 1] == '/')) {
                if (src[i] == '\n') {
                    linha++;
                }
                i++;
            }
            if (src[i] != '\0') {
                i += 2;
            }
            lista_adiciona(l, TK_COMENTARIO, src + ini, i - ini, linha_ini);
            continue;
        }

        /* diretiva de pre-processador: vai até o fim da linha lógica */
        if (src[i] == '#' && (i == 0 || src[i - 1] == '\n')) {
            size_t ini = i;
            int linha_ini = linha;
            while (src[i] != '\0') {
                if (src[i] == '\\' && src[i + 1] == '\n') {
                    i += 2;
                    linha++;
                    continue;
                }
                if (src[i] == '\n') {
                    break;
                }
                i++;
            }
            lista_adiciona(l, TK_DIRETIVA, src + ini, i - ini, linha_ini);
            continue;
        }

        /* literal de texto */
        if (src[i] == '"') {
            size_t ini = i;
            int linha_ini = linha;
            i++;
            while (src[i] != '\0' && src[i] != '"') {
                if (src[i] == '\\' && src[i + 1] != '\0') {
                    i += 2;
                    continue;
                }
                if (src[i] == '\n') {
                    linha++;
                }
                i++;
            }
            if (src[i] == '"') {
                i++;
            }
            lista_adiciona(l, TK_TEXTO, src + ini, i - ini, linha_ini);
            continue;
        }

        /* literal de caractere */
        if (src[i] == '\'') {
            size_t ini = i;
            i++;
            while (src[i] != '\0' && src[i] != '\'') {
                if (src[i] == '\\' && src[i + 1] != '\0') {
                    i += 2;
                    continue;
                }
                i++;
            }
            if (src[i] == '\'') {
                i++;
            }
            lista_adiciona(l, TK_CARACTERE, src + ini, i - ini, linha);
            continue;
        }

        /* identificador */
        if (inicio_ident(c)) {
            size_t ini = i;
            while (src[i] != '\0' && byte_ident((unsigned char) src[i])) {
                i++;
            }
            lista_adiciona(l, TK_IDENT, src + ini, i - ini, linha);
            continue;
        }

        /* número */
        if (isdigit(c)) {
            size_t ini = i;
            while (src[i] != '\0') {
                unsigned char d = (unsigned char) src[i];
                if (isalnum(d) || d == '.' || d == '_' || d == '\'') {
                    i++;
                    continue;
                }
                break;
            }
            lista_adiciona(l, TK_NUM, src + ini, i - ini, linha);
            continue;
        }

        /* operadores de vários caracteres */
        {
            size_t k;
            bool achou = false;
            for (k = 0; OPERADORES[k] != NULL; k++) {
                size_t n = strlen(OPERADORES[k]);
                if (strncmp(src + i, OPERADORES[k], n) == 0) {
                    lista_adiciona(l, TK_PONT, src + i, n, linha);
                    i += n;
                    achou = true;
                    break;
                }
            }
            if (achou) {
                continue;
            }
        }

        /* pontuação de um caractere */
        lista_adiciona(l, TK_PONT, src + i, 1, linha);
        i++;
    }
}

/* =====================================================================
 * Tabelas de tradução
 * ===================================================================== */

/* 1. Apelidos acentuados -> forma ASCII (só no caminho M1; ver D-03). */
typedef struct {
    const char *acentuado;
    const char *ascii;
} Apelido;

static const Apelido APELIDOS[] = {
    { "INÍCIO",      "INICIO" },
    { "ENTÃO",       "ENTAO" },
    { "SENÃO_SE",    "SENAO_SE" },
    { "SENÃO",       "SENAO" },
    { "FAÇA",        "FACA" },
    { "ENQUANTO",    "ENQUANTO" },
    { "ATÉ_QUE",     "ATE_QUE" },
    { "ATÉ",         "ATE" },
    { "NÃO",         "NAO" },
    { "LÓGICO",      "LOGICO" },
    { "FUNÇÃO",      "FUNCAO" },
    { "PROCEDIMENTO","PROCEDIMENTO" },
    { "CONTRÁRIO",   "CONTRARIO" },
    { NULL, NULL }
};

/* 2. Palavras-chave de bloco/estrutura -> C puro (chaves explícitas). */
typedef struct {
    const char *bg;
    const char *c;
    int         abre;   /* tipo de bloco que abre (0 = nenhum) */
    int         fecha;  /* tipo de bloco que fecha (0 = nenhum) */
} Traducao;

enum {
    BLOCO_NENHUM = 0,
    BLOCO_INICIO,
    BLOCO_SE,
    BLOCO_ENQUANTO,
    BLOCO_REPITA,
    BLOCO_ESCOLHA,
    BLOCO_PARA,
    BLOCO_REGISTRO,
    BLOCO_ENUMERACAO
};

static const Traducao TRADUCOES[] = {
    /* programa */
    { "INICIO",     "int main(void) {", BLOCO_INICIO,      BLOCO_NENHUM },
    { "FIM",        "}",                BLOCO_NENHUM,       BLOCO_INICIO },
    /* tipos */
    { "LOGICO",     "bool",             BLOCO_NENHUM,       BLOCO_NENHUM },
    { "CARACTERE",  "char",             BLOCO_NENHUM,       BLOCO_NENHUM },
    { "INTEIRO_CURTO", "short",         BLOCO_NENHUM,       BLOCO_NENHUM },
    { "INTEIRO_LONGO", "long",          BLOCO_NENHUM,       BLOCO_NENHUM },
    { "INTEIRO8",   "int8_t",           BLOCO_NENHUM,       BLOCO_NENHUM },
    { "INTEIRO16",  "int16_t",          BLOCO_NENHUM,       BLOCO_NENHUM },
    { "INTEIRO32",  "int32_t",          BLOCO_NENHUM,       BLOCO_NENHUM },
    { "INTEIRO64",  "int64_t",          BLOCO_NENHUM,       BLOCO_NENHUM },
    { "INTEIRO",    "int",              BLOCO_NENHUM,       BLOCO_NENHUM },
    { "NATURAL_CURTO", "unsigned short", BLOCO_NENHUM,      BLOCO_NENHUM },
    { "NATURAL_LONGO", "unsigned long",  BLOCO_NENHUM,      BLOCO_NENHUM },
    { "NATURAL8",   "uint8_t",          BLOCO_NENHUM,       BLOCO_NENHUM },
    { "NATURAL16",  "uint16_t",         BLOCO_NENHUM,       BLOCO_NENHUM },
    { "NATURAL32",  "uint32_t",         BLOCO_NENHUM,       BLOCO_NENHUM },
    { "NATURAL64",  "uint64_t",         BLOCO_NENHUM,       BLOCO_NENHUM },
    { "NATURAL",    "unsigned",         BLOCO_NENHUM,       BLOCO_NENHUM },
    { "REAL_CURTO", "float",            BLOCO_NENHUM,       BLOCO_NENHUM },
    { "REAL_LONGO", "long double",      BLOCO_NENHUM,       BLOCO_NENHUM },
    { "REAL",       "double",           BLOCO_NENHUM,       BLOCO_NENHUM },
    { "TEXTO",      "const char *",     BLOCO_NENHUM,       BLOCO_NENHUM },
    { "VAZIO",      "void",             BLOCO_NENHUM,       BLOCO_NENHUM },
    { "CONSTANTE",  "static const",     BLOCO_NENHUM,       BLOCO_NENHUM },
    { "ESTATICO",   "static",           BLOCO_NENHUM,       BLOCO_NENHUM },
    { "EXTERNO",    "extern",           BLOCO_NENHUM,       BLOCO_NENHUM },
    /* funções */
    { "FUNCAO",     "",                 BLOCO_NENHUM,       BLOCO_NENHUM },
    { "PROCEDIMENTO", "void",           BLOCO_NENHUM,       BLOCO_NENHUM },
    { "RETORNE",    "return",           BLOCO_NENHUM,       BLOCO_NENHUM },
    /* condicional */
    { "SE",         "if (",             BLOCO_SE,          BLOCO_NENHUM },
    { "ENTAO",      ") {",              BLOCO_NENHUM,       BLOCO_NENHUM },
    { "SENAO_SE",   "} else if (",      BLOCO_NENHUM,       BLOCO_NENHUM },
    { "SENAO",      "} else {",         BLOCO_NENHUM,       BLOCO_NENHUM },
    { "FIM_SE",     "}",                BLOCO_NENHUM,       BLOCO_SE },
    /* laços */
    { "ENQUANTO",   "while (",          BLOCO_ENQUANTO,     BLOCO_NENHUM },
    { "FACA",       ") {",              BLOCO_NENHUM,       BLOCO_NENHUM },
    { "FIM_ENQUANTO", "}",              BLOCO_NENHUM,       BLOCO_ENQUANTO },
    { "REPITA",     "do {",             BLOCO_REPITA,       BLOCO_NENHUM },
    { "FIM_PARA",   "}",                BLOCO_NENHUM,       BLOCO_PARA },
    /* seleção */
    { "CASO",       "case",             BLOCO_NENHUM,       BLOCO_NENHUM },
    { "CONTRARIO",  "default",          BLOCO_NENHUM,       BLOCO_NENHUM },
    { "FIM_ESCOLHA", "}",               BLOCO_NENHUM,       BLOCO_ESCOLHA },
    { "PARE",       "break",            BLOCO_NENHUM,       BLOCO_NENHUM },
    { "PROSSIGA",   "continue",         BLOCO_NENHUM,       BLOCO_NENHUM },
    /* operadores por palavra */
    { "E",          "&&",               BLOCO_NENHUM,       BLOCO_NENHUM },
    { "OU",         "||",               BLOCO_NENHUM,       BLOCO_NENHUM },
    { "NAO",        "!",                BLOCO_NENHUM,       BLOCO_NENHUM },
    { "MOD",        "%",                BLOCO_NENHUM,       BLOCO_NENHUM },
    { "VERDADEIRO", "true",             BLOCO_NENHUM,       BLOCO_NENHUM },
    { "FALSO",      "false",            BLOCO_NENHUM,       BLOCO_NENHUM },
    { NULL, NULL, 0, 0 }
};

/* 3. Construtores de valor: emitidos como BG_<NOME> (definidos no cabeçalho).
 *    Nada de duplicar a tabela de formatos nem a biblioteca. */
static const char *VALORES[] = {
    "ESCREVA", "ESCREVAL", "ESCREVA_LINHA", "ESCREVA_TEXTO", "ESCREVA_FMT",
    "ESCREVA_ERRO", "LEIA", "LEIA_OK", "LEIA_TEXTO",
    "VETOR", "TAMANHO_VETOR", "TEXTO_FIXO",
    "PARA", "PARA_PASSO", "ATE_QUE",
    "ABS", "MINIMO", "MAXIMO", "POTENCIA", "RAIZ", "ARREDONDA", "PISO", "TETO",
    "SORTEIA", "INICIA_SORTEIO",
    "IGUAL", "DIFERENTE", "TEXTOS_IGUAIS", "TEXTOS_DIFERENTES",
    "TAMANHO_TEXTO", "COPIA_TEXTO", "JUNTA_TEXTO", "ACRESCENTA_TEXTO",
    "TRECHO_TEXTO", "COMPARA_TEXTO", "TEXTO_VAZIO", "FORMATA_TEXTO",
    "TEXTO_PARA_INTEIRO", "TEXTO_PARA_NATURAL", "TEXTO_PARA_REAL",
    "REAL_PARA_INTEIRO",
    "E_DIGITO", "E_LETRA", "CARACTERE_MAIUSCULO", "CARACTERE_MINUSCULO",
    "OU_EXCLUSIVO", "LOGICO_DE", "DIVIDE", "RESTO",
    "TAMANHO_DE", "BITS_DE", "MAXIMO_INTEIRO", "MINIMO_INTEIRO",
    "MAXIMO_NATURAL", "MAXIMO_REAL", "EPSILON_REAL",
    "LEIA_INTEIRO", "LEIA_REAL",
    "SOMA_SEGURA", "SUBTRAI_SEGURA", "MULTIPLICA_SEGURA",
    "ERRO", "AFIRME",
    NULL
};

static const char *TIPOS[] = {
    "LOGICO", "CARACTERE", "INTEIRO_CURTO", "INTEIRO", "INTEIRO_LONGO",
    "INTEIRO8", "INTEIRO16", "INTEIRO32", "INTEIRO64", "NATURAL",
    "NATURAL_CURTO", "NATURAL_LONGO",
    "NATURAL8", "NATURAL16", "NATURAL32", "NATURAL64",
    "REAL_CURTO", "REAL", "REAL_LONGO", "TEXTO", "VAZIO",
    NULL
};

/* Palavras reservadas da linguagem que podem aparecer no laço açucarado. */
static const char *ESTRUTURAIS[] = {
    "DE", "ATE", "PASSO", "FACA", "PARA", "FIM_PARA", "REPITA",
    NULL
};

static bool em_lista(const char *s, const char *const *lista)
{
    int i;
    for (i = 0; lista[i] != NULL; i++) {
        if (strcmp(s, lista[i]) == 0) {
            return true;
        }
    }
    return false;
}

static const Traducao *busca_traducao(const char *s)
{
    int i;
    for (i = 0; TRADUCOES[i].bg != NULL; i++) {
        if (strcmp(s, TRADUCOES[i].bg) == 0) {
            return &TRADUCOES[i];
        }
    }
    return NULL;
}

/* Devolve a forma ASCII de um identificador acentuado, ou NULL. */
static const char *desacentua(const char *s)
{
    int i;
    for (i = 0; APELIDOS[i].acentuado != NULL; i++) {
        if (strcmp(s, APELIDOS[i].acentuado) == 0) {
            return APELIDOS[i].ascii;
        }
    }
    return NULL;
}

/* Pre-passe: troca os apelidos acentuados pela forma ASCII em TODOS os tokens,
 * de modo que o restante do programa so lide com identificadores ASCII. */
static void normaliza_apelidos(Lista *l)
{
    int i;
    for (i = 0; i < l->qtd; i++) {
        const char *ascii;
        if (l->itens[i].tipo != TK_IDENT) {
            continue;
        }
        ascii = desacentua(l->itens[i].texto);
        if (ascii != NULL) {
            size_t n = strlen(ascii);
            char *novo = (char *) malloc(n + 1);
            if (novo == NULL) {
                bg_morte("sem memoria");
            }
            memcpy(novo, ascii, n + 1);
            free(l->itens[i].texto);
            l->itens[i].texto = novo;
        }
    }
}

/* =====================================================================
 * Estado da tradução
 * ===================================================================== */

typedef struct {
    const char *arquivo;
    bool        sem_avisos;
    int         erros;
    int         avisos;
    Buf         saida;
    /* pilha de blocos para --checar e para erros de fechamento */
    int         pilha[256];
    int         linha_pilha[256];
    int         topo;
    /* avisos já emitidos (deduplicação simples por nome) */
    char       *vistos[512];
    int         qtd_vistos;
    bool        dentro_enumeracao;
    char       *enumeradores[256];
    int         qtd_enumeradores;
    char       *ponteiros[128];      /* parametros declarados como [] ou * */
    int         qtd_ponteiros;
} Estado;

static void erro(Estado *e, int linha, const char *fmt, ...) BG_FORMATO(3, 4);

static void erro(Estado *e, int linha, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d: erro: ", e->arquivo, linha);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    e->erros++;
}

static void aviso(Estado *e, int linha, const char *codigo, const char *fmt, ...)
    BG_FORMATO(4, 5);

static void aviso(Estado *e, int linha, const char *codigo, const char *fmt, ...)
{
    va_list ap;
    if (e->sem_avisos) {
        return;
    }
    fprintf(stderr, "%s:%d: aviso: [%s] ", e->arquivo, linha, codigo);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    e->avisos++;
}

static bool ja_avisado(Estado *e, const char *nome)
{
    int i;
    for (i = 0; i < e->qtd_vistos; i++) {
        if (strcmp(e->vistos[i], nome) == 0) {
            return true;
        }
    }
    if (e->qtd_vistos < 512) {
        size_t n = strlen(nome);
        e->vistos[e->qtd_vistos] = (char *) malloc(n + 1);
        if (e->vistos[e->qtd_vistos] == NULL) {
            bg_morte("sem memoria");
        }
        memcpy(e->vistos[e->qtd_vistos], nome, n + 1);
        e->qtd_vistos++;
    }
    return false;
}

/* Libera tudo o que o Estado alocou (nomes já avisados, enumeradores,
 * parâmetros-ponteiro) e o buffer de saída. */
static void estado_libera(Estado *e)
{
    int i;
    for (i = 0; i < e->qtd_vistos; i++) {
        free(e->vistos[i]);
    }
    e->qtd_vistos = 0;
    for (i = 0; i < e->qtd_enumeradores; i++) {
        free(e->enumeradores[i]);
    }
    e->qtd_enumeradores = 0;
    for (i = 0; i < e->qtd_ponteiros; i++) {
        free(e->ponteiros[i]);
    }
    e->qtd_ponteiros = 0;
    buf_libera(&e->saida);
}

/* Caminho único de saída: libera fonte, tokens e o estado, e devolve o código
 * de saída pedido. */
static int finaliza(Estado *e, Lista *l, char *src, int rc)
{
    lista_libera(l);
    free(src);
    estado_libera(e);
    return rc;
}

static const char *nome_bloco(int b)
{
    switch (b) {
        case BLOCO_INICIO:      return "INICIO";
        case BLOCO_SE:          return "SE";
        case BLOCO_ENQUANTO:    return "ENQUANTO";
        case BLOCO_REPITA:      return "REPITA";
        case BLOCO_ESCOLHA:     return "ESCOLHA";
        case BLOCO_PARA:        return "PARA ... DE ... FACA";
        case BLOCO_REGISTRO:    return "REGISTRO";
        case BLOCO_ENUMERACAO:  return "ENUMERACAO";
        default:                return "?";
    }
}

static const char *fechador_de(int b)
{
    switch (b) {
        case BLOCO_INICIO:      return "FIM";
        case BLOCO_SE:          return "FIM_SE";
        case BLOCO_ENQUANTO:    return "FIM_ENQUANTO";
        case BLOCO_REPITA:      return "ATE_QUE(c); ou ATE c;";
        case BLOCO_ESCOLHA:     return "FIM_ESCOLHA";
        case BLOCO_PARA:        return "FIM_PARA";
        case BLOCO_REGISTRO:    return "FIM_REGISTRO(Nome)";
        case BLOCO_ENUMERACAO:  return "FIM_ENUMERACAO(Nome)";
        default:                return "?";
    }
}

static void empilha(Estado *e, int bloco, int linha)
{
    if (e->topo >= 256) {
        erro(e, linha, "blocos aninhados demais (limite 256)");
        return;
    }
    e->pilha[e->topo] = bloco;
    e->linha_pilha[e->topo] = linha;
    e->topo++;
}

static void desempilha(Estado *e, int bloco, int linha)
{
    if (e->topo == 0) {
        erro(e, linha, "fechamento de %s sem bloco aberto", nome_bloco(bloco));
        return;
    }
    if (e->pilha[e->topo - 1] != bloco) {
        erro(e, linha,
             "'%s' fecha o bloco errado: esperado '%s' (aberto na linha %d)",
             fechador_de(bloco), fechador_de(e->pilha[e->topo - 1]),
             e->linha_pilha[e->topo - 1]);
        /* mesmo assim, descarta o bloco mais interno para continuar */
    }
    e->topo--;
}

/* =====================================================================
 * Utilidades de varredura
 * ===================================================================== */

static bool significado(TipoToken t)
{
    return t != TK_ESPACO && t != TK_COMENTARIO;
}

static int proximo_significativo(const Lista *l, int i)
{
    int j = i + 1;
    while (j < l->qtd && !significado(l->itens[j].tipo)) {
        j++;
    }
    return (j < l->qtd) ? j : -1;
}

/* Profundidade considerando ( [ { */
static void ajusta_profundidade(const char *txt, int *prof)
{
    if (strcmp(txt, "(") == 0 || strcmp(txt, "[") == 0 || strcmp(txt, "{") == 0) {
        (*prof)++;
    } else if (strcmp(txt, ")") == 0 || strcmp(txt, "]") == 0 || strcmp(txt, "}") == 0) {
        (*prof)--;
    }
}

/* Copia tokens de i até encontrar um dos terminadores (identificador, no nível
 * 0). Devolve o índice do terminador (ou l->qtd) e deixa o texto coletado em
 * 'dest'. O terminador NÃO é consumido. */
static int coleta_ate(Estado *e, const Lista *l, int i, const char *const *fins, Buf *dest)
{
    int prof = 0;

    buf_limpa(dest);
    while (i < l->qtd) {
        const Token *t = &l->itens[i];

        if (t->tipo == TK_DIRETIVA) {
            erro(e, t->linha, "diretiva de pre-processador dentro de expressao");
            return i;
        }
        if (t->tipo == TK_IDENT && prof == 0 && em_lista(t->texto, fins)) {
            return i;
        }
        if (significado(t->tipo)) {
            ajusta_profundidade(t->texto, &prof);
        }
        if (prof < 0) {
            erro(e, t->linha, "parenteses desbalanceados na expressao");
            return i;
        }
        buf_adiciona(dest, t->texto);
        i++;
    }
    return i;
}

static void buf_limpa_espacos(Buf *b)
{
    /* remove espaços no início e no fim */
    size_t i = 0;
    size_t fim;
    while (b->dados[i] == ' ' || b->dados[i] == '\t' || b->dados[i] == '\n' || b->dados[i] == '\r') {
        i++;
    }
    if (i > 0) {
        memmove(b->dados, b->dados + i, b->tam - i + 1);
        b->tam -= i;
    }
    fim = b->tam;
    while (fim > 0) {
        char c = b->dados[fim - 1];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            fim--;
        } else {
            break;
        }
    }
    b->dados[fim] = '\0';
    b->tam = fim;
}

/* =====================================================================
 * Verificações de aviso
 * ===================================================================== */

static bool em_bloco(const Estado *e, int bloco)
{
    int i;
    for (i = 0; i < e->topo; i++) {
        if (e->pilha[i] == bloco) {
            return true;
        }
    }
    return false;
}

/* Identificadores de C que o usuário pode legitimamente usar. */
static const char *NOMES_C[] = {
    "NULL", "EOF", "EXIT_SUCCESS", "EXIT_FAILURE", "SIZE_MAX", "RAND_MAX",
    "INT_MAX", "INT_MIN", "UINT_MAX", "CHAR_BIT", "BUFSIZ", "SEEK_SET",
    "SEEK_CUR", "SEEK_END", "true", "false",
    "NAN", "INFINITY", "HUGE_VAL", "DBL_MAX", "DBL_EPSILON",
    NULL
};

static bool eh_enumerador(const Estado *e, const char *s)
{
    int i;
    for (i = 0; i < e->qtd_enumeradores; i++) {
        if (strcmp(e->enumeradores[i], s) == 0) {
            return true;
        }
    }
    return false;
}


/* Le um arquivo devolvendo NULL quando ele nao existe (nao e erro fatal). */
static char *le_arquivo_ou_nulo(const char *caminho)
{
    FILE *f = fopen(caminho, "rb");
    char *dados;
    long  n;

    if (f == NULL) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (n = ftell(f)) < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    dados = (char *) malloc((size_t) n + 1);
    if (dados == NULL) {
        bg_morte("sem memoria para '%s'", caminho);
    }
    if (fread(dados, 1, (size_t) n, f) != (size_t) n) {
        free(dados);
        fclose(f);
        return NULL;
    }
    dados[n] = '\0';
    fclose(f);
    return dados;
}

static void registra_enumerador_nome(Estado *e, const char *nome)
{
    int i;
    size_t n;

    if (e->qtd_enumeradores >= 256) {
        return;
    }
    for (i = 0; i < e->qtd_enumeradores; i++) {
        if (strcmp(e->enumeradores[i], nome) == 0) {
            return;
        }
    }
    n = strlen(nome);
    e->enumeradores[e->qtd_enumeradores] = (char *) malloc(n + 1);
    if (e->enumeradores[e->qtd_enumeradores] == NULL) {
        bg_morte("sem memoria");
    }
    memcpy(e->enumeradores[e->qtd_enumeradores], nome, n + 1);
    e->qtd_enumeradores++;
}

/* Coleta os enumeradores declarados em cabecalhos locais (#include "...") para
 * que o W-05 respeite a excecao documentada de R-1 tambem entre arquivos. */
static void coleta_de_cabecalho(const char *caminho, Estado *e, int prof)
{
    static char vistos[8][512];
    static int  qtd_vistos = 0;
    char dir[512];
    char *texto;
    char *p;
    const char *barra;
    Lista l = { NULL, 0, 0 };
    int i;

    if (prof > 4 || qtd_vistos >= 8) {
        return;
    }
    for (i = 0; i < qtd_vistos; i++) {
        if (strcmp(vistos[i], caminho) == 0) {
            return;                       /* ja lido (guarda de inclusao) */
        }
    }
    if (strlen(caminho) < sizeof vistos[0]) {
        strcpy(vistos[qtd_vistos++], caminho);
    }

    texto = le_arquivo_ou_nulo(caminho);
    if (texto == NULL) {
        return;
    }

    barra = strrchr(caminho, '/');
    if (barra == NULL) {
        dir[0] = '\0';
    } else {
        size_t n = (size_t) (barra - caminho) + 1;
        if (n >= sizeof dir) {
            n = sizeof dir - 1;
        }
        memcpy(dir, caminho, n);
        dir[n] = '\0';
    }

    /* enumeradores do proprio cabecalho */
    tokeniza(texto, &l);
    {
        bool dentro = false;
        for (i = 0; i < l.qtd; i++) {
            const Token *t = &l.itens[i];
            if (t->tipo != TK_IDENT) {
                continue;
            }
            if (strcmp(t->texto, "ENUMERACAO") == 0) {
                dentro = true;
                continue;
            }
            if (strcmp(t->texto, "FIM_ENUMERACAO") == 0) {
                dentro = false;
                continue;
            }
            if (!dentro) {
                continue;
            }
            {
                int prox = proximo_significativo(&l, i);
                if (prox >= 0 && l.itens[prox].tipo == TK_PONT
                    && (strcmp(l.itens[prox].texto, ",") == 0
                        || strcmp(l.itens[prox].texto, "=") == 0)) {
                    registra_enumerador_nome(e, t->texto);
                } else if (prox >= 0 && l.itens[prox].tipo == TK_IDENT
                           && strcmp(l.itens[prox].texto, "FIM_ENUMERACAO") == 0) {
                    registra_enumerador_nome(e, t->texto);
                }
            }
        }
    }

    /* includes locais, resolvidos a partir do diretorio deste arquivo */
    p = strstr(texto, "#include \"");
    while (p != NULL) {
        const char *ini = p + strlen("#include \"");
        const char *fim = strchr(ini, '"');
        if (fim != NULL) {
            char alvo[1024];
            size_t n = (size_t) (fim - ini);
            if (strlen(dir) + n + 1 < sizeof alvo) {
                memcpy(alvo, dir, strlen(dir));
                memcpy(alvo + strlen(dir), ini, n);
                alvo[strlen(dir) + n] = '\0';
                coleta_de_cabecalho(alvo, e, prof + 1);
            }
        }
        p = strstr(p + 1, "#include \"");
    }

    lista_libera(&l);
    free(texto);
}

static void coleta_enumeradores(const char *caminho_bg, Estado *e)
{
    coleta_de_cabecalho(caminho_bg, e, 0);
}

static bool eh_ponteiro(const Estado *e, const char *nome)
{
    int i;
    for (i = 0; i < e->qtd_ponteiros; i++) {
        if (strcmp(e->ponteiros[i], nome) == 0) {
            return true;
        }
    }
    return false;
}

/* Pre-passe: nomes declarados como parametro de vetor/ponteiro (`v[]` ou `*v`).
 * Serve para o W-06 (TAMANHO_VETOR de um parametro nao da o tamanho). */
static void coleta_parametros(Estado *e, const Lista *l)
{
    int i;
    for (i = 0; i < l->qtd; i++) {
        const Token *t = &l->itens[i];
        int prox, ant;

        if (t->tipo != TK_IDENT) {
            continue;
        }
        prox = proximo_significativo(l, i);
        ant = i - 1;
        while (ant >= 0 && !significado(l->itens[ant].tipo)) {
            ant--;
        }
        {
            bool por_ponteiro = (ant >= 0 && l->itens[ant].tipo == TK_PONT
                                 && strcmp(l->itens[ant].texto, "*") == 0);
            bool por_colchete = false;

            if (prox >= 0 && l->itens[prox].tipo == TK_PONT
                && strcmp(l->itens[prox].texto, "[") == 0) {
                int fecha = proximo_significativo(l, prox);
                por_colchete = (fecha >= 0 && l->itens[fecha].tipo == TK_PONT
                                && strcmp(l->itens[fecha].texto, "]") == 0);
            }
            if ((por_ponteiro || por_colchete) && e->qtd_ponteiros < 128) {
                size_t n;
                char *copia;
                int k;
                bool repetido = false;

                for (k = 0; k < e->qtd_ponteiros; k++) {
                    if (strcmp(e->ponteiros[k], t->texto) == 0) {
                        repetido = true;
                    }
                }
                if (repetido) {
                    continue;
                }
                n = strlen(t->texto);
                copia = (char *) malloc(n + 1);
                if (copia == NULL) {
                    bg_morte("sem memoria");
                }
                memcpy(copia, t->texto, n + 1);
                e->ponteiros[e->qtd_ponteiros++] = copia;
            }
        }
    }
}

/* W-06: TAMANHO_VETOR de um parametro declarado como [] ou * (C-18). */
static void verifica_w06(Estado *e, const Lista *l, int i)
{
    int j = proximo_significativo(l, i);

    if (j < 0 || !(l->itens[j].tipo == TK_PONT && strcmp(l->itens[j].texto, "(") == 0)) {
        return;
    }
    j = proximo_significativo(l, j);
    if (j >= 0 && l->itens[j].tipo == TK_IDENT && eh_ponteiro(e, l->itens[j].texto)) {
        aviso(e, l->itens[j].linha, "W-06",
              "TAMANHO_VETOR de '%s', que e parametro (recebido por ponteiro): "
              "em C isso mede o tamanho do ponteiro, nao do vetor (C-18)",
              l->itens[j].texto);
    }
}

/* W-04: variavel de controle de PARA modificada dentro do corpo. */
static void verifica_w04(Estado *e, const Lista *l, int i)
{
    int j = proximo_significativo(l, i);
    int prof = 0;
    int fim_args = -1;
    int corpo = -1;
    char var[128];
    size_t n = 0;

    if (j < 0 || !(l->itens[j].tipo == TK_PONT && strcmp(l->itens[j].texto, "(") == 0)) {
        return;
    }
    /* nome da variavel de controle = primeiro argumento */
    j = proximo_significativo(l, j);
    if (j < 0 || l->itens[j].tipo != TK_IDENT) {
        return;
    }
    n = strlen(l->itens[j].texto);
    if (n >= sizeof var) {
        return;
    }
    memcpy(var, l->itens[j].texto, n + 1);

    /* fim da lista de argumentos */
    prof = 1;
    for (j = j + 1; j < l->qtd && prof > 0; j++) {
        if (l->itens[j].tipo == TK_PONT) {
            const char *t = l->itens[j].texto;
            if (strcmp(t, "(") == 0 || strcmp(t, "[") == 0) {
                prof++;
            } else if (strcmp(t, ")") == 0 || strcmp(t, "]") == 0) {
                prof--;
                if (prof == 0) {
                    fim_args = j;
                }
            }
        }
    }
    if (fim_args < 0) {
        return;
    }
    corpo = proximo_significativo(l, fim_args);
    if (corpo < 0 || !(l->itens[corpo].tipo == TK_PONT
                       && strcmp(l->itens[corpo].texto, "{") == 0)) {
        return;                       /* forma sem chaves: W-04 nao se aplica */
    }
    prof = 0;
    for (j = corpo; j < l->qtd; j++) {
        const Token *t = &l->itens[j];
        if (t->tipo == TK_PONT) {
            if (strcmp(t->texto, "{") == 0) {
                prof++;
                continue;
            }
            if (strcmp(t->texto, "}") == 0) {
                prof--;
                if (prof == 0) {
                    return;
                }
                continue;
            }
            if (strcmp(t->texto, "=") == 0 || strcmp(t->texto, "+=") == 0
                || strcmp(t->texto, "-=") == 0 || strcmp(t->texto, "*=") == 0
                || strcmp(t->texto, "/=") == 0 || strcmp(t->texto, "%=") == 0
                || strcmp(t->texto, "++") == 0 || strcmp(t->texto, "--") == 0) {
                int ant = j - 1;
                while (ant > corpo && !significado(l->itens[ant].tipo)) {
                    ant--;
                }
                if (ant > corpo && l->itens[ant].tipo == TK_IDENT
                    && strcmp(l->itens[ant].texto, var) == 0) {
                    aviso(e, l->itens[ant].linha, "W-04",
                          "a variavel de controle '%s' do PARA e modificada dentro "
                          "do corpo: o passo do laco sera sobrescrito", var);
                    return;
                }
            }
        }
    }
}

/* Macros que sao de funcao (exigem parenteses). Usar sem '(' nunca e o que a
 * pessoa quis — e o erro mais comum de quem vem do Portugol ("x IGUAL 3"). */
static const char *EXIGEM_PARENTESES[] = {
    "IGUAL", "DIFERENTE", "TEXTOS_IGUAIS", "TEXTOS_DIFERENTES",
    "MINIMO", "MAXIMO", "POTENCIA", "ABS", "RAIZ", "ARREDONDA", "PISO", "TETO",
    "SORTEIA", "TAMANHO_TEXTO", "TAMANHO_VETOR", "COPIA_TEXTO", "JUNTA_TEXTO",
    "TEXTO_PARA_INTEIRO", "TEXTO_PARA_REAL", "ATE_QUE", "VETOR", "TEXTO_FIXO",
    "REGISTRO", "ENUMERACAO", "FIM_REGISTRO", "FIM_ENUMERACAO",
    "ESCREVA", "ESCREVAL", "ESCREVA_LINHA", "ESCREVA_TEXTO", "ESCREVA_FMT",
    "ESCREVA_ERRO", "LEIA", "LEIA_OK", "LEIA_TEXTO", "ERRO", "AFIRME", "ESCOLHA",
    "PARA", "PARA_PASSO",
    NULL
};

static bool exige_parenteses(const char *nome)
{
    int i;
    for (i = 0; EXIGEM_PARENTESES[i] != NULL; i++) {
        if (strcmp(EXIGEM_PARENTESES[i], nome) == 0) {
            return true;
        }
    }
    return false;
}

/* Exige '(' depois do identificador em i e copia o conteudo balanceado para
 * 'dentro'. Devolve o indice do ')' correspondente (ou l->qtd em erro). */
static int captura_parenteses(Estado *e, const Lista *l, int i, Buf *dentro)
{
    int j = proximo_significativo(l, i);
    int prof = 0;

    buf_limpa(dentro);
    if (j < 0 || !(l->itens[j].tipo == TK_PONT && strcmp(l->itens[j].texto, "(") == 0)) {
        erro(e, l->itens[i].linha, "esperado '(' depois de '%s'", l->itens[i].texto);
        return l->qtd;
    }
    for (j = j + 1; j < l->qtd; j++) {
        const Token *t = &l->itens[j];
        if (t->tipo == TK_PONT && strcmp(t->texto, ")") == 0 && prof == 0) {
            buf_limpa_espacos(dentro);
            return j;
        }
        if (significado(t->tipo)) {
            ajusta_profundidade(t->texto, &prof);
        }
        buf_adiciona(dentro, t->texto);
    }
    erro(e, l->itens[i].linha, "'(' de '%s' nao foi fechado", l->itens[i].texto);
    return l->qtd;
}

/* Verifica o cabecalho de SE/ENQUANTO/SENAO_SE:
 *   - W-01 se houver '=' (atribuicao) no nivel 0;
 *   - erro se aparecer '{' ou ';' antes do terminador (ENTAO/FACA) —
 *     e o erro classico de quem escreve C com palavras do portugues. */
static void verifica_cabecalho(Estado *e, const Lista *l, int i, const char *const *fim)
{
    int j = i + 1;
    int prof = 0;

    /* W-03: literal numerico como condicao inteira (padrao "verdade em C") */
    {
        int a = proximo_significativo(l, i);
        if (a >= 0 && l->itens[a].tipo == TK_NUM) {
            int b = proximo_significativo(l, a);
            if (b >= 0 && l->itens[b].tipo == TK_IDENT && em_lista(l->itens[b].texto, fim)) {
                aviso(e, l->itens[a].linha, "W-03",
                      "condicao numerica literal: prefira uma comparacao ou LOGICO "
                      "(em C, qualquer valor != 0 e verdadeiro)");
            }
        }
    }

    while (j < l->qtd) {
        const Token *t = &l->itens[j];
        if (t->tipo == TK_IDENT && prof == 0 && em_lista(t->texto, fim)) {
            return;
        }
        if (significado(t->tipo)) {
            if (prof == 0 && t->tipo == TK_PONT) {
                if (strcmp(t->texto, "=") == 0) {
                    aviso(e, t->linha, "W-01",
                          "atribuicao ('=') dentro de condicao; use '==' ou IGUAL(a, b)");
                    return;
                }
                if (strcmp(t->texto, "{") == 0 || strcmp(t->texto, ";") == 0) {
                    erro(e, t->linha,
                         "'%s' exige '%s' antes de '%s' (nao use chaves nem ';' "
                         "no cabecalho do bloco)",
                         l->itens[i].texto, fim[0], t->texto);
                    return;
                }
            }
            ajusta_profundidade(t->texto, &prof);
        }
        j++;
    }
    erro(e, l->itens[i].linha, "'%s' sem '%s' correspondente",
         l->itens[i].texto, fim[0]);
}

/* W-02: TYPE IDENT ';'  -> declaração sem inicializador. */
static void verifica_w02(Estado *e, const Lista *l, int i)
{
    int j = proximo_significativo(l, i);
    int k;

    if (em_bloco(e, BLOCO_REGISTRO)) {
        return;              /* campo de registro não tem inicializador */
    }
    /* pular para trás até achar EXTERNO/ESTATICO/CONSTANTE na mesma declaração */
    {
        int b = i - 1;
        while (b >= 0) {
            if (!significado(l->itens[b].tipo)) {
                b--;
                continue;
            }
            if (l->itens[b].tipo == TK_IDENT
                && (strcmp(l->itens[b].texto, "EXTERNO") == 0
                    || strcmp(l->itens[b].texto, "ESTATICO") == 0)) {
                return;
            }
            if (l->itens[b].tipo == TK_PONT
                && (strcmp(l->itens[b].texto, ";") == 0
                    || strcmp(l->itens[b].texto, "{") == 0
                    || strcmp(l->itens[b].texto, "}") == 0)) {
                break;       /* começo da declaração */
            }
            b--;
        }
    }

    if (j < 0 || l->itens[j].tipo != TK_IDENT) {
        return;
    }
    k = proximo_significativo(l, j);
    if (k < 0) {
        return;
    }
    if (l->itens[k].tipo == TK_PONT && strcmp(l->itens[k].texto, ";") == 0) {
        aviso(e, l->itens[j].linha, "W-02",
              "variavel '%s' declarada sem inicializador", l->itens[j].texto);
    }
}

/* W-05: identificador 100% maiúsculo declarado pelo usuário. */
static void verifica_w05(Estado *e, const Token *t)
{
    const char *s = t->texto;
    size_t n = strlen(s);
    size_t i;
    bool tem_letra = false;

    if (e->dentro_enumeracao || n < 2) {
        return;
    }
    if (em_lista(s, VALORES) || em_lista(s, ESTRUTURAIS) || em_lista(s, NOMES_C)) {
        return;
    }
    if (eh_enumerador(e, s)) {
        return;
    }
    if (strncmp(s, "BG_", 3) == 0 || strncmp(s, "bg_", 3) == 0 || strncmp(s, "k_", 2) == 0) {
        return;
    }
    if (busca_traducao(s) != NULL) {
        return;
    }
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char) s[i];
        if (c >= 'A' && c <= 'Z') {
            tem_letra = true;
            continue;
        }
        if ((c >= '0' && c <= '9') || c == '_') {
            continue;
        }
        return;              /* tem minúscula (ou acento): ok */
    }
    if (!tem_letra || ja_avisado(e, s)) {
        return;
    }
    aviso(e, t->linha, "W-05",
          "'%s' viola R-1: identificadores do usuario devem ter ao menos uma minuscula", s);
}

/* W-07: tamanho de VETOR/TEXTO_FIXO que e um identificador k_*. */
static void verifica_w07(Estado *e, const Lista *l, int i, const char *nome)
{
    int j = proximo_significativo(l, i);
    int prof = 0;
    int ultimo_ident = -1;

    if (j < 0 || !(l->itens[j].tipo == TK_PONT && strcmp(l->itens[j].texto, "(") == 0)) {
        return;
    }
    for (j = j + 1; j < l->qtd; j++) {
        const Token *t = &l->itens[j];
        if (!significado(t->tipo)) {
            continue;
        }
        if (t->tipo == TK_PONT && strcmp(t->texto, ")") == 0 && prof == 0) {
            if (ultimo_ident >= 0 && strncmp(l->itens[ultimo_ident].texto, "k_", 2) == 0) {
                aviso(e, l->itens[ultimo_ident].linha, "W-07",
                      "tamanho de %s e '%s' (CONSTANTE): em C isso e VLA "
                      "(ver C-23); use um literal ou #define",
                      nome, l->itens[ultimo_ident].texto);
            }
            return;
        }
        if (prof == 0 && t->tipo == TK_IDENT) {
            ultimo_ident = j;
        }
        ajusta_profundidade(t->texto, &prof);
    }
}

/* =====================================================================
 * Emissão de identificador
 * ===================================================================== */

static void emite_identificador(Estado *e, const Lista *l, int *pi)
{
    const Token *t = &l->itens[*pi];
    const char *nome = t->texto;
    const Traducao *tr;
    int i = *pi;
    int prox_token = proximo_significativo(l, i);

    /* nome de macro de funcao sem '(' depois: quase sempre "x IGUAL 3" */
    if (exige_parenteses(nome)
        && !(prox_token >= 0 && l->itens[prox_token].tipo == TK_PONT
             && strcmp(l->itens[prox_token].texto, "(") == 0)) {
        /* ATE, PARA e PARA_PASSO tem formas alternativas (acucar) — ja tratadas acima */
        if (!(strcmp(nome, "PARA") == 0 || strcmp(nome, "PARA_PASSO") == 0
              || strcmp(nome, "ATE_QUE") == 0)) {
            erro(e, t->linha,
                 "'%s' e prefixado e exige parenteses: use %s(...) "
                 "(ou o operador de C equivalente)", nome, nome);
            *pi = i;
            return;
        }
    }

    /* --- laço contado açucarado: PARA <var> DE <ini> ATE <fim> [PASSO <p>] FACA */
    if (strcmp(nome, "PARA") == 0) {
        int prox = proximo_significativo(l, i);
        if (prox >= 0 && !(l->itens[prox].tipo == TK_PONT
                          && strcmp(l->itens[prox].texto, "(") == 0)) {
            static const char *fins_ate[]  = { "ATE", NULL };
            static const char *fins_passo[] = { "PASSO", "FACA", NULL };
            Buf var, ini, fim, passo;
            int j;

            buf_ini(&var); buf_ini(&ini); buf_ini(&fim); buf_ini(&passo);

            if (prox < 0 || l->itens[prox].tipo != TK_IDENT) {
                erro(e, t->linha, "esperado o nome da variavel de controle depois de PARA");
                *pi = i;
                return;
            }
            buf_adiciona(&var, l->itens[prox].texto);

            j = proximo_significativo(l, prox);
            if (j < 0 || l->itens[j].tipo != TK_IDENT || strcmp(l->itens[j].texto, "DE") != 0) {
                erro(e, l->itens[prox].linha,
                     "esperado 'DE' depois da variavel de controle em 'PARA %s DE ...'",
                     l->itens[prox].texto);
                *pi = i;
                return;
            }

            j = coleta_ate(e, l, j + 1, fins_ate, &ini);
            if (j >= l->qtd) {
                erro(e, t->linha, "faltou 'ATE' no laco 'PARA %s DE ... ATE ...'", var.dados);
            } else {
                j = coleta_ate(e, l, j + 1, fins_passo, &fim);
            }

            if (j < l->qtd && l->itens[j].tipo == TK_IDENT
                && strcmp(l->itens[j].texto, "PASSO") == 0) {
                j = coleta_ate(e, l, j + 1, fins_passo, &passo);
            }

            if (j >= l->qtd || !(l->itens[j].tipo == TK_IDENT
                                 && strcmp(l->itens[j].texto, "FACA") == 0)) {
                erro(e, t->linha, "faltou 'FACA' no fim do cabecalho do 'PARA ... DE ...'");
            }

            buf_limpa_espacos(&ini);
            buf_limpa_espacos(&fim);
            buf_limpa_espacos(&passo);
            if (passo.tam == 0) {
                buf_adiciona(&passo, "1");
            }
            /* Fase 3: passo literal zero nunca avanca o laco. Com passo
             * variavel a mesma situacao vira erro de execucao (bg_passo_zero). */
            if (strcmp(passo.dados, "0") == 0) {
                erro(e, t->linha,
                     "'PASSO 0' nunca avanca o laco: use um passo diferente de zero");
            }

            buf_adiciona(&e->saida, "for (");
            buf_adiciona(&e->saida, var.dados);
            buf_adiciona(&e->saida, " = (");
            buf_adiciona(&e->saida, ini.dados);
            buf_adiciona(&e->saida, "); (");
            buf_adiciona(&e->saida, passo.dados);
            buf_adiciona(&e->saida, ") > 0 ? (");
            buf_adiciona(&e->saida, var.dados);
            buf_adiciona(&e->saida, " <= (");
            buf_adiciona(&e->saida, fim.dados);
            buf_adiciona(&e->saida, ")) : ((");
            buf_adiciona(&e->saida, passo.dados);
            buf_adiciona(&e->saida, ") == 0 ? bg_passo_zero(__FILE__, __LINE__) : (");
            buf_adiciona(&e->saida, var.dados);
            buf_adiciona(&e->saida, " >= (");
            buf_adiciona(&e->saida, fim.dados);
            buf_adiciona(&e->saida, "))); ");
            buf_adiciona(&e->saida, var.dados);
            buf_adiciona(&e->saida, " += (");
            buf_adiciona(&e->saida, passo.dados);
            buf_adiciona(&e->saida, ")) { ");

            empilha(e, BLOCO_PARA, t->linha);

            free(var.dados); free(ini.dados); free(fim.dados); free(passo.dados);
            *pi = (j < l->qtd) ? j : l->qtd - 1;   /* consome FACA */
            return;
        }
    }

    /* --- ATE <expr> ;  (açúcar do REPITA) --- */
    if (strcmp(nome, "ATE") == 0) {
        Buf expr;
        int k;
        int prof = 0;

        if (e->topo == 0 || e->pilha[e->topo - 1] != BLOCO_REPITA) {
            erro(e, t->linha,
                 "'ATE' sem 'REPITA': use 'REPITA ... ATE_QUE(cond);' ou abra um REPITA");
            *pi = i;
            return;
        }

        buf_ini(&expr);
        k = i + 1;
        while (k < l->qtd) {
            const Token *tk = &l->itens[k];
            if (tk->tipo == TK_PONT && strcmp(tk->texto, ";") == 0 && prof == 0) {
                break;
            }
            if (significado(tk->tipo)) {
                ajusta_profundidade(tk->texto, &prof);
            }
            buf_adiciona(&expr, tk->texto);
            k++;
        }
        buf_limpa_espacos(&expr);

        buf_adiciona(&e->saida, "} while (!(");
        buf_adiciona(&e->saida, expr.dados);
        buf_adiciona(&e->saida, "))");
        desempilha(e, BLOCO_REPITA, t->linha);
        free(expr.dados);
        *pi = (k < l->qtd) ? k - 1 : l->qtd - 1;   /* o ';' e emitido normal */
        return;
    }

    /* --- palavras-chave de bloco/estrutura --- */
    tr = busca_traducao(nome);
    if (tr != NULL) {
        /* 'SENAO SE' e o erro classico de encadeamento (C-05) */
        if (strcmp(nome, "SENAO") == 0) {
            int prox = proximo_significativo(l, i);
            if (prox >= 0 && l->itens[prox].tipo == TK_IDENT
                && strcmp(l->itens[prox].texto, "SE") == 0) {
                /* Regra C-05: o par de palavras e sempre recusado, mesmo quando
                 * se quer um SE aninhado dentro do SENAO (que o C aceitaria).
                 * A mensagem diz o que fazer nos dois casos. */
                erro(e, t->linha,
                     "'SENAO SE' nao encadeia: use SENAO_SE (C-05 da especificacao); "
                     "para um SE dentro do SENAO, escreva antes dele um comando do bloco");
            }
        }
        emite_seguro(&e->saida, tr->c);

        if (tr->abre == BLOCO_SE || tr->abre == BLOCO_ENQUANTO) {
            static const char *fins_se[] = { "ENTAO", NULL };
            static const char *fins_enq[] = { "FACA", NULL };
            verifica_cabecalho(e, l, i, (tr->abre == BLOCO_SE)
                                  ? (const char *const *) fins_se
                                  : (const char *const *) fins_enq);
        } else if (strcmp(nome, "SENAO_SE") == 0) {
            static const char *fins[] = { "ENTAO", NULL };
            verifica_cabecalho(e, l, i, (const char *const *) fins);
        }

        if (tr->abre != BLOCO_NENHUM) {
            empilha(e, tr->abre, t->linha);
        }
        if (tr->fecha != BLOCO_NENHUM) {
            if (tr->fecha == BLOCO_ENUMERACAO) {
                e->dentro_enumeracao = false;
            }
            desempilha(e, tr->fecha, t->linha);
        }

        if (em_lista(nome, TIPOS) && tr->abre == BLOCO_NENHUM) {
            verifica_w02(e, l, i);
        }
        if (strcmp(nome, "SENAO") == 0 || strcmp(nome, "SENAO_SE") == 0) {
            if (e->topo == 0 || e->pilha[e->topo - 1] != BLOCO_SE) {
                erro(e, t->linha, "'%s' sem 'SE' correspondente", nome);
            }
        }
        *pi = i;
        return;
    }

    /* --- blocos que recebem um nome/expressao entre parenteses --- */
    if (strcmp(nome, "ESCOLHA") == 0 || strcmp(nome, "REGISTRO") == 0
        || strcmp(nome, "ENUMERACAO") == 0 || strcmp(nome, "FIM_REGISTRO") == 0
        || strcmp(nome, "FIM_ENUMERACAO") == 0) {
        Buf dentro;
        int fim;
        int chave;

        buf_ini(&dentro);
        fim = captura_parenteses(e, l, i, &dentro);

        /* REGISTRO/ENUMERACAO ja emitem as chaves pela macro: um '{' explicito e
         * erro, mas e engolido para nao gerar uma cascata de mensagens. */
        chave = -1;
        if (strcmp(nome, "REGISTRO") == 0 || strcmp(nome, "ENUMERACAO") == 0) {
            int depois = proximo_significativo(l, fim);
            if (depois >= 0 && l->itens[depois].tipo == TK_PONT
                && strcmp(l->itens[depois].texto, "{") == 0) {
                erro(e, l->itens[depois].linha,
                     "'%s' ja abre o bloco: nao use '{' (va direto para os campos)",
                     nome);
                chave = depois;
            }
        }

        if (strcmp(nome, "ESCOLHA") == 0) {
            buf_adiciona(&e->saida, "switch (");
            buf_adiciona(&e->saida, dentro.dados);
            buf_adiciona(&e->saida, ") {");
            empilha(e, BLOCO_ESCOLHA, t->linha);
        } else if (strcmp(nome, "REGISTRO") == 0) {
            buf_adiciona(&e->saida, "typedef struct ");
            buf_adiciona(&e->saida, dentro.dados);
            buf_adiciona(&e->saida, " {");
            empilha(e, BLOCO_REGISTRO, t->linha);
        } else if (strcmp(nome, "ENUMERACAO") == 0) {
            buf_adiciona(&e->saida, "typedef enum ");
            buf_adiciona(&e->saida, dentro.dados);
            buf_adiciona(&e->saida, " {");
            empilha(e, BLOCO_ENUMERACAO, t->linha);
            e->dentro_enumeracao = true;
        } else if (strcmp(nome, "FIM_REGISTRO") == 0) {
            buf_adiciona(&e->saida, "} ");
            buf_adiciona(&e->saida, dentro.dados);
            desempilha(e, BLOCO_REGISTRO, t->linha);
        } else {
            buf_adiciona(&e->saida, "} ");
            buf_adiciona(&e->saida, dentro.dados);
            desempilha(e, BLOCO_ENUMERACAO, t->linha);
            e->dentro_enumeracao = false;
        }

        free(dentro.dados);
        *pi = (chave >= 0) ? chave : ((fim < l->qtd) ? fim : l->qtd - 1);
        return;
    }

    /* --- construtores de valor: forma prefixada do cabecalho --- */
    if (em_lista(nome, VALORES)) {
        buf_adiciona(&e->saida, "BG_");
        buf_adiciona(&e->saida, nome);
        if (strcmp(nome, "VETOR") == 0) {
            verifica_w07(e, l, i, "VETOR");
        } else if (strcmp(nome, "TEXTO_FIXO") == 0) {
            verifica_w07(e, l, i, "TEXTO_FIXO");
        } else if (strcmp(nome, "TAMANHO_VETOR") == 0) {
            verifica_w06(e, l, i);
        } else if (strcmp(nome, "PARA") == 0 || strcmp(nome, "PARA_PASSO") == 0) {
            verifica_w04(e, l, i);
        }
        if (strcmp(nome, "ATE_QUE") == 0) {
            desempilha(e, BLOCO_REPITA, t->linha);
            /* ATE_QUE(c) expande para "} while (!(c))": sem ';' logo depois
             * do ')', o erro so apareceria longe, no compilador C. Fase 3. */
            {
                Buf cond;
                int fim;
                int depois;

                buf_ini(&cond);
                fim = captura_parenteses(e, l, i, &cond);
                if (fim < l->qtd) {
                    depois = proximo_significativo(l, fim);
                    if (depois < 0 || !(l->itens[depois].tipo == TK_PONT
                                        && strcmp(l->itens[depois].texto, ";") == 0)) {
                        erro(e, t->linha,
                             "'ATE_QUE(cond)' exige ';' depois do ')' (e um do/while)");
                    }
                }
                free(cond.dados);
            }
        }
        *pi = i;
        return;
    }

    /* --- identificador comum --- */
    if (e->dentro_enumeracao && e->qtd_enumeradores < 256 && !ja_avisado(e, nome)) {
        size_t n = strlen(nome);
        e->enumeradores[e->qtd_enumeradores] = (char *) malloc(n + 1);
        if (e->enumeradores[e->qtd_enumeradores] == NULL) {
            bg_morte("sem memoria");
        }
        memcpy(e->enumeradores[e->qtd_enumeradores], nome, n + 1);
        e->qtd_enumeradores++;
    }
    verifica_w05(e, t);
    buf_adiciona(&e->saida, nome);
    *pi = i;
}

/* =====================================================================
 * Tradução completa
 * ===================================================================== */

static void traduz(Estado *e, const Lista *l)
{
    int i = 0;

    while (i < l->qtd) {
        const Token *t = &l->itens[i];

        if (t->tipo == TK_IDENT) {
            emite_identificador(e, l, &i);
        } else {
            buf_adiciona(&e->saida, t->texto);
        }
        i++;
    }

    /* nao cascatear erros de bloco quando ja houve um erro estrutural */
    if (e->erros > 0) {
        return;
    }
    while (e->topo > 0) {
        e->topo--;
        erro(e, e->linha_pilha[e->topo],
             "'%s' aberto aqui nao foi fechado (falta '%s')",
             nome_bloco(e->pilha[e->topo]), fechador_de(e->pilha[e->topo]));
    }
}

/* =====================================================================
 * Linha de comando
 * ===================================================================== */

static void uso(FILE *f)
{
    fputs(
        "uso: basicgolc [opcoes] arquivo.c\n"
        "\n"
        "  -o ARQUIVO      escreve o C gerado em ARQUIVO (padrao: stdout)\n"
        "  --checar        so valida estrutura de blocos e avisos; nao gera C\n"
        "  -w, --sem-avisos  suprime avisos (W-xx)\n"
        "  -v, --versao    mostra a versao\n"
        "  -h, --ajuda     mostra esta ajuda\n"
        "\n"
        "Sai com 0 em sucesso, 1 em erro de sintaxe/estrutura, 2 em erro de uso.\n",
        f);
}

int main(int argc, char **argv)
{
    const char *entrada = NULL;
    const char *saida = NULL;
    bool checar = false;
    Estado e;
    char *src;
    Lista l;
    int i;
    FILE *f;
    int rc;

    memset(&e, 0, sizeof e);
    memset(&l, 0, sizeof l);

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--ajuda") == 0) {
            uso(stdout);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--versao") == 0) {
            printf("basicgolc %s (BASICGOL 1.1.0)\n", BG_VERSAO);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fputs("basicgolc: -o exige um argumento\n", stderr);
                return 2;
            }
            saida = argv[++i];
        } else if (strcmp(argv[i], "--checar") == 0 || strcmp(argv[i], "--check") == 0) {
            checar = true;
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--sem-avisos") == 0) {
            e.sem_avisos = true;
        } else if (strcmp(argv[i], "-") == 0) {
            entrada = "-";
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "basicgolc: opcao desconhecida '%s'\n", argv[i]);
            uso(stderr);
            return 2;
        } else if (entrada == NULL) {
            entrada = argv[i];
        } else {
            fprintf(stderr, "basicgolc: mais de um arquivo de entrada ('%s')\n", argv[i]);
            return 2;
        }
    }

    if (entrada == NULL) {
        fputs("basicgolc: falta o arquivo de entrada\n", stderr);
        uso(stderr);
        return 2;
    }
    if (checar && saida != NULL) {
        fputs("basicgolc: --checar nao combina com -o\n", stderr);
        return 2;
    }

    e.arquivo = entrada;
    buf_ini(&e.saida);

    src = le_arquivo(entrada);
    tokeniza(src, &l);
    normaliza_apelidos(&l);
    coleta_enumeradores(entrada, &e);
    coleta_parametros(&e, &l);
    traduz(&e, &l);

    if (e.erros > 0) {
        fprintf(stderr, "basicgolc: %d erro(s), nenhum arquivo gerado\n", e.erros);
        return finaliza(&e, &l, src, 1);
    }

    if (checar) {
        fprintf(stderr, "basicgolc: %s ok (%d aviso(s))\n", entrada, e.avisos);
        return finaliza(&e, &l, src, 0);
    }

    f = (saida == NULL) ? stdout : fopen(saida, "wb");
    if (f == NULL) {
        bg_morte("nao consegui criar '%s'", saida);
    }
    if (fwrite(e.saida.dados, 1, e.saida.tam, f) != e.saida.tam) {
        if (saida != NULL) {
            fclose(f);
        }
        bg_morte("falha ao escrever '%s'", saida ? saida : "stdout");
    }
    /* Fechar (ou descarregar stdout) e onde o erro de E/S realmente aparece:
     * o retorno de fwrite nao pega disco cheio em fluxo bufferizado, entao
     * `basicgolc x.c > /dev/full` saia 0 e o C gerado se perdia em silencio.
     * O arquivo de saida pode ficar incompleto quando isto falha: o codigo 70
     * e o sinal; nao removemos o arquivo porque o destino pode ser um
     * dispositivo (ex.: /dev/null, /dev/full, um pipe nomeado). */
    rc = 0;
    if (saida != NULL) {
        if (fclose(f) != 0) {
            rc = -1;
        }
    } else if (fflush(stdout) != 0 || ferror(stdout)) {
        rc = -1;
    }
    if (rc != 0) {
        finaliza(&e, &l, src, 0);
        bg_morte("falha ao gravar a saida (%s)", saida ? saida : "stdout");
    }

    return finaliza(&e, &l, src, 0);
}
