/*
 * basicgol/filesystem.c — utilitários portáveis de sistema de arquivos.
 *
 * Os caminhos são montados por `junta_caminho`, que confere o tamanho ANTES
 * de escrever. Isso substitui snprintf de propósito: no MinGW-w64 o gcc
 * prova que `"%s\\%s"` com `cFileName` (char[MAX_PATH]) não cabe em outro
 * buffer de MAX_PATH e o aviso vira erro sob -Werror=format-truncation
 * (o mesmo código passava no Linux, onde ninguém era tão específico).
 */
#define _POSIX_C_SOURCE 200809L

#include <basicgol/filesystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#  define BG_SEPARADOR '\\'
#  define BG_TAMANHO_CAMINHO MAX_PATH
#else
#  define BG_SEPARADOR '/'
#  define BG_TAMANHO_CAMINHO 4096
#endif

/* ------------------------------------------------------------- utilidades */

static int bg_e_separador(char c)
{
#ifdef _WIN32
    /* A API do Windows aceita as duas barras em caminhos locais. */
    return c == '\\' || c == '/';
#else
    return c == '/';
#endif
}

/* Copia `origem` em `destino` com NUL; -1 quando não couber. */
static int copia_texto(char *destino, size_t capacidade, const char *origem)
{
    size_t n = strlen(origem);

    if (n + 1u > capacidade) {
        return -1;
    }
    memcpy(destino, origem, n + 1u);
    return 0;
}

/* Junta diretório e nome sem duplicar o separador; -1 quando não couber. */
static int junta_caminho(char *destino, size_t capacidade,
                         const char *diretorio, const char *nome)
{
    size_t tam_dir = strlen(diretorio);
    size_t tam_nome = strlen(nome);
    int precisa_separador =
        (tam_dir > 0u) && !bg_e_separador(diretorio[tam_dir - 1u]) ? 1 : 0;
    size_t total = tam_dir + (size_t)precisa_separador + tam_nome + 1u;
    size_t pos = tam_dir;

    if (total > capacidade) {
        return -1;
    }
    memcpy(destino, diretorio, tam_dir);
    if (precisa_separador) {
        destino[pos] = BG_SEPARADOR;
        pos += 1u;
    }
    memcpy(destino + pos, nome, tam_nome + 1u);
    return 0;
}

/* ---------------------------------------------------------------- público */

int bg_fs_copy_file(const char *origem, const char *destino)
{
    char buf[8192];
    size_t n;
    FILE *entrada;
    FILE *saida;

    if (!origem || !destino) {
        return -1;
    }
    entrada = fopen(origem, "rb");
    if (!entrada) {
        return -1;
    }
    saida = fopen(destino, "wb");
    if (!saida) {
        fclose(entrada);
        return -1;
    }
    while ((n = fread(buf, 1, sizeof buf, entrada)) > 0u) {
        if (fwrite(buf, 1, n, saida) != n) {
            fclose(entrada);
            fclose(saida);
            return -1;
        }
    }
    if (ferror(entrada)) {
        fclose(entrada);
        fclose(saida);
        return -1;
    }
    fclose(entrada);
    if (fclose(saida) != 0) {
        return -1;
    }
    return 0;
}

int bg_fs_make_temp(char *buf, size_t capacidade)
{
    char caminho[BG_TAMANHO_CAMINHO];

    if (!buf || capacidade == 0u) {
        return -1;
    }
#ifdef _WIN32
    {
        char pasta[MAX_PATH];
        DWORD escrito = GetTempPathA((DWORD)sizeof pasta, pasta);

        if (escrito == 0u || (size_t)escrito >= sizeof pasta) {
            return -1;
        }
        /* GetTempFileNameA grava até MAX_PATH bytes: o alvo é local, nunca
         * o buffer do chamador (que pode ser menor que MAX_PATH). */
        if (GetTempFileNameA(pasta, "bg", 0u, caminho) == 0u) {
            return -1;
        }
    }
#else
    {
        static const char modelo[] = "/tmp/bg_XXXXXX";
        int fd;

        memcpy(caminho, modelo, sizeof modelo);
        fd = mkstemp(caminho);
        if (fd == -1) {
            return -1;
        }
        if (close(fd) != 0) {
            return -1;
        }
    }
#endif
    return copia_texto(buf, capacidade, caminho);
}

#ifdef _WIN32
static int walk_dir(const char *raiz, int (*cb)(const char *, int, void *),
                    void *ctx)
{
    char padrao[MAX_PATH];
    WIN32_FIND_DATAA dados;
    HANDLE busca;

    if (junta_caminho(padrao, sizeof padrao, raiz, "*") != 0) {
        return -1;
    }
    busca = FindFirstFileA(padrao, &dados);
    if (busca == INVALID_HANDLE_VALUE) {
        return -1;
    }
    do {
        char caminho[MAX_PATH];
        int eh_dir;

        if (strcmp(dados.cFileName, ".") == 0 ||
            strcmp(dados.cFileName, "..") == 0) {
            continue;
        }
        if (junta_caminho(caminho, sizeof caminho, raiz, dados.cFileName) != 0) {
            FindClose(busca);
            return -1;
        }
        eh_dir = (dados.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0u;
        if (cb(caminho, eh_dir, ctx) != 0) {
            FindClose(busca);
            return -1;
        }
        if (eh_dir && walk_dir(caminho, cb, ctx) != 0) {
            FindClose(busca);
            return -1;
        }
    } while (FindNextFileA(busca, &dados) != 0);
    FindClose(busca);
    return 0;
}
#else
static int walk_dir(const char *raiz, int (*cb)(const char *, int, void *),
                    void *ctx)
{
    DIR *dir;
    struct dirent *entrada;

    dir = opendir(raiz);
    if (!dir) {
        return -1;
    }
    while ((entrada = readdir(dir)) != NULL) {
        char caminho[4096];
        struct stat info;
        int eh_dir;

        if (strcmp(entrada->d_name, ".") == 0 ||
            strcmp(entrada->d_name, "..") == 0) {
            continue;
        }
        if (junta_caminho(caminho, sizeof caminho, raiz, entrada->d_name) != 0) {
            closedir(dir);
            return -1;
        }
        if (stat(caminho, &info) != 0) {
            closedir(dir);
            return -1;
        }
        eh_dir = S_ISDIR(info.st_mode) ? 1 : 0;
        if (cb(caminho, eh_dir, ctx) != 0) {
            closedir(dir);
            return -1;
        }
        if (eh_dir && walk_dir(caminho, cb, ctx) != 0) {
            closedir(dir);
            return -1;
        }
    }
    closedir(dir);
    return 0;
}
#endif

int bg_fs_walk(const char *raiz,
               int (*cb)(const char *caminho, int is_dir, void *ctx),
               void *ctx)
{
    if (!raiz || !cb) {
        return -1;
    }
    /* a própria raiz é anunciada como diretório, antes do conteúdo */
    if (cb(raiz, 1, ctx) != 0) {
        return -1;
    }
    return walk_dir(raiz, cb, ctx);
}
