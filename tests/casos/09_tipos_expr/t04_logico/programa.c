// LOGICO: representação, valores de verdade, OU_EXCLUSIVO e curto-circuito
#include <basicgol.h>

INICIO
    LOGICO v = VERDADEIRO;
    LOGICO f = FALSO;

    ESCREVAL(v);
    ESCREVAL(f);
    ESCREVAL(sizeof(LOGICO) >= 1);

    // conversão para LOGICO: zero é falso, qualquer outra coisa é verdadeira
    ESCREVAL(LOGICO_DE(0));
    ESCREVAL(LOGICO_DE(2));
    ESCREVAL(LOGICO_DE(-1));
    ESCREVAL(LOGICO_DE(0.0));
    ESCREVAL(LOGICO_DE(0.5));
    ESCREVAL(LOGICO_DE(NAN));      // NaN é diferente de zero -> verdadeiro
    ESCREVAL(LOGICO_DE("x"));      // ponteiro não nulo -> verdadeiro

    // OU_EXCLUSIVO normaliza os operandos e avalia cada um uma vez
    ESCREVAL(OU_EXCLUSIVO(v, f));
    ESCREVAL(OU_EXCLUSIVO(v, v));
    ESCREVAL(OU_EXCLUSIVO(f, f));
    ESCREVAL(OU_EXCLUSIVO(1, 2));  // 1 != 1 -> falso

    // E / OU têm curto-circuito: o lado direito não roda quando não precisa
    INTEIRO n = 0;
    SE f E (++n > 0) ENTAO
        ESCREVAL("nao deveria");
    FIM_SE
    SE v OU (++n > 0) ENTAO
        ESCREVAL("curto-circuito ok");
    FIM_SE
    ESCREVAL(n);

    // NAO tem precedência de unário: NAO a E b = (!a) && b
    ESCREVAL(NAO f E v);
    ESCREVAL(NAO (f E v));

    // LOGICO é um inteiro em expressão aritmética (promoção de C)
    ESCREVAL(v + v);
FIM
