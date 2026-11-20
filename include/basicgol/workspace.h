/*
 * basicgol/workspace.h — workspace falsa com persistência (OPCIONAL).
 * Parte da biblioteca padrão BASICGOL (C11).
 *
 * Uma "workspace" é um disco simulado, só em memória: pastas e arquivos que
 * existem dentro do processo. Ela serve de cenário para programas que precisam
 * de arquivos sem tocar o sistema de arquivos de verdade e, quando se escolhe
 * um arquivo de persistência, o estado sobrevive ao fim do processo.
 *
 *   • enviar — leva um arquivo do disco real (ou de um buffer) para a workspace;
 *   • baixar — copia um arquivo da workspace de volta para o disco real;
 *   • persistir — grava tudo em um arquivo único (cabeçalho `BGWS1`) que
 *     `bg_ws_abrir` recarrega depois.
 *
 * DESLIGADA POR PADRÃO: este cabeçalho não declara nada até que se defina
 *
 *     #define BG_USAR_WORKSPACE
 *
 * antes do #include. Sem o macro, a API não existe para o programa e o módulo
 * fica inerte: nenhuma workspace, nenhum arquivo aberto, nenhuma alocação, nenhum
 * acesso a disco. Com o macro, a API continua sendo opt-in no código — uma
 * workspace só passa a existir depois de uma chamada a `bg_ws_abrir`.
 *
 * O módulo é C puro: NÃO depende da camada de linguagem (`basicgol.h`), como os
 * demais cabeçalhos de módulo. Não é thread-safe — o estado é um único objeto
 * global do processo, como o PRNG de `basicgol/utilidades.h`.
 *
 * Caminhos da workspace (sempre normalizados, com '/' como separador):
 *   • são relativos; caminho vazio ("") e "/" designam a raiz;
 *   • partes vazias e "." são ignoradas; ".." e partes com ':' são recusadas;
 *   • nomes virtuais têm no máximo 4096 bytes;
 *   • a workspace aceita no máximo 4096 arquivos e 64 MiB de dados no total.
 */
#ifndef BASICGOL_WORKSPACE_H
#define BASICGOL_WORKSPACE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef BG_USAR_WORKSPACE

#define BG_TEM_WORKSPACE 1

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ ciclo de vida */

/**
 * Abre (ou cria) a workspace do processo. `arquivo` é o caminho real do
 * snapshot de persistência: se existir, o conteúdo é carregado; se não
 * existir, a workspace começa vazia e `bg_ws_salvar` passará a criá-lo.
 * `arquivo` NULL (ou "") abre uma workspace só em memória, sem persistência.
 * Devolve false se já há workspace aberta (feche antes), se o snapshot existe
 * mas está corrompido, ou se falta memória. Em falha, nenhuma workspace fica
 * aberta.
 */
bool bg_ws_abrir(const char *arquivo);

/**
 * Grava o estado atual no arquivo de persistência. Devolve false se não há
 * workspace aberta, se ela não tem arquivo de persistência ou se a gravação
 * falha. A gravação é atômica: o snapshot anterior só é substituído se o
 * novo for escrito por inteiro.
 */
bool bg_ws_salvar(void);

/**
 * Grava (quando há arquivo de persistência) e libera a workspace. Devolve
 * false se a gravação falha; o estado é liberado de qualquer forma. Sem
 * workspace aberta, não faz nada e devolve true.
 */
bool bg_ws_fechar(void);

/** True se há uma workspace aberta. */
bool bg_ws_esta_aberta(void);

/**
 * Copia o caminho de persistência para `destino` ("" quando a workspace é só
 * em memória). Devolve false se não há workspace aberta ou se o caminho não
 * couber; em falha, `destino` recebe texto vazio quando a capacidade permite.
 */
bool bg_ws_caminho_persistencia(char *destino, size_t capacidade);

/* ---------------------------------------------------------------- consulta */

