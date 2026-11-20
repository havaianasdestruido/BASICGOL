/*
 * basicgol/workspace.c — workspace falsa com persistência (OPCIONAL).
 *
 * O estado é uma lista plana de arquivos com caminho virtual normalizado
 * ("pasta/arquivo"); pastas são implícitas (um caminho "a/b" implica a pasta
 * "a"). Isso basta para o contrato do módulo e mantém as operações simples e
 * determinísticas. A persistência é um arquivo único, de cabeçalho `BGWS1`,
 * com o conteúdo em base64 — legível o bastante para inspeção e seguro para
 * bytes arbitrários.
 *
 * A implementação vai no archive (o Makefile soma o diretório `src/workspace/`), para
 * que ligar um programa não exija rebuild com flag especial. O que o cabeçalho
 * declara — e portanto o que existe para o programa — continua dependendo de
 * `BG_USAR_WORKSPACE`; definimos o macro aqui somente para enxergar as
 * próprias declarações.
 *
 * O arquivo mora fora de `src/biblioteca/` de propósito: o playground compila
 * esse diretório inteiro em cada programa e a workspace é opt-in, então não
 * deve pesar (nem rodar) onde não foi pedida.
 */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BG_USAR_WORKSPACE
#define BG_USAR_WORKSPACE
#endif
#include <basicgol/workspace.h>

/* ------------------------------------------------------------ constantes */

#define BG_WS_MAGICO        "BGWS1"
#define BG_WS_MAX_CAMINHO   4096u
#define BG_WS_MAX_ARQUIVOS  4096u
#define BG_WS_MAX_TOTAL     (64u * 1024u * 1024u)
#define BG_WS_LINHA_B64     76u
#define BG_WS_SUFIXO_TMP    ".tmp"

/* ------------------------------------------------------------ estado */

typedef struct {
    char *nome;              /* caminho virtual normalizado, sem NUL extra */
    unsigned char *dados;    /* conteúdo; NULL só quando tamanho == 0 */
    size_t tamanho;
} bg_ws_item;

struct bg_ws {
    char *persistencia;      /* caminho real do snapshot; NULL = só memória */
    bg_ws_item *itens;
    size_t quantidade;
    size_t capacidade;
    size_t total;            /* soma dos tamanhos, para o limite de 64 MiB */
};

static struct bg_ws *g_ws = NULL;

/* ------------------------------------------------------------ utilidades */

static char *copia_dup(const char *texto)
{
    size_t n = strlen(texto);
    char *copia = malloc(n + 1u);

    if (copia == NULL) {
        return NULL;
    }
    memcpy(copia, texto, n + 1u);
    return copia;
}

static char *junta_sufixo(const char *caminho, const char *sufixo)
{
    size_t nc = strlen(caminho);
    size_t ns = strlen(sufixo);
    char *junto = malloc(nc + ns + 1u);

    if (junto == NULL) {
        return NULL;
    }
    memcpy(junto, caminho, nc);
    memcpy(junto + nc, sufixo, ns + 1u);
    return junto;
}

static void libera_item(bg_ws_item *item)
{
    free(item->nome);
    free(item->dados);
    item->nome = NULL;
    item->dados = NULL;
    item->tamanho = 0u;
}

static void libera_ws(struct bg_ws *w)
{
    size_t i;

    if (w == NULL) {
        return;
    }
    for (i = 0; i < w->quantidade; i++) {
        libera_item(&w->itens[i]);
    }
    free(w->itens);
    free(w->persistencia);
    free(w);
}

/* Tamanho máximo de um componente de caminho virtual. */
static bool componente_valido(const char *c, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        unsigned char ch = (unsigned char) c[i];

        /* ':' separa unidade em caminhos de Windows e não faz parte do
         * vocabulário da workspace; bytes de controle quebram o snapshot. */
        if (ch < 0x20u || ch == 0x7fu || ch == ':') {
            return false;
        }
    }
    return true;
}

/*
 * Normaliza um caminho virtual para malloc'd: '/' como separador, sem partes
 * vazias nem ".", recusando "..", ":" e absolutos. Caminho vazio é a raiz.
 * Devolve NULL em caminho inválido ou falta de memória.
 */
