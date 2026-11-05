// tipos.c — um valor de cada tipo suportado, com a representacao C visivel.
// Perfil: M0 (macros).
// Compilar: cc -std=c11 -Wall -Wextra tipos.c -o tipos -lm
//
// Versao executavel da tabela de tipos de docs/06-tipos-e-expressoes.md:
// cada bloco imprime valor, largura em bits e faixa quando faz sentido.
#include <basicgol.h>

PROCEDIMENTO linha(void) {
    ESCREVAL("---------------------------------------------");
}

// VAZIO e o tipo de retorno de um PROCEDIMENTO: nao devolve valor.
PROCEDIMENTO mostra_logico(void) {
    LOGICO sim = VERDADEIRO;
    LOGICO nao = FALSO;

    ESCREVA("LOGICO        "); ESCREVA(sim); ESCREVA(" "); ESCREVAL(nao);
    ESCREVA("  tamanho: "); ESCREVA(TAMANHO_DE(LOGICO)); ESCREVAL(" byte(s)");
    ESCREVA("  LOGICO_DE(0.5) = "); ESCREVAL(LOGICO_DE(0.5));
    ESCREVA("  OU_EXCLUSIVO(verdadeiro, falso) = ");
    ESCREVAL(OU_EXCLUSIVO(VERDADEIRO, FALSO));
}

PROCEDIMENTO mostra_caractere(void) {
    CARACTERE letra = 'A';
    CARACTERE cifrao = '$';

    ESCREVA("CARACTERE     ");
    ESCREVA(letra); ESCREVA(" "); ESCREVA(cifrao); ESCREVA_LINHA();
    ESCREVA("  tamanho: "); ESCREVA(TAMANHO_DE(CARACTERE)); ESCREVAL(" byte(s)");
    ESCREVA("  'A' + 2 = "); ESCREVAL(letra + 2);   // promoção para INTEIRO
    ESCREVA("  (CARACTERE) ('A' + 2) = "); ESCREVAL((CARACTERE) (letra + 2));
    ESCREVA("  E_DIGITO('7') = "); ESCREVA(E_DIGITO('7'));
    ESCREVA("  E_LETRA('7') = "); ESCREVAL(E_LETRA('7'));
    ESCREVA("  CARACTERE_MAIUSCULO('q') = "); ESCREVAL(CARACTERE_MAIUSCULO('q'));
    ESCREVA("  CARACTERE_MINUSCULO('Q') = "); ESCREVAL(CARACTERE_MINUSCULO('Q'));
}

PROCEDIMENTO mostra_inteiros(void) {
    INTEIRO_CURTO curto = -12;
    INTEIRO       normal = 1234567;
    INTEIRO_LONGO longo = 1234567890L;
    INTEIRO8  i8  = 100;
    INTEIRO16 i16 = 30000;
    INTEIRO32 i32 = 2000000000;
    INTEIRO64 i64 = 9000000000;

    ESCREVA("INTEIRO_CURTO "); ESCREVAL(curto);
    ESCREVA("INTEIRO       "); ESCREVAL(normal);
    ESCREVA("INTEIRO_LONGO "); ESCREVAL(longo);
    ESCREVA("INTEIRO8/16/32/64: ");
    ESCREVA(i8); ESCREVA(" "); ESCREVA(i16); ESCREVA(" ");
    ESCREVA(i32); ESCREVA(" "); ESCREVAL(i64);
    // Largura garantida: só os tipos de largura exata têm valor fixo. Os
    // tipos nativos têm a largura da plataforma (PT-1).
    ESCREVA("  largura exata: ");
    ESCREVA(BITS_DE(INTEIRO8)); ESCREVA("/");
    ESCREVA(BITS_DE(INTEIRO16)); ESCREVA("/");
    ESCREVA(BITS_DE(INTEIRO32)); ESCREVA("/");
    ESCREVA(BITS_DE(INTEIRO64)); ESCREVAL(" bits");
    ESCREVA("  INTEIRO_CURTO <= INTEIRO <= INTEIRO_LONGO ? ");
    ESCREVAL(sizeof(INTEIRO_CURTO) <= sizeof(INTEIRO)
             E sizeof(INTEIRO) <= sizeof(INTEIRO_LONGO));
    ESCREVA("  faixa de INTEIRO: ");
    ESCREVA(MINIMO_INTEIRO); ESCREVA(" .. "); ESCREVAL(MAXIMO_INTEIRO);
}

