// expressoes.c — precedencia, conversoes, verdade e operacoes verificadas.
// Perfil: M0 (macros).
// Compilar a partir da raiz: make exemplos
//
// Cada bloco mostra uma regra de docs/06-tipos-e-expressoes.md com o resultado
// concreto. Onde a regra depende do tipo, o tipo aparece na etiqueta.
#include <basicgol.h>

PROCEDIMENTO mostra_precedencia(void) {
    ESCREVAL("Precedencia (a mesma de C; parenteses explicitos):");
    ESCREVA("  2 + 3 * 4        = "); ESCREVAL(2 + 3 * 4);
    ESCREVA("  (2 + 3) * 4      = "); ESCREVAL((2 + 3) * 4);
    ESCREVA("  10 - 2 - 3       = "); ESCREVAL(10 - 2 - 3);
    ESCREVA("  100 / 10 / 2     = "); ESCREVAL(100 / 10 / 2);
    ESCREVA("  7 MOD 2 + 1      = "); ESCREVAL(7 MOD 2 + 1);
    ESCREVA("  2 + 3 < 6        = "); ESCREVAL(2 + 3 < 6);
    ESCREVA("  (1 E 0) OU 1     = "); ESCREVAL((1 E 0) OU 1);
    ESCREVA("  NAO 0 E 0        = "); ESCREVAL(NAO 0 E 0);
    ESCREVA("  -7 / 2           = "); ESCREVAL(-7 / 2);
    ESCREVA("  -7 MOD 2         = "); ESCREVAL(-7 MOD 2);
}

PROCEDIMENTO mostra_atribuicao(void) {
    INTEIRO x = 5;

    ESCREVAL("Atribuicao e comparacao sao operadores diferentes:");
    ESCREVA("  x = 5;  x == 5    = "); ESCREVAL(x == 5);
    x = x + 1;
    ESCREVA("  x = x + 1;  x     = "); ESCREVAL(x);
    x += 2;
    ESCREVA("  x += 2;  x        = "); ESCREVAL(x);

    // IGUAL/DIFERENTE sao prefixados: evitam a duvida de = versus ==.
    ESCREVA("  IGUAL(x, 8)       = "); ESCREVAL(IGUAL(x, 8));
    ESCREVA("  DIFERENTE(x, 8)   = "); ESCREVAL(DIFERENTE(x, 8));
}

PROCEDIMENTO mostra_verdade(void) {
    ESCREVAL("Valor de verdade: zero e falso, o resto e verdadeiro.");
    ESCREVA("  LOGICO_DE(0)      = "); ESCREVAL(LOGICO_DE(0));
    ESCREVA("  LOGICO_DE(-3)     = "); ESCREVAL(LOGICO_DE(-3));
    ESCREVA("  LOGICO_DE(0.0)    = "); ESCREVAL(LOGICO_DE(0.0));
    ESCREVA("  LOGICO_DE(0.1)    = "); ESCREVAL(LOGICO_DE(0.1));
    ESCREVA("  LOGICO_DE(NAN)    = "); ESCREVAL(LOGICO_DE(NAN));
    ESCREVA("  OU_EXCLUSIVO(1,0) = "); ESCREVAL(OU_EXCLUSIVO(1, 0));
    ESCREVA("  OU_EXCLUSIVO(1,1) = "); ESCREVAL(OU_EXCLUSIVO(1, 1));

    // E e OU tem curto-circuito: o lado direito nao roda quando nao precisa.
    INTEIRO avaliacoes = 0;
    LOGICO falso = FALSO;
    LOGICO verdadeiro = VERDADEIRO;
    SE falso E (++avaliacoes > 0) ENTAO
        ESCREVAL("  nao deveria imprimir");
    FIM_SE
    SE verdadeiro OU (++avaliacoes > 0) ENTAO
        ESCREVA("  curto-circuito: lado direito avaliado "); ESCREVA(avaliacoes);
        ESCREVAL(" vez(es)");
    FIM_SE
}

PROCEDIMENTO mostra_conversoes(void) {
    INTEIRO n = 0;
    NATURAL u = 0;
    REAL r = 0.0;
    LOGICO ok = FALSO;

    ESCREVAL("Conversoes:");
    ESCREVA("  (REAL) 7 / 2      = "); ESCREVA_FMT("%.1f\n", (REAL) 7 / 2);
    ESCREVA("  7 / 2             = "); ESCREVAL(7 / 2);
    ESCREVA("  (INTEIRO) 3.9     = "); ESCREVAL((INTEIRO) 3.9);
    ESCREVA("  (INTEIRO) -3.9    = "); ESCREVAL((INTEIRO) -3.9);
    ESCREVA("  (NATURAL8) 300    = "); ESCREVAL((NATURAL8) 300);

    // Texto -> numero: aceita o texto inteiro, recusa sobra e estouro.
    ok = TEXTO_PARA_INTEIRO(" 42 ", &n);
    ESCREVA("  TEXTO_PARA_INTEIRO(\" 42 \")  = "); ESCREVA(ok);
    ESCREVA("  -> "); ESCREVAL(n);
    ok = TEXTO_PARA_INTEIRO("4x2", &n);
    ESCREVA("  TEXTO_PARA_INTEIRO(\"4x2\")   = "); ESCREVA(ok);
    ESCREVA("  (n continua "); ESCREVA(n); ESCREVA(")"); ESCREVA_LINHA();
    ok = TEXTO_PARA_INTEIRO("2147483648", &n);
    ESCREVA("  TEXTO_PARA_INTEIRO(\"2147483648\") = "); ESCREVAL(ok);
    ok = TEXTO_PARA_NATURAL("-1", &u);
    ESCREVA("  TEXTO_PARA_NATURAL(\"-1\")    = "); ESCREVAL(ok);
    ok = TEXTO_PARA_NATURAL("42", &u);
    ESCREVA("  TEXTO_PARA_NATURAL(\"42\")    = "); ESCREVA(ok);
    ESCREVA("  -> "); ESCREVAL(u);
    ok = TEXTO_PARA_REAL("2.5", &r);
    ESCREVA("  TEXTO_PARA_REAL(\"2.5\")      = "); ESCREVA(ok);
    ESCREVA("  -> "); ESCREVA_FMT("%.1f\n", r);
    ok = TEXTO_PARA_REAL("2,5", &r);
    ESCREVA("  TEXTO_PARA_REAL(\"2,5\")      = "); ESCREVAL(ok);
    ok = TEXTO_PARA_REAL("1e999", &r);
    ESCREVA("  TEXTO_PARA_REAL(\"1e999\")    = "); ESCREVAL(ok);

    // Real -> inteiro verificado: NaN e fora de faixa devolvem FALSO.
    ok = REAL_PARA_INTEIRO(2.9, &n);
    ESCREVA("  REAL_PARA_INTEIRO(2.9)    = "); ESCREVA(ok);
    ESCREVA("  -> "); ESCREVAL(n);
    ok = REAL_PARA_INTEIRO(NAN, &n);
    ESCREVA("  REAL_PARA_INTEIRO(NAN)    = "); ESCREVAL(ok);
    ok = REAL_PARA_INTEIRO(1e300, &n);
    ESCREVA("  REAL_PARA_INTEIRO(1e300)  = "); ESCREVAL(ok);
}