static char *normaliza_dup(const char *entrada)
{
    size_t n;
    size_t i = 0u;
    size_t j = 0u;
    char *saida;

    if (entrada == NULL) {
        return NULL;
    }
    n = strlen(entrada);
    saida = malloc(n + 1u);
    if (saida == NULL) {
        return NULL;
    }
    while (i < n) {
        size_t inicio;
        size_t fim;

        while (i < n && (entrada[i] == '/' || entrada[i] == '\\')) {
            i++;
        }
        if (i >= n) {
            break;
        }
        inicio = i;
        while (i < n && entrada[i] != '/' && entrada[i] != '\\') {
            i++;
        }
        fim = i;
        if (fim - inicio == 1u && entrada[inicio] == '.') {
            continue;
        }
        if (fim - inicio == 2u && entrada[inicio] == '.'
            && entrada[inicio + 1u] == '.') {
            free(saida);
            return NULL;
        }
        if (!componente_valido(entrada + inicio, fim - inicio)) {
            free(saida);
            return NULL;
        }
        if (j > 0u) {
            saida[j++] = '/';
        }
        memcpy(saida + j, entrada + inicio, fim - inicio);
        j += fim - inicio;
    }
    if (j > BG_WS_MAX_CAMINHO) {
        free(saida);
        return NULL;
    }
    saida[j] = '\0';
    return saida;
}

/* `nome` é descendente direto de `pasta` (ou seja, `pasta/nome`). */
static bool e_pasta_de(const char *pasta, const char *nome)
{
    size_t lp = strlen(pasta);

    return strncmp(nome, pasta, lp) == 0 && nome[lp] == '/';
}

static int encontra(struct bg_ws *w, const char *nome)
{
    size_t i;

    for (i = 0; i < w->quantidade; i++) {
        if (strcmp(w->itens[i].nome, nome) == 0) {
            return (int) i;
        }
    }
    return -1;
}

/* True se há algum arquivo sob `nome` (raiz conta sempre como pasta). */
static bool nome_e_pasta(struct bg_ws *w, const char *nome)
{
    size_t i;

    if (nome[0] == '\0') {
        return true;
    }
    for (i = 0; i < w->quantidade; i++) {
        if (e_pasta_de(nome, w->itens[i].nome)) {
            return true;
        }
    }
    return false;
}

/* True se alguma pasta de `nome` já existe como arquivo. */
static bool tem_pasta_arquivo(struct bg_ws *w, const char *nome)
{
    size_t i;

    for (i = 0; i < w->quantidade; i++) {
        if (e_pasta_de(w->itens[i].nome, nome)) {
            return true;
        }
    }
    return false;
}

static bool sob_raiz(const char *raiz, const char *nome)
{
    return raiz[0] == '\0' || e_pasta_de(raiz, nome);
}

static bool garante_espaco(struct bg_ws *w)
{
    size_t nova;
    bg_ws_item *itens;

    if (w->quantidade < w->capacidade) {
        return true;
    }
    nova = w->capacidade == 0u ? 8u : w->capacidade * 2u;
    if (nova > BG_WS_MAX_ARQUIVOS) {
        nova = BG_WS_MAX_ARQUIVOS;
    }
    if (nova <= w->capacidade) {
        return false;
    }
    itens = realloc(w->itens, nova * sizeof *itens);
    if (itens == NULL) {
        return false;
    }
    w->itens = itens;
    w->capacidade = nova;
    return true;
}

static void remove_item(struct bg_ws *w, size_t indice)
{
    size_t tamanho = w->itens[indice].tamanho;

    libera_item(&w->itens[indice]);
    w->total -= tamanho;
    memmove(&w->itens[indice], &w->itens[indice + 1u],
            (w->quantidade - indice - 1u) * sizeof *w->itens);
    w->quantidade -= 1u;
}

/*
 * Guarda `nome` (normalizado) com o conteúdo dado. Substitui arquivo já
 * existente; recusa se `nome` é raiz, se uma pasta viraria arquivo ou se um
 * arquivo viraria pasta. Devolve false em limite atingido ou falta de memória
 * (sem alterar a workspace).
 */
