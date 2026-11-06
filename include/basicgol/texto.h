/*
 * basicgol/texto.h — operações seguras sobre strings C terminadas em NUL.
 * Os tamanhos e índices são em bytes; não há interpretação de UTF-8.
 */
#ifndef BASICGOL_TEXTO_H
#define BASICGOL_TEXTO_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Comprimento em bytes; NULL é tratado como texto vazio (0). */
size_t bg_tamanho_texto(const char *texto);

/** Copia texto para destino com capacidade explícita; aceita origem NULL vazia. */
bool bg_copia_texto(char *destino, size_t capacidade, const char *origem);

/** Junta dois textos em destino; aceita origens NULL vazias e sobreposição. */
bool bg_junta_texto(char *destino, size_t capacidade,
                    const char *primeiro, const char *segundo);

/** Acrescenta origem ao texto já existente em destino; aceita autoacréscimo. */
bool bg_acrescenta_texto(char *destino, size_t capacidade, const char *origem);

/**
 * Copia no máximo `quantidade` bytes de `origem` a partir de `inicio`.
 * Quantidade é limitada ao fim do texto; início além do fim produz vazio.
 */
bool bg_trecho_texto(char *destino, size_t capacidade, const char *origem,
                     size_t inicio, size_t quantidade);

/** Compara lexicograficamente; retorna -1, 0 ou 1. NULL equivale a "". */
int bg_compara_texto(const char *primeiro, const char *segundo);
/** Retorna true para "" e NULL. */
bool bg_texto_vazio(const char *texto);

/** Formata como snprintf em buffer limitado; false indica formato/saída inválida ou truncamento. */
bool bg_formata_texto(char *destino, size_t capacidade,
                      const char *formato, ...);

/** Converte texto decimal integral para int; texto não consumido ou estouro falha. */
bool bg_texto_para_inteiro(const char *texto, int *saida);
/** Converte texto decimal não negativo para unsigned; sinal '-' e estouro falham. */
bool bg_texto_para_natural(const char *texto, unsigned *saida);
/** Converte texto para double usando a convenção numérica da locale C atual. */
bool bg_texto_para_real(const char *texto, double *saida);
/** Converte double para int truncando em direção a zero; NaN/estouro falham. */
bool bg_numero_para_inteiro(double valor, int *saida);

#ifdef __cplusplus
}
#endif

#endif /* BASICGOL_TEXTO_H */
