/*
 * basicgol/utilidades.h — utilidades gerais portáveis.
 * O gerador é próprio (xorshift64*, semente derivada por splitmix64) e
 * compartilha o estado do processo: a MESMA semente produz a MESMA sequência
 * em qualquer plataforma C11. Veja a limitação documentada.
 */
#ifndef BASICGOL_UTILIDADES_H
#define BASICGOL_UTILIDADES_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Inicializa explicitamente a sequência pseudoaleatória global (qualquer
 *  semente de 32 bits é válida; resultados idênticos em qualquer plataforma). */
void bg_inicia_sorteio(unsigned semente);

/**
 * Sorteia um valor em [0, limite_exclusivo). Requer limite > 0 (sem teto:
 * qualquer `unsigned` não nulo é aceito; o universo de sorteio é 2^32).
 * False indica argumento inválido; em falha `saida` não muda.
 * A remoção do viés do módulo usa rejeição sobre 32 bits de estado.
 * Não é criptográfico nem independente de thread.
 */
bool bg_sorteia_ate(unsigned limite_exclusivo, unsigned *saida);

/**
 * Compatibilidade com SORTEIA(n): devolve [0,n) para n válido; n <= 0 devolve
 * 0. Prefira bg_sorteia_ate.
 */
int bg_sorteia(int n);

#ifdef __cplusplus
}
#endif

#endif /* BASICGOL_UTILIDADES_H */