static bool insere(struct bg_ws *w, const char *nome,
                   const unsigned char *dados, size_t n)
{
    int indice = encontra(w, nome);
    unsigned char *copia;
    size_t total;

    if (nome[0] == '\0') {
        return false;
    }
    if (indice < 0) {
        if (nome_e_pasta(w, nome) || tem_pasta_arquivo(w, nome)) {
            return false;
        }
        if (w->quantidade >= BG_WS_MAX_ARQUIVOS) {
            return false;
        }
        total = w->total + n;
    } else {
        total = w->total - w->itens[indice].tamanho + n;
    }
    if (total > BG_WS_MAX_TOTAL) {
        return false;
    }
    copia = malloc(n > 0u ? n : 1u);
    if (copia == NULL) {
        return false;
    }
    if (n > 0u) {
        memcpy(copia, dados, n);
    }
    if (indice >= 0) {
        free(w->itens[indice].dados);
        w->itens[indice].dados = copia;
        w->itens[indice].tamanho = n;
        w->total = total;
        return true;
    }
    if (!garante_espaco(w)) {
        free(copia);
        return false;
    }
    w->itens[w->quantidade].nome = copia_dup(nome);
    if (w->itens[w->quantidade].nome == NULL) {
        free(copia);
        return false;
    }
    w->itens[w->quantidade].dados = copia;
    w->itens[w->quantidade].tamanho = n;
    w->quantidade += 1u;
    w->total = total;
    return true;
}

/* ------------------------------------------------------------ base64 */

static const char bg_ws_b64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char *b64_codifica(const unsigned char *dados, size_t n, size_t *saida)
{
    size_t i = 0u;
    size_t j = 0u;
    char *b = malloc(4u * ((n + 2u) / 3u) + 1u);

    if (b == NULL) {
        return NULL;
    }
    while (i + 2u < n) {
        unsigned v = ((unsigned) dados[i] << 16)
                   | ((unsigned) dados[i + 1u] << 8)
                   | (unsigned) dados[i + 2u];

        b[j++] = bg_ws_b64[(v >> 18) & 63u];
        b[j++] = bg_ws_b64[(v >> 12) & 63u];
        b[j++] = bg_ws_b64[(v >> 6) & 63u];
        b[j++] = bg_ws_b64[v & 63u];
        i += 3u;
    }
    if (i < n) {
        unsigned v = (unsigned) dados[i] << 16u;
        size_t resto = n - i;

        if (resto == 2u) {
            v |= (unsigned) dados[i + 1u] << 8u;
        }
        b[j++] = bg_ws_b64[(v >> 18) & 63u];
        b[j++] = bg_ws_b64[(v >> 12) & 63u];
        b[j++] = resto == 2u ? bg_ws_b64[(v >> 6) & 63u] : '=';
        b[j++] = '=';
    }
    b[j] = '\0';
    *saida = j;
    return b;
}

static int b64_valor(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    if (c == '+') {
        return 62;
    }
    if (c == '/') {
        return 63;
    }
    return -1;
}

/* Decodifica base64 estrito (com padding só no fim). NULL em texto inválido. */
static unsigned char *b64_decodifica(const char *texto, size_t n, size_t *saida)
{
    unsigned char *b;
    size_t i;
    size_t j = 0u;
    size_t pad = 0u;

    if (n == 0u) {
        b = malloc(1u);
        if (b == NULL) {
            return NULL;
        }
        *saida = 0u;
        return b;
    }
    if (n % 4u != 0u) {
        return NULL;
    }
    if (texto[n - 1u] == '=') {
        pad = 1u;
        if (texto[n - 2u] == '=') {
            pad = 2u;
        }
    }
    for (i = 0u; i + pad < n; i += 1u) {
        if (b64_valor(texto[i]) < 0) {
            return NULL;
        }
    }
    for (i = n - pad; i < n; i += 1u) {
        if (texto[i] != '=') {
            return NULL;
        }
    }
    b = malloc(n / 4u * 3u + 1u);
    if (b == NULL) {
        return NULL;
    }
    for (i = 0u; i < n; i += 4u) {
        int a = b64_valor(texto[i]);
        int c = b64_valor(texto[i + 1u]);
        int d = b64_valor(texto[i + 2u]);
        int e = b64_valor(texto[i + 3u]);

        b[j++] = (unsigned char) ((a << 2) | (c >> 4));
        if (texto[i + 2u] == '=') {
            continue;
        }
        b[j++] = (unsigned char) (((c & 15) << 4) | (d >> 2));
        if (texto[i + 3u] == '=') {
            continue;
        }
        b[j++] = (unsigned char) (((d & 3) << 6) | e);
    }
    *saida = j;
    return b;
}

