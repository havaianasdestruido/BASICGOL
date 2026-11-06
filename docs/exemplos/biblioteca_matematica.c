/*
 * biblioteca_matematica.c — macros BASICGOL sobre <math.h> e operações verificadas.
 * Compilar a partir da raiz: make exemplos
 */
#include <basicgol.h>

int main(void)
{
    printf("ABS(-7) = %d\n", ABS(-7));
    printf("MINIMO/MAXIMO = %d/%d\n", MINIMO(3, 9), MAXIMO(3, 9));
    printf("POTENCIA/RAIZ = %.0f/%.0f\n", POTENCIA(2.0, 10.0), RAIZ(144.0));
    printf("DIVIDE/RESTO = %d/%d\n", DIVIDE(-7, 2), RESTO(-7, 3));
    printf("PISO/TETO/ARREDONDA = %.0f/%.0f/%lld\n",
           PISO(2.9), TETO(2.1), ARREDONDA(-2.5));
    return 0;
}