PROCEDIMENTO mostra_naturais(void) {
    NATURAL       natural = 60000u;
    NATURAL_CURTO ncurto = 65535;
    NATURAL_LONGO nlongo = 4000000000UL;
    NATURAL8  n8  = 250;
    NATURAL16 n16 = 65000;
    NATURAL32 n32 = 4000000000u;
    NATURAL64 n64 = 18000000000000000000ull;

    ESCREVA("NATURAL       "); ESCREVA(natural);
    ESCREVA("  (sem sinal; mesma largura de INTEIRO? ");
    ESCREVAL(BITS_DE(NATURAL) == BITS_DE(INTEIRO)); ESCREVA(")"); ESCREVA_LINHA();
    ESCREVA("NATURAL_CURTO "); ESCREVA(ncurto); ESCREVA_LINHA();
    ESCREVA("NATURAL_LONGO "); ESCREVA(nlongo); ESCREVA_LINHA();
    ESCREVA("NATURAL8/16/32/64: ");
    ESCREVA(n8); ESCREVA(" "); ESCREVA(n16); ESCREVA(" ");
    ESCREVA(n32); ESCREVA(" "); ESCREVAL(n64);
    ESCREVA("  faixa de NATURAL: 0 .. "); ESCREVAL(MAXIMO_NATURAL);

    // Sem sinal, o estouro e definido: modulo 2^largura.
    n8 = n8 + 10;
    ESCREVA("  NATURAL8 250 + 10 = "); ESCREVAL(n8);
    ESCREVAL("  (modulo 256, comportamento definido)");
}

PROCEDIMENTO mostra_reais(void) {
    REAL_CURTO aproximado = 1.5f;
    REAL       normal = 3.14159265358979;
    REAL_LONGO preciso = 2.5L;

    ESCREVA("REAL_CURTO "); ESCREVA(aproximado); ESCREVA_LINHA();
    ESCREVA("REAL       "); ESCREVA_FMT("%.10f", normal); ESCREVA_LINHA();
    ESCREVA("REAL_LONGO "); ESCREVA(preciso); ESCREVA_LINHA();
    // A precisão de REAL_LONGO varia por plataforma: compare, não imprima bits.
    ESCREVA("  REAL_CURTO <= REAL <= REAL_LONGO ? ");
    ESCREVAL(sizeof(REAL_CURTO) <= sizeof(REAL)
             E sizeof(REAL) <= sizeof(REAL_LONGO));
    ESCREVA("  REAL tem ao menos 32 bits? "); ESCREVAL(BITS_DE(REAL) >= 32);
    ESCREVA("  EPSILON_REAL = "); ESCREVA(EPSILON_REAL); ESCREVA_LINHA();
    ESCREVA("  MAXIMO_REAL = "); ESCREVA(MAXIMO_REAL); ESCREVA_LINHA();

    // 0.1 + 0.2 nao e 0.3: a comparacao correta usa tolerancia.
    REAL soma = 0.1 + 0.2;
    ESCREVA("  0.1 + 0.2 == 0.3 ? "); ESCREVA(soma == 0.3);
    ESCREVA("   |diferenca| < EPSILON_REAL ? ");
    ESCREVAL(ABS(soma - 0.3) < EPSILON_REAL);
}

PROCEDIMENTO mostra_texto(void) {
    TEXTO nome = "BASICGOL";
    TEXTO_FIXO(mensagem, 32);

    ESCREVA("TEXTO         \""); ESCREVA_TEXTO(nome); ESCREVA("\"");
    ESCREVA_LINHA();
    ESCREVA("  TAMANHO_TEXTO = "); ESCREVAL(TAMANHO_TEXTO(nome));
    ESCREVA("  TEXTO_VAZIO(nome) = "); ESCREVAL(TEXTO_VAZIO(nome));

    // TEXTO e imutavel; para montar texto use um TEXTO_FIXO.
    LOGICO coube = COPIA_TEXTO(mensagem, TAMANHO_VETOR(mensagem), "tipo");
    coube = ACRESCENTA_TEXTO(mensagem, TAMANHO_VETOR(mensagem), " texto");
    ESCREVA("  copia + acrescimo coube? "); ESCREVAL(coube);
    ESCREVA("  TEXTO_FIXO     \""); ESCREVA_TEXTO(mensagem); ESCREVA("\"  tamanho ");
    ESCREVAL(TAMANHO_TEXTO(mensagem));
    ESCREVA("  COMPARA_TEXTO(\"a\", \"b\") = "); ESCREVAL(COMPARA_TEXTO("a", "b"));
}

INICIO
    linha();
    ESCREVAL("TIPOS SUPORTADOS");
    linha();
    mostra_logico();
    mostra_caractere();
    mostra_inteiros();
    mostra_naturais();
    mostra_reais();
    mostra_texto();
    linha();
FIM