/* ------------------------------------------------------------ snapshot */

/* Linha do snapshot sem CR/LF; false em EOF, estouro ou erro. */
static bool le_linha(FILE *f, char *buf, size_t capacidade)
{
    size_t i = 0u;
    int c;

    for (;;) {
        c = fgetc(f);
        if (c == EOF) {
            if (i == 0u) {
                return false;
            }
            break;
        }
        if (c == '\n') {
            break;
        }
        if (c == '\r') {
            continue;
        }
        if (i + 1u >= capacidade) {
            return false;
        }
        buf[i++] = (char) c;
    }
    buf[i] = '\0';
    return true;
}

static bool le_exato(FILE *f, char *buf, size_t n)
{
    size_t i;

    for (i = 0u; i < n; i++) {
        int c = fgetc(f);

        if (c == EOF) {
            return false;
        }
        buf[i] = (char) c;
    }
    return true;
}

/* Base64 do snapshot: lê exatos `n` caracteres, saltando as quebras de linha. */
static bool le_b64(FILE *f, char *buf, size_t n)
{
    size_t i = 0u;
    int c;

    while (i < n) {
        c = fgetc(f);
        if (c == EOF) {
            return false;
        }
        if (c == '\n' || c == '\r') {
            continue;
        }
        buf[i++] = (char) c;
    }
    return true;
}

/* Consome o resto da linha (o le_b64 pára exatamente no último caractere). */
static void descarta_linha(FILE *f)
{
    int c;

    while ((c = fgetc(f)) != EOF && c != '\n') {
    }
}

static bool texto_para_ulong(const char *texto, unsigned long *saida)
{
    unsigned long v = 0ul;
    const char *p = texto;

    if (*p == '\0') {
        return false;
    }
    while (*p >= '0' && *p <= '9') {
        unsigned long digito = (unsigned long) (*p - '0');

        if (v > (ULONG_MAX - digito) / 10ul) {
            return false;
        }
        v = v * 10ul + digito;
        p++;
    }
    if (*p != '\0') {
        return false;
    }
    *saida = v;
    return true;
}

/* Um registro "N …/D …" do snapshot; insere na workspace. */
static bool le_um_item(FILE *f, struct bg_ws *w)
{
    char linha[64];
    char *nome = NULL;
    char *b64 = NULL;
    unsigned char *dados = NULL;
    unsigned long tam_nome;
    unsigned long tam_dados;
    unsigned long tam_b64;
    size_t ndados = 0u;
    bool ok = false;

    if (!le_linha(f, linha, sizeof linha)
        || sscanf(linha, "N %lu", &tam_nome) != 1) {
        return false;
    }
    if (tam_nome == 0ul || tam_nome > (unsigned long) BG_WS_MAX_CAMINHO) {
        return false;
    }
    nome = malloc((size_t) tam_nome + 1u);
    if (nome == NULL) {
        return false;
    }
    if (!le_exato(f, nome, (size_t) tam_nome) || fgetc(f) != '\n') {
        goto fim;
    }
    nome[tam_nome] = '\0';
    if (!le_linha(f, linha, sizeof linha)
        || sscanf(linha, "D %lu %lu", &tam_dados, &tam_b64) != 2) {
        goto fim;
    }
    /* o tamanho em base64 é função do tamanho em bytes: desconfia de corrupto */
    if (tam_dados > (unsigned long) BG_WS_MAX_TOTAL
        || tam_b64 != 4ul * ((tam_dados + 2ul) / 3ul)) {
        goto fim;
    }
    b64 = malloc((size_t) tam_b64 + 1u);
    if (b64 == NULL) {
        goto fim;
    }
    if (!le_b64(f, b64, (size_t) tam_b64)) {
        goto fim;
    }
    descarta_linha(f);
    b64[tam_b64] = '\0';
    dados = b64_decodifica(b64, (size_t) tam_b64, &ndados);
    if (dados == NULL || ndados != (size_t) tam_dados) {
        goto fim;
    }
    if (!insere(w, nome, dados, ndados)) {
        goto fim;
    }
    ok = true;
fim:
    free(nome);
    free(b64);
    free(dados);
    return ok;
}

