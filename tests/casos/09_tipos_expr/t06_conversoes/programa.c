// Conversões: as de C (declaradas) e as verificadas da biblioteca (Fase 4)
#include <basicgol.h>

INICIO
    // promoções e casts explícitos
    CARACTERE c = 'A';
    ESCREVAL(c + 1);                // 66 (promoção para INTEIRO)
    ESCREVAL((CARACTERE) (c + 1));  // B
    INTEIRO i = 7;
    ESCREVAL(i / 2);                // 3  divisão inteira
    ESCREVAL((REAL) i / 2);         // 3.5 cast antes da divisão
    ESCREVAL((REAL) (i / 2));       // 3  cast depois da divisão
    REAL r = 3.99;
    ESCREVAL((INTEIRO) r);          // 3  truncamento
    ESCREVAL((INTEIRO) -3.99);      // -3
    ESCREVAL((LOGICO) 0);           // 0
    ESCREVAL((LOGICO) 3);           // 1

    // atribuição estreita converte em módulo 2^N para tipo sem sinal (definido)
    INTEIRO largo = 300;
    NATURAL8 um_byte = (NATURAL8) largo;
    ESCREVAL(um_byte);              // 300 - 256 = 44

    // texto -> INTEIRO: aceita o texto inteiro, recusa sobra e estouro
    INTEIRO n = 0;
    ESCREVAL(TEXTO_PARA_INTEIRO(" 42 ", &n));
    ESCREVAL(n);
    ESCREVAL(TEXTO_PARA_INTEIRO("42x", &n));        // 0: sobra
    ESCREVAL(n);                    // 42: falha não altera o destino
    ESCREVAL(TEXTO_PARA_INTEIRO("2147483648", &n)); // 0: > MAXIMO_INTEIRO
    ESCREVAL(n);
    ESCREVAL(TEXTO_PARA_INTEIRO("-2147483648", &n));
    ESCREVAL(n);

    // texto -> NATURAL: "-1" é recusado (viraria ULONG_MAX em strtoul)
    NATURAL u = 0;
    ESCREVAL(TEXTO_PARA_NATURAL("-1", &u));
    ESCREVAL(TEXTO_PARA_NATURAL("42", &u));
    ESCREVAL(u);

    // texto -> REAL: locale C, sem vírgula decimal, sem estouro nem underflow
    REAL d = 0.0;
    ESCREVAL(TEXTO_PARA_REAL("2.5", &d));
    ESCREVAL(TEXTO_PARA_REAL("2,5", &d));
    ESCREVAL(TEXTO_PARA_REAL("1e999", &d));
    ESCREVAL(TEXTO_PARA_REAL("1e-999", &d));
    ESCREVA_FMT("%.1f\n", d);

    // texto -> REAL aceita o que o strtod aceita: "inf" e "nan" são valores
    // representáveis (não são erro de faixa); "%f" imprime o que veio
    ESCREVAL(TEXTO_PARA_REAL("inf", &d));
    ESCREVA_FMT("%.1f\n", d);
    ESCREVAL(TEXTO_PARA_REAL("nan", &d));
    ESCREVAL(d == d);               // 0: NaN não é igual a si mesmo

    // REAL -> INTEIRO verificado: NaN, infinito e fora de faixa devolvem falso
    ESCREVAL(REAL_PARA_INTEIRO(2.9, &n));
    ESCREVAL(n);                    // 2
    ESCREVAL(REAL_PARA_INTEIRO(1e300, &n));
    ESCREVAL(REAL_PARA_INTEIRO(NAN, &n));
    ESCREVAL(REAL_PARA_INTEIRO(INFINITY, &n));
    ESCREVAL(n);                    // 2 (não alterado quando falha)
FIM
