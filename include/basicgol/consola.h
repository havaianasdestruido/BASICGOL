/*
 * basicgol/consola.h — entrada, saída e diagnóstico portáveis.
 * Parte da biblioteca padrão BASICGOL (C11).
 */
#ifndef BASICGOL_CONSOLA_H
#define BASICGOL_CONSOLA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Tamanho máximo, em bytes incluindo NUL, usado por LEIA_INTEIRO/LEIA_REAL. */
#define BG_TAMANHO_LINHA_NUMERO 128

/**
 * Imprime `arquivo:linha: erro: mensagem` em stderr e encerra com status 1.
 * NULL em `arquivo` ou `mensagem` é aceito como "?" ou texto vazio.
 */
_Noreturn void bg_erro(const char *arquivo, int linha, const char *mensagem);

/**
 * Diagnóstico interno do laço PARA_PASSO. Não retorna: chama bg_erro.
 * Mantém retorno int para poder ser usado como expressão de condição em C.
 */
int bg_passo_zero(const char *arquivo, int linha);

/**
 * Lê uma linha de `entrada` em `destino`, sem ultrapassar `capacidade`.
 * Remove LF, CRLF ou CR final. Linha vazia é sucesso; EOF sem bytes, erro de
 * leitura, NUL embutido ou linha longa são falhas. Em linha longa, o restante
 * é consumido para manter o fluxo sincronizado. Em falha, destino utilizável
 * fica vazio. A capacidade inclui o byte NUL.
 *
 * @return true se a linha foi lida inteira; false em argumento inválido, EOF,
 *         erro de E/S, NUL embutido ou truncamento.
 */
bool bg_leia_linha(FILE *entrada, char *destino, size_t capacidade);

/**
 * Lê uma linha de stdin. Compatibilidade com LEIA_TEXTO: descarta um LF ou
 * CRLF pendente deixado por uma leitura formatada anterior. Use
 * bg_leia_linha(stdin, ...) quando uma linha vazia precisar ser preservada.
 */
bool bg_leia_texto(char *destino, size_t capacidade);

/** Lê e escreve uma linha de stdin para int; falha não altera `saida`. */
bool bg_leia_inteiro(int *saida);
/** Lê e escreve uma linha de stdin para double; falha não altera `saida`. */
bool bg_leia_real(double *saida);

/** Lê uma linha de `entrada`, converte-a para int; falha não altera `saida`. */
bool bg_leia_inteiro_de(FILE *entrada, int *saida);
/** Lê uma linha de `entrada`, converte-a para double; falha não altera `saida`. */
bool bg_leia_real_de(FILE *entrada, double *saida);

/** Escreve texto literal (não formato printf) no fluxo; NULL é inválido. */
bool bg_escreva_texto(FILE *saida, const char *texto);
/** Escreve texto literal seguido de LF; NULL é inválido. */
bool bg_escreva_linha(FILE *saida, const char *texto);

#ifdef __cplusplus
}
#endif

#endif /* BASICGOL_CONSOLA_H */
