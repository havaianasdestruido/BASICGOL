// tipos de largura fixa, arredondamentos, comparações e sorteio
#include <basicgol.h>

INICIO
    INTEIRO_CURTO curto = 100;
    NATURAL16 a16 = 65535;
    NATURAL32 a32 = 4000000000;
    NATURAL64 a64 = 1234567890123;

    ESCREVAL(curto);
    ESCREVAL(a16);
    ESCREVAL(a32);
    ESCREVAL(a64);

    // PISO/TETO respeitam o sinal
    ESCREVAL(PISO(3.7));
    ESCREVAL(TETO(3.2));
    ESCREVAL(PISO(-3.2));
    ESCREVAL(TETO(-3.7));

    // DIFERENTE e TEXTOS_DIFERENTES (formas prefixadas)
    ESCREVAL(DIFERENTE(3, 4));
    ESCREVAL(DIFERENTE(3, 3));
    ESCREVAL(TEXTOS_DIFERENTES("a", "b"));
    ESCREVAL(TEXTOS_DIFERENTES("a", "a"));

    // sorteio: só a faixa é previsível, o valor não
    INICIA_SORTEIO(7);
    INTEIRO s = SORTEIA(100);
    SE s >= 0 E s < 100 ENTAO
        ESCREVAL("sorteio na faixa");
    SENAO
        ESCREVAL("sorteio fora da faixa");
    FIM_SE
FIM
