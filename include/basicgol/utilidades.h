/*
 * basicgol/utilidades.h — utilidades gerais portáveis.
 * O gerador rand() é compartilhado pelo processo; veja a limitação documentada.
 */
#ifndef BASICGOL_UTILIDADES_H
#define BASICGOL_UTILIDADES_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Inicializa explicitamente a sequência pseudoaleatória global de rand(). */
void bg_inicia_sorteio(unsigned semente);

/**
 * Sorteia um valor em [0, limite_exclusivo). Requer limite > 0 e não maior que
 * RAND_MAX + 1. False indica argumento inválido; em falha `saida` não muda.
 * A remoção do viés do módulo pressupõe amostras uniformes, algo que a
 * implementação C não garante. Não é criptográfico nem independente de thread.
 */
bool bg_sorteia_ate(unsigned limite_exclusivo, unsigned *saida);

/**
 * Compatibilidade com SORTEIA(n): devolve [0,n) para n válido; n <= 0 (ou um
 * limite fora do alcance de rand()) devolve 0. Prefira bg_sorteia_ate.
 */
int bg_sorteia(int n);

#ifdef __cplusplus
}
#endif

#endif /* BASICGOL_UTILIDADES_H */
