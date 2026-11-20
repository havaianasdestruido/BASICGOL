/*
 * test_workspace.c — testes da workspace falsa (BG_USAR_WORKSPACE).
 *
 * Compilado com -DBG_USAR_WORKSPACE e ligado à biblioteca (a implementação vai
 * no archive como os demais módulos). O runner é tests/run_workspace.sh.
 */
#include <basicgol/filesystem.h>
#include <basicgol/workspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned verificacoes;
static unsigned falhas;

#define VERIFICA(condicao) do {                                            \
    ++verificacoes;                                                        \
    if (!(condicao)) {                                                     \
        ++falhas;                                                          \
        (void) fprintf(stderr, "FALHA %s:%d: %s\n", __FILE__, __LINE__,     \
                       #condicao);                                         \
    }                                                                      \
} while (0)

/* ------------------------------------------------------------ utilidades */

static char base[320];      /* caminho temporário único, sem arquivo */
static char snap[360];      /* base + ".bgws" */
static char temporario[380]; /* snap + ".tmp" */
static char saida[360];     /* base + ".saida" */

static void prepara_caminhos(void)
{
    if (bg_fs_make_temp(base, sizeof base) != 0) {
        (void) fprintf(stderr, "sem caminho temporário\n");
        exit(2);
    }
    remove(base);
    (void) snprintf(snap, sizeof snap, "%s.bgws", base);
    (void) snprintf(temporario, sizeof temporario, "%s.tmp", snap);
    (void) snprintf(saida, sizeof saida, "%s.saida", base);
}

static void limpa_caminhos(void)
{
    remove(snap);
    remove(temporario);
    remove(saida);
}

static void escreve_arquivo(const char *caminho, const char *conteudo)
{
    FILE *f = fopen(caminho, "wb");

    if (f == NULL) {
        (void) fprintf(stderr, "sem permissão de escrita em %s\n", caminho);
        exit(2);
    }
    (void) fputs(conteudo, f);
    (void) fclose(f);
}

static void le_arquivo(const char *caminho, char *buf, size_t capacidade)
{
    FILE *f = fopen(caminho, "rb");
    size_t n;

    if (f == NULL) {
        buf[0] = '\0';
        return;
    }
    n = fread(buf, 1, capacidade - 1u, f);
    if (ferror(f)) {
        buf[0] = '\0';
        (void) fclose(f);
        return;
    }
    buf[n] = '\0';
    (void) fclose(f);
}

struct ctx_lista {
    unsigned pastas;
    unsigned arquivos;
    unsigned vistos;
};

static int conta_entradas(const char *caminho, int eh_pasta, void *ctx)
{
    struct ctx_lista *l = ctx;

    (void) caminho;
    l->vistos++;
    if (eh_pasta) {
        l->pastas++;
    } else {
        l->arquivos++;
    }
    return 0;
}

static int para_na_segunda(const char *caminho, int eh_pasta, void *ctx)
{
    struct ctx_lista *l = ctx;

    (void) caminho;
    (void) eh_pasta;
    l->vistos++;
    return l->vistos >= 2;
}

/* ------------------------------------------------------------ testes */

static void testa_desligada_sem_workspace(void)
{
    VERIFICA(!bg_ws_esta_aberta());
    VERIFICA(!bg_ws_existe("qualquer"));
    VERIFICA(!bg_ws_salvar());
    VERIFICA(!bg_ws_enviar_texto("a.txt", "x"));
    VERIFICA(!bg_ws_remover("a.txt", true));
    VERIFICA(!bg_ws_limpar());
    VERIFICA(bg_ws_fechar()); /* nada aberto: nada a fazer */
    VERIFICA(!bg_ws_esta_aberta());
}

