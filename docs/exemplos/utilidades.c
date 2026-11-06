// utilidades.c — catalogo das funcoes e tipos de conveniencia da biblioteca.
// Perfil: M0 (macros).
// Compilar a partir da raiz: make exemplos
//
// Serve como folha de referencia executavel: cada bloco imprime o resultado de
// um grupo de funcoes. Os valores sao fixos, entao a saida e reproduzivel.
#include <basicgol.h>

CONSTANTE REAL    k_pi = 3.14159265358979;
CONSTANTE INTEIRO k_amostras = 5;

// EXTERNO declara um simbolo definido mais adiante (ou em outra unidade).
EXTERNO INTEIRO contador_global;
INTEIRO contador_global = 7;

// Funcao privada deste arquivo (ESTATICO) que nao devolve nada (VAZIO).
ESTATICO FUNCAO VAZIO linha(char preenchimento) {
    INTEIRO i = 0;
    PARA (i, 1, 30) {
        ESCREVA(preenchimento);
    }
    ESCREVA_LINHA();
}

PROCEDIMENTO mostra_tipos(void) {
    CARACTERE letra = 'B';
    LOGICO    ok = VERDADEIRO;
    INTEIRO8  pequeno = 100;
    INTEIRO16 medio = 30000;
    INTEIRO32 exato = 2000000000;
    INTEIRO64 grande = 9000000000;
    INTEIRO_CURTO curto = 12;
    INTEIRO_LONGO longo = 1234567890L;
    NATURAL    natural = 60000u;
    NATURAL8   byte = 250u;
    NATURAL16  meio = 65000u;
    NATURAL32 sem_sinal = 4000000000u;
    NATURAL64 enorme = 18000000000000000000ull;
    REAL_CURTO aproximado = 1.5f;
    REAL_LONGO preciso = 2.5L;

    ESCREVA("CARACTERE: "); ESCREVAL(letra);
    ESCREVA("LOGICO:    "); ESCREVAL(ok);
    ESCREVA("INTEIRO8:  "); ESCREVAL(pequeno);
    ESCREVA("INTEIRO16: "); ESCREVAL(medio);
    ESCREVA("INTEIRO32: "); ESCREVAL(exato);
    ESCREVA("CURTO:     "); ESCREVAL(curto);
    ESCREVA("LONGO:     "); ESCREVAL(longo);
    ESCREVA("INTEIRO64: "); ESCREVAL(grande);
    ESCREVA("NATURAL:   "); ESCREVAL(natural);
    ESCREVA("NATURAL8:  "); ESCREVAL(byte);
    ESCREVA("NATURAL16: "); ESCREVAL(meio);
    ESCREVA("NATURAL32: "); ESCREVAL(sem_sinal);
    ESCREVA("NATURAL64: "); ESCREVAL(enorme);
    ESCREVA("REAL_CURTO:"); ESCREVAL(aproximado);
    ESCREVA("REAL_LONGO:"); ESCREVAL(preciso);
}

PROCEDIMENTO mostra_matematica(void) {
    ESCREVA("ABS(-7)          = "); ESCREVAL(ABS(-7));
    ESCREVA("MINIMO(3, 9)     = "); ESCREVAL(MINIMO(3, 9));
    ESCREVA("MAXIMO(3, 9)     = "); ESCREVAL(MAXIMO(3, 9));
    ESCREVA("POTENCIA(2, 10)  = "); ESCREVAL(POTENCIA(2, 10));
    ESCREVA("RAIZ(144)        = "); ESCREVAL(RAIZ(144.0));
    ESCREVA("ARREDONDA(2.6)   = "); ESCREVAL(ARREDONDA(2.6));
    ESCREVA("PISO(2.9)        = "); ESCREVAL(PISO(2.9));
    ESCREVA("TETO(2.1)        = "); ESCREVAL(TETO(2.1));
    ESCREVA("MOD: 17 MOD 5    = "); ESCREVAL(17 MOD 5);
    ESCREVA("IGUAL(4, 4)      = "); ESCREVAL(IGUAL(4, 4));
    ESCREVA("DIFERENTE(4, 5)  = "); ESCREVAL(DIFERENTE(4, 5));

    // ABS, MINIMO e MAXIMO encaminham para funcoes C escolhidas por _Generic:
    // o argumento e avaliado uma unica vez (P-11), ao contrario de uma macro
    // ingenua do tipo "((x) < 0 ? -(x) : (x))".
}