/*
 * Carrega o snapshot em `caminho`. Arquivo ausente é workspace vazia; arquivo
 * presente e corrompido devolve false.
 */
static bool carrega_do_disco(struct bg_ws *w, const char *caminho)
{
    FILE *f;
    char linha[64];
    unsigned long quantidade;
    unsigned long i;
    bool ok = false;

    f = fopen(caminho, "rb");
    if (f == NULL) {
        return errno == ENOENT;
    }
    if (!le_linha(f, linha, sizeof linha) || strcmp(linha, BG_WS_MAGICO) != 0) {
        goto fim;
    }
    if (!le_linha(f, linha, sizeof linha)
        || !texto_para_ulong(linha, &quantidade)
        || quantidade > (unsigned long) BG_WS_MAX_ARQUIVOS) {
        goto fim;
    }
    for (i = 0ul; i < quantidade; i++) {
        if (!le_um_item(f, w)) {
            goto fim;
        }
    }
    if (fgetc(f) != EOF) {
        goto fim; /* sobra lixo depois do último registro */
    }
    ok = true;
fim:
    fclose(f);
    return ok;
}

/* Grava o snapshot em `caminho` de forma atômica (arquivo temporário + rename). */
static bool grava_no_disco(struct bg_ws *w, const char *caminho)
{
    char *temporario = junta_sufixo(caminho, BG_WS_SUFIXO_TMP);
    FILE *f;
    size_t i;
    bool ok = false;

    if (temporario == NULL) {
        return false;
    }
    f = fopen(temporario, "wb");
    if (f == NULL) {
        free(temporario);
        return false;
    }
    if (fputs(BG_WS_MAGICO "\n", f) < 0
        || fprintf(f, "%lu\n", (unsigned long) w->quantidade) < 0) {
        goto fim;
    }
    for (i = 0; i < w->quantidade; i++) {
        bg_ws_item *item = &w->itens[i];
        size_t tamanho_nome = strlen(item->nome);
        size_t nb64 = 0u;
        size_t j;
        char *b64 = b64_codifica(item->dados, item->tamanho, &nb64);

        if (b64 == NULL) {
            goto fim;
        }
        if (fprintf(f, "N %lu\n", (unsigned long) tamanho_nome) < 0
            || fwrite(item->nome, 1u, tamanho_nome, f) != tamanho_nome
            || fputc('\n', f) == EOF) {
            free(b64);
            goto fim;
        }
        if (fprintf(f, "D %lu %lu\n", (unsigned long) item->tamanho,
                    (unsigned long) nb64) < 0) {
            free(b64);
            goto fim;
        }
        if (nb64 == 0u) {
            if (fputc('\n', f) == EOF) {
                free(b64);
                goto fim;
            }
        } else {
            for (j = 0u; j < nb64; j += BG_WS_LINHA_B64) {
                size_t resto = nb64 - j < BG_WS_LINHA_B64
                             ? nb64 - j : BG_WS_LINHA_B64;

                if (fwrite(b64 + j, 1u, resto, f) != resto
                    || fputc('\n', f) == EOF) {
                    free(b64);
                    goto fim;
                }
            }
        }
        free(b64);
    }
    if (ferror(f)) {
        goto fim;
    }
    if (fclose(f) != 0) {
        f = NULL;
        goto fim;
    }
    f = NULL;
    if (rename(temporario, caminho) != 0) {
        goto fim;
    }
    ok = true;
fim:
    if (f != NULL) {
        (void) fclose(f);
    }
    if (!ok) {
        (void) remove(temporario);
    }
    free(temporario);
    return ok;
}