static void testa_memoria(void)
{
    char caminho[8] = "abc";
    size_t n;

    VERIFICA(bg_ws_abrir(NULL));
    VERIFICA(bg_ws_esta_aberta());
    VERIFICA(!bg_ws_caminho_persistencia(caminho, 0)); /* capacidade zero */
    VERIFICA(strcmp(caminho, "abc") == 0);            /* nada foi escrito */
    VERIFICA(bg_ws_caminho_persistencia(caminho, sizeof caminho));
    VERIFICA(strcmp(caminho, "") == 0); /* só memória */
    VERIFICA(!bg_ws_salvar());          /* sem arquivo de persistência */

    VERIFICA(bg_ws_enviar_texto("a.txt", "olar"));
    VERIFICA(bg_ws_existe("a.txt"));
    VERIFICA(!bg_ws_eh_pasta("a.txt"));
    n = 0u;
    VERIFICA(bg_ws_tamanho("a.txt", &n));
    VERIFICA(n == 4u);

    VERIFICA(bg_ws_enviar_bytes("pasta/b.bin", "\0\1\2", 3));
    VERIFICA(bg_ws_eh_pasta("pasta"));
    VERIFICA(bg_ws_existe("pasta"));
    VERIFICA(bg_ws_enviar_texto(".\\pasta//c.txt", "x"));
    VERIFICA(bg_ws_existe("pasta/c.txt"));
    VERIFICA(bg_ws_eh_pasta(""));
    VERIFICA(bg_ws_eh_pasta("/"));

    /* normalização e recusas */
    VERIFICA(!bg_ws_enviar_texto("../fora", "x"));
    VERIFICA(!bg_ws_enviar_texto("C:/dir/x", "x"));
    VERIFICA(!bg_ws_enviar_texto("", "x"));
    VERIFICA(!bg_ws_enviar_texto("/", "x"));
    VERIFICA(!bg_ws_enviar_texto("a.txt/este", "x"));   /* arquivo no caminho */
    VERIFICA(!bg_ws_enviar_texto("pasta/b.bin/filho", "x"));
    VERIFICA(!bg_ws_enviar_texto("pasta", "x"));        /* pasta virando arquivo */
    VERIFICA(!bg_ws_enviar_bytes("nulo.bin", NULL, 5));
    VERIFICA(bg_ws_enviar_bytes("pasta/b.bin", NULL, 0)); /* arquivo vazio */
    n = 99u;
    VERIFICA(bg_ws_tamanho("pasta/b.bin", &n));
    VERIFICA(n == 0u);
    VERIFICA(bg_ws_enviar_bytes("vazio.bin", NULL, 0));

    /* download */
    {
        char buf[16];
        size_t k;
        k = 99u;
        VERIFICA(bg_ws_baixar_bytes("a.txt", buf, sizeof buf, &k));
        VERIFICA(k == 4u);
        VERIFICA(memcmp(buf, "olar", 4u) == 0);
        k = 99u;
        VERIFICA(bg_ws_baixar_bytes("a.txt", buf, 2u, &k));
        VERIFICA(k == 2u);
        VERIFICA(memcmp(buf, "ol", 2u) == 0);
        k = 99u;
        VERIFICA(bg_ws_baixar_bytes("a.txt", NULL, 0u, &k));
        VERIFICA(k == 0u);
        k = 99u;
        VERIFICA(!bg_ws_baixar_bytes("a.txt", NULL, 4u, &k));
        VERIFICA(k == 99u);
        k = 99u;
        VERIFICA(!bg_ws_baixar_bytes("inexistente", buf, sizeof buf, &k));
        VERIFICA(k == 99u);
        VERIFICA(!bg_ws_baixar_bytes("pasta", buf, sizeof buf, &k));
    }

    /* listagem: raiz, pasta "pasta" (2 arquivos) e os 4 arquivos no total */
    {
        struct ctx_lista l;
        memset(&l, 0, sizeof l);
        VERIFICA(bg_ws_listar("", conta_entradas, &l));
        VERIFICA(l.pastas == 2u);     /* raiz + "pasta" */
        VERIFICA(l.arquivos == 4u);
        memset(&l, 0, sizeof l);
        VERIFICA(bg_ws_listar("pasta", conta_entradas, &l));
        VERIFICA(l.pastas == 1u);
        VERIFICA(l.arquivos == 2u);
        memset(&l, 0, sizeof l);
        VERIFICA(!bg_ws_listar("a.txt", conta_entradas, &l));
        VERIFICA(!bg_ws_listar("inexistente", conta_entradas, &l));
        memset(&l, 0, sizeof l);
        VERIFICA(!bg_ws_listar("", para_na_segunda, &l));
        VERIFICA(l.vistos == 2u);
        VERIFICA(!bg_ws_listar(NULL, conta_entradas, &l));
    }

    /* remoção */
    VERIFICA(!bg_ws_remover("pasta", false));
    VERIFICA(bg_ws_remover("pasta/b.bin", false));
    VERIFICA(!bg_ws_existe("pasta/b.bin"));
    VERIFICA(bg_ws_remover("pasta", true));
    VERIFICA(!bg_ws_eh_pasta("pasta"));
    VERIFICA(!bg_ws_remover("pasta", true));
    VERIFICA(!bg_ws_remover("", false)); /* raiz com conteúdo */
    VERIFICA(bg_ws_remover("", true));   /* esvazia */
    VERIFICA(bg_ws_existe(""));
    VERIFICA(!bg_ws_existe("a.txt"));
    VERIFICA(bg_ws_remover("", false));  /* raiz já vazia */
    VERIFICA(bg_ws_limpar());
    VERIFICA(bg_ws_fechar());
    VERIFICA(!bg_ws_esta_aberta());
}