PROCEDIMENTO mostra_texto(void) {
    TEXTO_FIXO(junto, 40);
    TEXTO_FIXO(copia, 40);
    LOGICO coube = FALSO;

    ESCREVA("TAMANHO_TEXTO(\"BASICGOL\") = ");
    ESCREVAL(TAMANHO_TEXTO("BASICGOL"));

    ESCREVA("TEXTOS_IGUAIS(\"abc\", \"abc\") = ");
    ESCREVAL(TEXTOS_IGUAIS("abc", "abc"));

    ESCREVA("TEXTOS_DIFERENTES(\"abc\", \"abd\") = ");
    ESCREVAL(TEXTOS_DIFERENTES("abc", "abd"));

    coube = COPIA_TEXTO(copia, (INTEIRO) sizeof copia, "BASIC");
    ESCREVA("COPIA_TEXTO coube? "); ESCREVAL(coube);

    coube = JUNTA_TEXTO(junto, (INTEIRO) sizeof junto, copia, "GOL");
    ESCREVA("JUNTA_TEXTO coube? "); ESCREVA(coube); ESCREVA("  ->  "); ESCREVAL(junto);

    // Truncamento tem que ser reportado, nunca silencioso (RB-3).
    TEXTO_FIXO(apertado, 4);
    coube = JUNTA_TEXTO(apertado, (INTEIRO) sizeof apertado, "BASIC", "GOL");
    ESCREVA("JUNTA_TEXTO em buffer pequeno coube? ");
    ESCREVAL(coube);
}

PROCEDIMENTO mostra_enumeracao(void) {
    ENUMERACAO(Conceito)
        CONCEITO_A = 10,
        CONCEITO_B,
        CONCEITO_C
    FIM_ENUMERACAO(Conceito);

    Conceito c = CONCEITO_B;
    // enumeração é um tipo do usuário: o despacho de ESCREVA (_Generic) não
    // cobre tipos definidos pelo usuário — converta para INTEIRO ao imprimir.
    ESCREVA("CONCEITO_B vale ");
    ESCREVAL((INTEIRO) c);

    ESCOLHA (c)
        CASO CONCEITO_A:
            ESCREVAL("conceito A");
            PARE;
        CASO CONCEITO_B:
            ESCREVAL("conceito B");
            PARE;
        CONTRARIO:
            ESCREVAL("outro conceito");
    FIM_ESCOLHA
}

PROCEDIMENTO mostra_conversoes(void) {
    INTEIRO valor = 0;
    REAL    real = 0.0;

    SE TEXTO_PARA_INTEIRO("42", &valor) ENTAO
        ESCREVA("TEXTO_PARA_INTEIRO(\"42\") = ");
        ESCREVAL(valor);
    FIM_SE

    SE NAO TEXTO_PARA_INTEIRO("4x2", &valor) ENTAO
        ESCREVAL("TEXTO_PARA_INTEIRO rejeitou \"4x2\" (correto)");
    FIM_SE

    SE TEXTO_PARA_REAL("2.75", &real) ENTAO
        ESCREVA("TEXTO_PARA_REAL(\"2.75\") = ");
        ESCREVAL(real);
    FIM_SE

    // AFIRME verifica contratos internos; nao e para validar entrada do usuario.
    AFIRME(k_amostras > 0, "k_amostras deve ser positiva");
    ESCREVAL("AFIRME passou: contrato interno satisfeito.");

    // ESCREVA_ERRO escreve em stderr, sem quebrar a linha de saida normal.
    ESCREVA_ERRO("(esta linha vai para stderr)\n");
}

INICIO
    linha('=');
    ESCREVAL("TIPOS");
    linha('-');
    mostra_tipos();

    linha('=');
    ESCREVAL("MATEMATICA");
    linha('-');
    mostra_matematica();

    linha('=');
    ESCREVAL("TEXTO");
    linha('-');
    mostra_texto();

    linha('=');
    ESCREVAL("ENUMERACAO");
    linha('-');
    mostra_enumeracao();

    linha('=');
    ESCREVAL("CONVERSOES E CONTRATOS");
    linha('-');
    mostra_conversoes();

    linha('=');
    ESCREVA("pi = ");
    ESCREVA_FMT("%.5f", k_pi);
    ESCREVA("   contador_global (EXTERNO) = ");
    ESCREVAL(contador_global);
    ESCREVA_LINHA();
FIM