/** True se `caminho` existe como arquivo ou como pasta (a raiz sempre existe). */
bool bg_ws_existe(const char *caminho);

/** True se `caminho` é uma pasta (a raiz é pasta; arquivo não é). */
bool bg_ws_eh_pasta(const char *caminho);

/**
 * Tamanho em bytes do arquivo `caminho`. Devolve false (sem alterar `*bytes`)
 * se não há workspace aberta, se o caminho não existe ou se é uma pasta.
 */
bool bg_ws_tamanho(const char *caminho, size_t *bytes);

/**
 * Percorre a workspace a partir de `pasta`: anuncia a própria pasta e, em
 * profundidade, cada pasta e arquivo abaixo dela (pastas antes dos arquivos,
 * na ordem de descoberta). `cb` recebe o caminho virtual normalizado,
 * `eh_pasta` (1 para pasta) e `ctx`; devolver diferente de 0 interrompe a
 * listagem. Devolve false se não há workspace aberta, se `pasta` não é pasta
 * ou se o callback interrompeu; true ao listar tudo.
 */
bool bg_ws_listar(const char *pasta,
                  int (*cb)(const char *caminho, int eh_pasta, void *ctx),
                  void *ctx);

/* ------------------------------------------------------------------- upload */

/**
 * Envia um arquivo do disco real para a workspace: `caminho_real` é lido em
 * binário e guardado com o nome virtual `caminho_ws`. Substitui arquivo já
 * existente. Devolve false se não há workspace aberta, se o disco não pôde
 * ser lido, se o nome virtual é inválido ou conflita com a árvore (um arquivo
 * não pode virar pasta nem pastas virarem arquivo).
 */
bool bg_ws_enviar_disco(const char *caminho_real, const char *caminho_ws);

/**
 * Guarda `bytes` de `dados` com o nome virtual `caminho_ws`. `dados` NULL com
 * `bytes` 0 cria arquivo vazio; NULL com `bytes` > 0 falha. Substitui arquivo
 * já existente. Mesmas regras de nome de `bg_ws_enviar_disco`.
 */
bool bg_ws_enviar_bytes(const char *caminho_ws, const void *dados, size_t bytes);

/** Atalho de `bg_ws_enviar_bytes` para texto: NULL equivale a arquivo vazio. */
bool bg_ws_enviar_texto(const char *caminho_ws, const char *texto);

/* ----------------------------------------------------------------- download */

/**
 * Baixa um arquivo da workspace para o disco real, em binário (substituindo o
 * destino). Devolve false se não há workspace aberta, se `caminho_ws` não
 * existe ou não é arquivo, ou se a gravação falha.
 */
bool bg_ws_baixar_disco(const char *caminho_ws, const char *caminho_real);

/**
 * Baixa um arquivo da workspace para um buffer do chamador: copia no máximo
 * `capacidade` bytes e devolve em `*copiados` quantos foram copiados. Para saber
 * o tamanho total antes, use `bg_ws_tamanho`. `saida` NULL com `capacidade` 0
 * devolve true com 0 copiados; NULL com capacidade maior falha. Em falha,
 * `*copiados` não muda.
 */
bool bg_ws_baixar_bytes(const char *caminho_ws, void *saida, size_t capacidade,
                        size_t *copiados);

/* ----------------------------------------------------------------- remoção */

/**
 * Remove arquivo ou pasta. Pasta só é removida com `recursivo` true (junto com
 * todo o conteúdo); com false, falha. A raiz ("") com `recursivo` true esvazia
 * a workspace; sem `recursivo`, só é removida se já estiver vazia. Devolve
 * false se não há workspace aberta, se o caminho não existe ou se é pasta não
 * recursiva.
 */
bool bg_ws_remover(const char *caminho, bool recursivo);

/** Esvazia a workspace (equivalente a `bg_ws_remover("", true)`). */
bool bg_ws_limpar(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BG_USAR_WORKSPACE */

#endif /* BASICGOL_WORKSPACE_H */