PROCEDIMENTO mostra_divisao(void) {
    ESCREVAL("Divisao verificada (DIVIDE/RESTO recusam divisor zero):");
    ESCREVA("  DIVIDE(7, 2)      = "); ESCREVAL(DIVIDE(7, 2));
    ESCREVA("  DIVIDE(-7, 2)     = "); ESCREVAL(DIVIDE(-7, 2));
    ESCREVA("  RESTO(7, 2)       = "); ESCREVAL(RESTO(7, 2));
    ESCREVA("  RESTO(-7, 2)      = "); ESCREVAL(RESTO(-7, 2));
    ESCREVA("  DIVIDE(7.0, 2.0)  = "); ESCREVA_FMT("%.2f\n", DIVIDE(7.0, 2.0));
    ESCREVA("  DIVIDE(MINIMO_INTEIRO, 1) == MINIMO_INTEIRO ? ");
    ESCREVAL(DIVIDE(MINIMO_INTEIRO, 1) == MINIMO_INTEIRO);
    // DIVIDE(1, 0) encerraria o programa com "erro: divisao por zero".
}

PROCEDIMENTO mostra_texto(void) {
    TEXTO_FIXO(buffer, 12);
    LOGICO coube = FALSO;

    ESCREVAL("Texto (todas as operacoes reportam truncamento):");
    coube = COPIA_TEXTO(buffer, TAMANHO_VETOR(buffer), "abc");
    ESCREVA("  COPIA_TEXTO(\"abc\")        = "); ESCREVA(coube);
    ESCREVA("  -> \""); ESCREVA_TEXTO(buffer); ESCREVA("\""); ESCREVA_LINHA();

    coube = ACRESCENTA_TEXTO(buffer, TAMANHO_VETOR(buffer), "def");
    ESCREVA("  ACRESCENTA_TEXTO(\"def\")   = "); ESCREVA(coube);
    ESCREVA("  -> \""); ESCREVA_TEXTO(buffer); ESCREVA("\""); ESCREVA_LINHA();

    coube = ACRESCENTA_TEXTO(buffer, TAMANHO_VETOR(buffer), "0123456789");
    ESCREVA("  acrescentar demais         = "); ESCREVA(coube);
    ESCREVA("  -> tamanho "); ESCREVAL(TAMANHO_TEXTO(buffer));

    coube = TRECHO_TEXTO(buffer, TAMANHO_VETOR(buffer), "BASICGOL", 5, 3);
    ESCREVA("  TRECHO_TEXTO(\"BASICGOL\", 5, 3) = "); ESCREVA(coube);
    ESCREVA("  -> \""); ESCREVA_TEXTO(buffer); ESCREVA("\""); ESCREVA_LINHA();

    coube = FORMATA_TEXTO(buffer, TAMANHO_VETOR(buffer), "%d/%d", 22, 7);
    ESCREVA("  FORMATA_TEXTO(\"%d/%d\")     = "); ESCREVA(coube);
    ESCREVA("  -> \""); ESCREVA_TEXTO(buffer); ESCREVA("\""); ESCREVA_LINHA();

    ESCREVA("  COMPARA_TEXTO(\"a\", \"b\")   = "); ESCREVAL(COMPARA_TEXTO("a", "b"));
    ESCREVA("  COMPARA_TEXTO(\"b\", \"b\")   = "); ESCREVAL(COMPARA_TEXTO("b", "b"));
    ESCREVA("  TEXTOS_IGUAIS(\"a\", \"a\")   = "); ESCREVAL(TEXTOS_IGUAIS("a", "a"));
}

INICIO
    ESCREVAL("=============================================");
    ESCREVAL("EXPRESSOES: SEMANTICA");
    ESCREVAL("=============================================");
    mostra_precedencia();
    ESCREVA_LINHA();
    mostra_atribuicao();
    ESCREVA_LINHA();
    mostra_verdade();
    ESCREVA_LINHA();
    mostra_conversoes();
    ESCREVA_LINHA();
    mostra_divisao();
    ESCREVA_LINHA();
    mostra_texto();
    ESCREVAL("=============================================");
FIM
