/*
 * biblioteca_workspace.c — workspace falsa: enviar, baixar e persistir.
 *
 * A workspace é um disco simulado em memória. Aqui ela recebe três arquivos
 * (dois deles enviados de um buffer, um do disco real), é gravada num snapshot
 * e recarregada depois — o arquivo baixado no fim vem do snapshot.
 *
 * O módulo é opt-in: defina BG_USAR_WORKSPACE antes de incluir o cabeçalho.
 * Compilar a partir da raiz: make exemplos
 *
 * Este exemplo não entra no seletor do playground: ele grava um snapshot em
 * disco, e lá não há disco (o playground só tem a workspace em memória). Ele é
 * verificado pela suíte do repositório (make test / make exemplos).
 */
#define BG_USAR_WORKSPACE
#include <basicgol.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char base[320];
static char snap[360];
static char saida[360];

static int mostra(const char *caminho, int eh_pasta, void *ctx)
{
    size_t n = 0;

    (void) ctx;
    if (caminho[0] == '\0') {
        return 0; /* a raiz não se anuncia */
    }
    if (eh_pasta) {
        (void) printf("  %s/ (pasta)\n", caminho);
        return 0;
    }
    if (!bg_ws_tamanho(caminho, &n)) {
        return 1;
    }
    (void) printf("  %s (%lu bytes)\n", caminho, (unsigned long) n);
    return 0;
}

int main(void)
{
    static const unsigned char binario[] = { 0u, 255u, 1u, 2u, 3u };
    char buf[64];
    FILE *f;
    size_t n = 0;

    if (bg_fs_make_temp(base, sizeof base) != 0
        || snprintf(snap, sizeof snap, "%s.bgws", base) < 0
        || snprintf(saida, sizeof saida, "%s.saida", base) < 0) {
        return 1;
    }
    remove(base);

    if (!bg_ws_abrir(snap)) {
        return 1;
    }

    if (!bg_ws_enviar_texto("texto.txt", "conteudo")
        || !bg_ws_enviar_bytes("pasta/bin.bin", binario, sizeof binario)) {
        return 1;
    }
    f = fopen(saida, "wb");
    if (f == NULL) {
        return 1;
    }
    (void) fputs("do disco", f);
    (void) fclose(f);
    if (!bg_ws_enviar_disco(saida, "disco.txt")) {
        return 1;
    }

    (void) printf("conteudo:\n");
    if (!bg_ws_listar("", mostra, NULL)) {
        return 1;
    }

    if (!bg_ws_salvar() || !bg_ws_fechar()) {
        return 1;
    }

    /* recarrega do snapshot e baixa um arquivo de volta para o disco real */
    if (!bg_ws_abrir(snap) || !bg_ws_baixar_disco("texto.txt", saida)) {
        return 1;
    }
    f = fopen(saida, "rb");
    if (f == NULL) {
        return 1;
    }
    n = fread(buf, 1, sizeof buf - 1u, f);
    buf[n] = '\0';
    (void) fclose(f);
    (void) printf("baixado: %s\n", buf);
    if (!bg_ws_tamanho("pasta/bin.bin", &n)) {
        return 1;
    }
    (void) printf("recarregado: pasta/bin.bin (%lu bytes)\n",
                  (unsigned long) n);
    if (!bg_ws_fechar()) {
        return 1;
    }

    remove(snap);
    remove(saida);
    return 0;
}
