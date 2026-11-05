
// reais: formatacao explicita e comparacao com tolerancia
#include <basicgol.h>

FUNCAO LOGICO perto(REAL a, REAL b, REAL tolerancia) {
    RETORNE ABS(a - b) <= tolerancia;
}

INICIO
    REAL dez = 10.0;
    REAL tres = 3.0;

    ESCREVA_FMT("%.2f\n", dez / tres);
    ESCREVA_FMT("%.5f\n", dez / tres);
    ESCREVA_FMT("%g\n", dez / tres);
    ESCREVA_FMT("%.0f\n", 2.5 + 2.5);

    // 0.1 + 0.2 != 0.3 em binario: a comparacao correta usa tolerancia
    SE perto(0.1 + 0.2, 0.3, 1e-9) ENTAO
        ESCREVAL("soma ok");
    SENAO
        ESCREVAL("soma estranha");
    FIM_SE

    SE 0.1 + 0.2 == 0.3 ENTAO
        ESCREVAL("igualdade exata (nao esperado)");
    SENAO
        ESCREVAL("igualdade exata falha (esperado)");
    FIM_SE
FIM
