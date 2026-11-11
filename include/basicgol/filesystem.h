/*
 * basicgol/filesystem.h — utilitários portáveis de sistema de arquivos.
 * Parte da biblioteca padrão BASICGOL (C11).
 *
 * Este módulo é C puro: NÃO depende da camada de linguagem (`basicgol.h`),
 * portanto continua utilizável de C++ como os demais cabeçalhos de módulo
 * (ver D-15 em docs/04-restricoes-e-decisoes.md). Incluir `basicgol.h` aqui
 * quebrava essa promessa: ele recusa C++ de propósito.
 */
#ifndef BASICGOL_FILESYSTEM_H
#define BASICGOL_FILESYSTEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Copia o conteúdo de `origem` para `destino` em modo binário.
 * Devolve 0 em sucesso e -1 em erro (ponteiro NULL, arquivo ilegível ou
 * falha de escrita); em erro, `destino` pode ficar parcialmente escrito.
 */
int bg_fs_copy_file(const char *origem, const char *destino);

/**
 * Cria um arquivo temporário vazio e escreve o caminho em `buf` (terminado
 * em NUL). `capacidade` é o tamanho de `buf` em bytes; o caminho é recusado
 * com -1 quando não couber — nada é escrito fora do buffer.
 */
int bg_fs_make_temp(char *buf, size_t capacidade);

/**
 * Percorre `raiz` recursivamente, chamando `cb` para a própria raiz e para
 * cada entrada (arquivo ou diretório). `cb` recebe `is_dir` (1 para
 * diretório) e `ctx`; devolver diferente de 0 interrompe a caminhada.
 * Devolve 0 em sucesso e -1 em erro (ponteiro NULL, diretório ilegível ou
 * interrupção pelo callback).
 */
int bg_fs_walk(const char *raiz,
               int (*cb)(const char *caminho, int is_dir, void *ctx),
               void *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BASICGOL_FILESYSTEM_H */