/* ------------------------------------------------------------ listagem */

struct lista_ctx {
    int (*cb)(const char *, int, void *);
    void *ctx;
    char **vistas;       /* pastas já anunciadas (cópias) */
    size_t quantas;
    size_t capacidade;
};

static bool lista_reserva(struct lista_ctx *lc)
{
    size_t nova;
    char **vistas;

    if (lc->quantas < lc->capacidade) {
        return true;
    }
    nova = lc->capacidade == 0u ? 8u : lc->capacidade * 2u;
    vistas = realloc(lc->vistas, nova * sizeof *vistas);
    if (vistas == NULL) {
        return false;
    }
    lc->vistas = vistas;
    lc->capacidade = nova;
    return true;
}

static bool lista_guarda(struct lista_ctx *lc, const char *caminho)
{
    char *copia;

    if (!lista_reserva(lc)) {
        return false;
    }
    copia = copia_dup(caminho);
    if (copia == NULL) {
        return false;
    }
    lc->vistas[lc->quantas++] = copia;
    return true;
}

static bool lista_ja_vista(struct lista_ctx *lc, const char *caminho)
{
    size_t i;

    for (i = 0u; i < lc->quantas; i++) {
        if (strcmp(lc->vistas[i], caminho) == 0) {
            return true;
        }
    }
    return false;
}

static void lista_libera(struct lista_ctx *lc)
{
    size_t i;

    for (i = 0u; i < lc->quantas; i++) {
        free(lc->vistas[i]);
    }
    free(lc->vistas);
    lc->vistas = NULL;
    lc->quantas = 0u;
    lc->capacidade = 0u;
}

/* Anuncia `prefixo` como pasta (com deduplicação). False se interromper. */
static bool lista_anuncia_pasta(struct lista_ctx *lc, const char *prefixo)
{
    if (lista_ja_vista(lc, prefixo)) {
        return true;
    }
    if (!lista_guarda(lc, prefixo)) {
        return false;
    }
    return lc->cb(prefixo, 1, lc->ctx) == 0;
}

/* Anuncia as pastas de `item` que estão sob `raiz`. */
static bool lista_pastas_de(struct lista_ctx *lc, const char *raiz,
                             const char *item)
{
    size_t lr = strlen(raiz);
    const char *p = item;

    while ((p = strchr(p, '/')) != NULL) {
        size_t tamanho = (size_t) (p - item);
        char *prefixo;

        p++;
        if (raiz[0] != '\0' && tamanho <= lr) {
            continue;
        }
        prefixo = malloc(tamanho + 1u);
        if (prefixo == NULL) {
            return false;
        }
        memcpy(prefixo, item, tamanho);
        prefixo[tamanho] = '\0';
        if (!lista_anuncia_pasta(lc, prefixo)) {
            free(prefixo);
            return false;
        }
        free(prefixo);
    }
    return true;
}

/* ------------------------------------------------------------ público */

bool bg_ws_abrir(const char *arquivo)
{
    struct bg_ws *w;

    if (g_ws != NULL) {
        return false;
    }
    w = calloc(1u, sizeof *w);
    if (w == NULL) {
        return false;
    }
    if (arquivo != NULL && arquivo[0] != '\0') {
        w->persistencia = copia_dup(arquivo);
        if (w->persistencia == NULL || !carrega_do_disco(w, w->persistencia)) {
            libera_ws(w);
            return false;
        }
    }
    g_ws = w;
    return true;
}

bool bg_ws_salvar(void)
{
    if (g_ws == NULL || g_ws->persistencia == NULL) {
        return false;
    }
    return grava_no_disco(g_ws, g_ws->persistencia);
}

bool bg_ws_fechar(void)
{
    bool ok = true;

    if (g_ws == NULL) {
        return true;
    }
    if (g_ws->persistencia != NULL) {
        ok = grava_no_disco(g_ws, g_ws->persistencia);
    }
    libera_ws(g_ws);
    g_ws = NULL;
    return ok;
}

