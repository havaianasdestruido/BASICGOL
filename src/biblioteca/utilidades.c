/*
 * utilidades.c — gerador pseudoaleatório portável próprio: a MESMA semente
 * produz a MESMA sequência em qualquer plataforma C11 (o rand() da libc varia
 * entre implementações, o que quebraria exemplos e testes dourados).
 */
#include <basicgol/utilidades.h>

#include <stdint.h>
#include <stddef.h>

/* Estado global do processo (compartilhado; ver a limitação no cabeçalho). */
static uint64_t bg_prng_estado = 0x9E3779B97F4A7C15ULL;

/* xorshift64* (Marsaglia; multiplicador de Vigna). Estado nunca é zero. */
static uint64_t bg_prng_proximo(void)
{
    uint64_t x = bg_prng_estado;

    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    bg_prng_estado = x;
    return x * 0x2545F4914F6CDD1DULL;
}

void bg_inicia_sorteio(unsigned semente)
{
    /* splitmix64 (Steele/Lea/Vigna) deriva um estado bem distribuído e
     * nonzero a partir de qualquer semente de 32 bits. */
    uint64_t z = (uint64_t) semente;

    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    z = z ^ (z >> 31);
    bg_prng_estado = (z != 0) ? z : 0x9E3779B97F4A7C15ULL;
}

bool bg_sorteia_ate(unsigned limite_exclusivo, unsigned *saida)
{
    uint64_t universo = ((uint64_t) UINT32_MAX) + 1U; /* 2^32 */
    uint64_t limite;
    uint64_t limite_aceito;
    uint32_t valor;

    if (saida == NULL || limite_exclusivo == 0U) {
        return false;
    }

    limite = (uint64_t) limite_exclusivo;

    /* Rejeição remove o viés de x % limite com amostras de 32 bits.
     * A qualidade do restante vem do xorshift64*. */
    limite_aceito = universo - (universo % limite);
    do {
        valor = (uint32_t) bg_prng_proximo();
    } while ((uint64_t) valor >= limite_aceito);

    *saida = (unsigned) ((uint64_t) valor % limite);
    return true;
}

int bg_sorteia(int n)
{
    unsigned resultado;

    if (n <= 0 || !bg_sorteia_ate((unsigned) n, &resultado)) {
        return 0;
    }
    return (int) resultado;
}
