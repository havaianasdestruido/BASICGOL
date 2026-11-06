/* utilidades.c — gerador pseudoaleatório padrão do C, sem dependência externa. */
#include <basicgol/utilidades.h>

#include <stdint.h>
#include <stdlib.h>

void bg_inicia_sorteio(unsigned semente)
{
    srand(semente);
}

bool bg_sorteia_ate(unsigned limite_exclusivo, unsigned *saida)
{
    uintmax_t universo = (uintmax_t) RAND_MAX + (uintmax_t) 1;
    uintmax_t limite_aceito;
    uintmax_t valor;

    if (saida == NULL || limite_exclusivo == 0
        || (uintmax_t) limite_exclusivo > universo) {
        return false;
    }

    /* Rejeição remove o viés de x % limite quando as amostras de rand()
     * forem uniformes. A qualidade real continua sendo a da implementação C. */
    limite_aceito = universo - (universo % (uintmax_t) limite_exclusivo);
    do {
        valor = (uintmax_t) rand();
    } while (valor >= limite_aceito);

    *saida = (unsigned) (valor % (uintmax_t) limite_exclusivo);
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