bool bg_ws_esta_aberta(void)
{
    return g_ws != NULL;
}

bool bg_ws_caminho_persistencia(char *destino, size_t capacidade)
{
    const char *caminho = "";

    if (destino == NULL || capacidade == 0u) {
        return false;
    }
    if (g_ws == NULL) {
        destino[0] = '\0';
        return false;
    }
    if (g_ws->persistencia != NULL) {
        caminho = g_ws->persistencia;
    }
    if (strlen(caminho) + 1u > capacidade) {
        destino[0] = '\0';
        return false;
    }
    memcpy(destino, caminho, strlen(caminho) + 1u);
    return true;
}

bool bg_ws_existe(const char *caminho)
{
    char *nome;
    bool achou;

    if (g_ws == NULL) {
        return false;
    }
    nome = normaliza_dup(caminho);
    if (nome == NULL) {
        return false;
    }
    achou = nome[0] == '\0' || encontra(g_ws, nome) >= 0
            || nome_e_pasta(g_ws, nome);
    free(nome);
    return achou;
}

bool bg_ws_eh_pasta(const char *caminho)
{
    char *nome;
    bool achou;

    if (g_ws == NULL) {
        return false;
    }
    nome = normaliza_dup(caminho);
    if (nome == NULL) {
        return false;
    }
    achou = nome_e_pasta(g_ws, nome);
    free(nome);
    return achou;
}

bool bg_ws_tamanho(const char *caminho, size_t *bytes)
{
    char *nome;
    int indice;

    if (g_ws == NULL || bytes == NULL) {
        return false;
    }
    nome = normaliza_dup(caminho);
    if (nome == NULL) {
        return false;
    }
    indice = encontra(g_ws, nome);
    free(nome);
    if (indice < 0) {
        return false;
    }
    *bytes = g_ws->itens[indice].tamanho;
    return true;
}

bool bg_ws_listar(const char *pasta,
                  int (*cb)(const char *caminho, int eh_pasta, void *ctx),
                  void *ctx)
{
    struct lista_ctx lc;
    char *raiz;
    size_t i;
    bool ok = true;

    if (g_ws == NULL || cb == NULL) {
        return false;
    }
    raiz = normaliza_dup(pasta);
    if (raiz == NULL) {
        return false;
    }
    if (raiz[0] != '\0' && !nome_e_pasta(g_ws, raiz)) {
        free(raiz);
        return false;
    }
    memset(&lc, 0, sizeof lc);
    lc.cb = cb;
    lc.ctx = ctx;
    if (cb(raiz, 1, ctx) != 0 || !lista_guarda(&lc, raiz)) {
        lista_libera(&lc);
        free(raiz);
        return false;
    }
    for (i = 0u; i < g_ws->quantidade && ok; i++) {
        if (sob_raiz(raiz, g_ws->itens[i].nome)) {
            ok = lista_pastas_de(&lc, raiz, g_ws->itens[i].nome);
        }
    }
    for (i = 0u; i < g_ws->quantidade && ok; i++) {
        if (sob_raiz(raiz, g_ws->itens[i].nome)
            && cb(g_ws->itens[i].nome, 0, ctx) != 0) {
            ok = false;
        }
    }
    lista_libera(&lc);
    free(raiz);
    return ok;
}

bool bg_ws_enviar_bytes(const char *caminho_ws, const void *dados, size_t bytes)
{
    char *nome;

    if (g_ws == NULL) {
        return false;
    }
    if (dados == NULL && bytes > 0u) {
        return false;
    }
    nome = normaliza_dup(caminho_ws);
    if (nome == NULL) {
        return false;
    }
    if (!insere(g_ws, nome, (const unsigned char *) (dados != NULL ? dados : ""),
                bytes)) {
        free(nome);
        return false;
    }
    free(nome);
    return true;
}

bool bg_ws_enviar_texto(const char *caminho_ws, const char *texto)
{
    return bg_ws_enviar_bytes(caminho_ws, texto,
                              texto != NULL ? strlen(texto) : 0u);
}

