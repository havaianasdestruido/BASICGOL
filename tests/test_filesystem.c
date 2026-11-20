/* Testes da API de filesystem. Executar em um diretório temporário vazio. */
#include <basicgol/filesystem.h>

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define CRIA_DIR(caminho) _mkdir(caminho)
#else
#include <sys/stat.h>
#define CRIA_DIR(caminho) mkdir(caminho, 0700)
#endif

static unsigned verificacoes;
static unsigned falhas;

#define VERIFICA(condicao) do { \
    ++verificacoes; \
    if (!(condicao)) { \
        ++falhas; \
        fprintf(stderr, "FALHA %s:%d: %s\n", __FILE__, __LINE__, #condicao); \
    } \
} while (0)

/* Conteúdo maior que o buffer interno de cópia, incluindo NUL e CR/LF. */
static int grava_binario(const char *nome, size_t tamanho)
{
    FILE *f = fopen(nome, "wb");
    size_t i;
    if (!f) return -1;
    for (i = 0; i < tamanho; ++i) {
        if (fputc((int)(i % 256u), f) == EOF) {
            fclose(f);
            return -1;
        }
    }
    return fclose(f);
}

static int confere_binario(const char *nome, size_t tamanho)
{
    FILE *f = fopen(nome, "rb");
    size_t i;
    int ok = 1;
    if (!f) return 0;
    for (i = 0; i < tamanho; ++i) {
        if (fgetc(f) != (int)(i % 256u)) {
            ok = 0;
            break;
        }
    }
    if (ok && (fgetc(f) != EOF || ferror(f))) ok = 0;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

struct caminhada {
    unsigned raiz, sub, arquivo, profundo, inesperado, total;
    int parar;
};

static int visita(const char *caminho, int eh_dir, void *ctx)
{
    struct caminhada *estado = (struct caminhada *)ctx;
    char normalizado[256];
    size_t i, n = strlen(caminho);

    ++estado->total;
    if (estado->parar && (int)estado->total >= estado->parar) return 1;
    if (n >= sizeof normalizado) {
        ++estado->inesperado;
        return 0;
    }
    for (i = 0; i <= n; ++i)
        normalizado[i] = caminho[i] == '\\' ? '/' : caminho[i];
    /* Uma barra final na raiz não deve introduzir barras duplicadas. */
    if (strcmp(normalizado, "arvore") == 0 || strcmp(normalizado, "arvore/") == 0)
        estado->raiz += eh_dir == 1 ? 1u : 100u;
    else if (strcmp(normalizado, "arvore/sub") == 0)
        estado->sub += eh_dir == 1 ? 1u : 100u;
    else if (strcmp(normalizado, "arvore/a.bin") == 0)
        estado->arquivo += eh_dir == 0 ? 1u : 100u;
    else if (strcmp(normalizado, "arvore/sub/b.bin") == 0)
        estado->profundo += eh_dir == 0 ? 1u : 100u;
    else
        ++estado->inesperado;
    return 0;
}

static void testa_copia(void)
{
    VERIFICA(grava_binario("origem.bin", 20000) == 0);
    VERIFICA(grava_binario("destino.bin", 24000) == 0);
    VERIFICA(bg_fs_copy_file("origem.bin", "destino.bin") == 0);
    VERIFICA(confere_binario("destino.bin", 20000)); /* truncou o destino */
    VERIFICA(confere_binario("origem.bin", 20000));  /* preservou a origem */
    VERIFICA(grava_binario("vazio.bin", 0) == 0);
    VERIFICA(bg_fs_copy_file("vazio.bin", "destino.bin") == 0);
    VERIFICA(confere_binario("destino.bin", 0));
    VERIFICA(bg_fs_copy_file(NULL, "destino.bin") == -1);
    VERIFICA(bg_fs_copy_file("origem.bin", NULL) == -1);
    VERIFICA(bg_fs_copy_file("inexistente.bin", "destino.bin") == -1);
    VERIFICA(confere_binario("destino.bin", 0)); /* origem ausente não trunca */
    VERIFICA(bg_fs_copy_file("origem.bin", "ausente/destino.bin") == -1);
}

static void testa_temporarios(void)
{
    char primeiro[4096], segundo[4096];
    char sentinela = 'x';
    int r1, r2;

    VERIFICA(bg_fs_make_temp(NULL, 10) == -1);
    VERIFICA(bg_fs_make_temp(&sentinela, 0) == -1);
    VERIFICA(sentinela == 'x');
    r1 = bg_fs_make_temp(primeiro, sizeof primeiro);
    r2 = bg_fs_make_temp(segundo, sizeof segundo);
    VERIFICA(r1 == 0);
    VERIFICA(r2 == 0);
    if (r1 == 0) VERIFICA(confere_binario(primeiro, 0));
    if (r2 == 0) VERIFICA(confere_binario(segundo, 0));
    if (r1 == 0 && r2 == 0) VERIFICA(strcmp(primeiro, segundo) != 0);
    if (r1 == 0) VERIFICA(remove(primeiro) == 0);
    if (r2 == 0) VERIFICA(remove(segundo) == 0);
}

static void testa_caminhada(void)
{
    struct caminhada estado = {0};
    VERIFICA(CRIA_DIR("arvore") == 0);
    VERIFICA(CRIA_DIR("arvore/sub") == 0);
    VERIFICA(grava_binario("arvore/a.bin", 1) == 0);
    VERIFICA(grava_binario("arvore/sub/b.bin", 1) == 0);
    VERIFICA(bg_fs_walk(NULL, visita, &estado) == -1);
    VERIFICA(bg_fs_walk("arvore", NULL, &estado) == -1);
    VERIFICA(bg_fs_walk("nao-existe", visita, &estado) == -1);
    memset(&estado, 0, sizeof estado);
    VERIFICA(bg_fs_walk("arvore", visita, &estado) == 0);
    VERIFICA(estado.total == 4 && estado.raiz == 1 && estado.sub == 1 &&
             estado.arquivo == 1 && estado.profundo == 1 && estado.inesperado == 0);
    memset(&estado, 0, sizeof estado);
    VERIFICA(bg_fs_walk("arvore/", visita, &estado) == 0);
    VERIFICA(estado.total == 4 && estado.raiz == 1 && estado.sub == 1 &&
             estado.arquivo == 1 && estado.profundo == 1 && estado.inesperado == 0);
    memset(&estado, 0, sizeof estado);
    estado.parar = 1;
    VERIFICA(bg_fs_walk("arvore", visita, &estado) == -1);
    VERIFICA(estado.total == 1);
    memset(&estado, 0, sizeof estado);
    estado.parar = 2;
    VERIFICA(bg_fs_walk("arvore", visita, &estado) == -1);
    VERIFICA(estado.total == 2);
}

int main(void)
{
    testa_copia();
    testa_temporarios();
    testa_caminhada();
    if (falhas) {
        fprintf(stderr, "%u falhas em %u verificacoes\n", falhas, verificacoes);
        return 1;
    }
    printf("filesystem: %u verificacoes aprovadas\n", verificacoes);
    return 0;
}