static void testa_persistencia(void)
{
    static const unsigned char binario[] = { 0u, 255u, 1u, 2u, 3u };
    char buf[64];

    VERIFICA(bg_ws_abrir(snap));
    VERIFICA(bg_ws_caminho_persistencia(buf, sizeof buf));
    VERIFICA(strcmp(buf, snap) == 0);
    VERIFICA(bg_ws_enviar_texto("texto.txt", "conteudo"));
    VERIFICA(bg_ws_enviar_bytes("pasta/bin.bin", binario, sizeof binario));
    VERIFICA(bg_ws_salvar());
    VERIFICA(bg_ws_fechar());

    /* recarrega */
    VERIFICA(bg_ws_abrir(snap));
    {
        size_t n = 0;
        VERIFICA(bg_ws_tamanho("texto.txt", &n));
        VERIFICA(n == 8u);
    }
    {
        unsigned char copia[8];
        size_t n = 0;
        VERIFICA(bg_ws_baixar_bytes("pasta/bin.bin", copia, sizeof copia, &n));
        VERIFICA(n == sizeof binario);
        VERIFICA(memcmp(copia, binario, sizeof binario) == 0);
    }
    VERIFICA(bg_ws_baixar_disco("texto.txt", saida));
    le_arquivo(saida, buf, sizeof buf);
    VERIFICA(strcmp(buf, "conteudo") == 0);

    /* enviar do disco real */
    escreve_arquivo(saida, "do disco");
    VERIFICA(bg_ws_enviar_disco(saida, "disco.txt"));
    {
        size_t n = 0;
        VERIFICA(bg_ws_tamanho("disco.txt", &n));
        VERIFICA(n == 8u);
    }
    VERIFICA(!bg_ws_enviar_disco(saida, "disco.txt/filho")); /* conflito */
    VERIFICA(!bg_ws_enviar_disco("nao-existe.txt", "x.txt"));
    VERIFICA(bg_ws_salvar());
    VERIFICA(bg_ws_fechar());

    VERIFICA(bg_ws_abrir(snap));
    {
        size_t n = 0;
        VERIFICA(bg_ws_tamanho("disco.txt", &n));
        VERIFICA(n == 8u);
    }
    VERIFICA(bg_ws_fechar());
}

static void testa_snapshot_invalido(void)
{
    VERIFICA(bg_ws_abrir(snap));
    VERIFICA(bg_ws_enviar_texto("a.txt", "valor"));
    VERIFICA(bg_ws_salvar());
    VERIFICA(bg_ws_fechar());

    escreve_arquivo(snap, "BGWS1\nnão é um snapshot\n");
    VERIFICA(!bg_ws_abrir(snap));
    VERIFICA(!bg_ws_esta_aberta());
    VERIFICA(bg_ws_fechar());

    /* snapshot ausente começa vazio */
    remove(snap);
    VERIFICA(bg_ws_abrir(snap));
    VERIFICA(bg_ws_esta_aberta());
    VERIFICA(!bg_ws_existe("a.txt"));
    VERIFICA(bg_ws_fechar());
}

static void testa_falha_de_gravacao(void)
{
    char inexistente[420];

    (void) snprintf(inexistente, sizeof inexistente, "%s/sem-diretorio/x.bgws", base);
    VERIFICA(bg_ws_abrir(inexistente));
    VERIFICA(bg_ws_enviar_texto("a.txt", "valor"));
    VERIFICA(!bg_ws_salvar());      /* o diretório não existe */
    VERIFICA(!bg_ws_fechar());      /* gravação falhou, estado liberado */
    VERIFICA(!bg_ws_esta_aberta());
}

static void testa_limites(void)
{
    char nome[32];
    unsigned i;
    unsigned char *grande;

    VERIFICA(bg_ws_abrir(NULL));
    for (i = 0u; i < 4096u; i++) {
        (void) snprintf(nome, sizeof nome, "f%04u", i);
        if (!bg_ws_enviar_texto(nome, "x")) {
            break;
        }
    }
    VERIFICA(i == 4096u);
    VERIFICA(!bg_ws_enviar_texto("f4096", "x"));   /* limite de arquivos */
    VERIFICA(bg_ws_limpar());

    grande = calloc(64u * 1024u * 1024u + 1u, 1u);
    VERIFICA(grande != NULL);
    if (grande != NULL) {
        VERIFICA(bg_ws_enviar_bytes("grande.bin", grande, 64u * 1024u * 1024u));
        VERIFICA(!bg_ws_enviar_bytes("mais.bin", grande, 1u)); /* 64 MiB + 1 */
        {
            size_t n = 0;
            VERIFICA(bg_ws_tamanho("grande.bin", &n));
            VERIFICA(n == 64u * 1024u * 1024u);
        }
        free(grande);
    }
    VERIFICA(bg_ws_limpar());
    VERIFICA(bg_ws_fechar());
}

int main(void)
{
    prepara_caminhos();
    testa_desligada_sem_workspace();
    testa_memoria();
    testa_persistencia();
    testa_snapshot_invalido();
    testa_falha_de_gravacao();
    testa_limites();
    limpa_caminhos();

    if (falhas != 0u) {
        (void) fprintf(stderr, "workspace: %u de %u verificacoes falharam\n",
                       falhas, verificacoes);
        return 1;
    }
    (void) printf("workspace: %u verificacoes aprovadas\n", verificacoes);
    return 0;
}