bool bg_ws_enviar_disco(const char *caminho_real, const char *caminho_ws)
{
    unsigned char *dados = NULL;
    char *nome;
    FILE *f;
    long tamanho;
    size_t lido;
    bool ok;

    if (g_ws == NULL || caminho_real == NULL) {
        return false;
    }
    f = fopen(caminho_real, "rb");
    if (f == NULL) {
        return false;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (tamanho = ftell(f)) < 0
        || fseek(f, 0, SEEK_SET) != 0) {
        (void) fclose(f);
        return false;
    }
    dados = malloc((size_t) tamanho + 1u);
    if (dados == NULL) {
        (void) fclose(f);
        return false;
    }
    lido = tamanho > 0 ? fread(dados, 1u, (size_t) tamanho, f) : 0u;
    if (lido != (size_t) tamanho || ferror(f)) {
        free(dados);
        (void) fclose(f);
        return false;
    }
    if (fclose(f) != 0) {
        free(dados);
        return false;
    }
    nome = normaliza_dup(caminho_ws);
    if (nome == NULL) {
        free(dados);
        return false;
    }
    ok = insere(g_ws, nome, dados, (size_t) tamanho);
    free(nome);
    free(dados);
    return ok;
}

bool bg_ws_baixar_disco(const char *caminho_ws, const char *caminho_real)
{
    char *nome;
    int indice;
    FILE *f;
    size_t n;
    bool ok;

    if (g_ws == NULL || caminho_real == NULL) {
        return false;
    }
    nome = normaliza_dup(caminho_ws);
    if (nome == NULL) {
        return false;
    }
    indice = encontra(g_ws, nome);
    free(nome);
    if (indice < 0) {
        return false;
    }
    n = g_ws->itens[indice].tamanho;
    f = fopen(caminho_real, "wb");
    if (f == NULL) {
        return false;
    }
    ok = n == 0u || fwrite(g_ws->itens[indice].dados, 1u, n, f) == n;
    if (fclose(f) != 0) {
        ok = false;
    }
    return ok;
}

bool bg_ws_baixar_bytes(const char *caminho_ws, void *saida, size_t capacidade,
                        size_t *copiados)
{
    char *nome;
    int indice;
    size_t n;

    if (g_ws == NULL || copiados == NULL) {
        return false;
    }
    if (saida == NULL && capacidade > 0u) {
        return false;
    }
    nome = normaliza_dup(caminho_ws);
    if (nome == NULL) {
        return false;
    }
    indice = encontra(g_ws, nome);
    free(nome);
    if (indice < 0) {
        return false;
    }
    n = g_ws->itens[indice].tamanho;
    if (n > capacidade) {
        n = capacidade;
    }
    if (n > 0u) {
        memcpy(saida, g_ws->itens[indice].dados, n);
    }
    *copiados = n;
    return true;
}

bool bg_ws_remover(const char *caminho, bool recursivo)
{
    char *nome;
    int indice;
    size_t i;
    bool ok;

    if (g_ws == NULL) {
        return false;
    }
    nome = normaliza_dup(caminho);
    if (nome == NULL) {
        return false;
    }
    if (nome[0] == '\0') {
        /* a raiz é pasta: sem recursivo, só esvazia se já estiver vazia */
        if (!recursivo && g_ws->quantidade > 0u) {
            free(nome);
            return false;
        }
        ok = bg_ws_limpar();
        free(nome);
        return ok;
    }
    indice = encontra(g_ws, nome);
    if (indice >= 0) {
        remove_item(g_ws, (size_t) indice);
        free(nome);
        return true;
    }
    if (!nome_e_pasta(g_ws, nome) || !recursivo) {
        free(nome);
        return false;
    }
    i = 0u;
    while (i < g_ws->quantidade) {
        if (e_pasta_de(nome, g_ws->itens[i].nome)) {
            remove_item(g_ws, i);
        } else {
            i++;
        }
    }
    free(nome);
    return true;
}

bool bg_ws_limpar(void)
{
    size_t i;

    if (g_ws == NULL) {
        return false;
    }
    for (i = 0u; i < g_ws->quantidade; i++) {
        libera_item(&g_ws->itens[i]);
    }
    g_ws->quantidade = 0u;
    g_ws->total = 0u;
    return true;
}
