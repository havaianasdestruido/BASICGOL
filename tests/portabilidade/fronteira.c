/* fronteira.c — casos de fronteira da biblioteca padrão (Fase 6).
 *
 * Cada verificação é uma asserção (AFIRME) sobre o resultado de uma função da
 * biblioteca: capacidades exatas, um byte a mais, NULL, sobreposição, estouro
 * de conversão, sinais, mínimos assinados, limites do sorteio. Sucesso imprime
 * "fronteira: ok (N verificacoes)" e sai 0; falha sai 1 com arquivo:linha.
 *
 * Não imprime ponto flutuante: comparar texto de `%g` varia entre bibliotecas
 * C (glibc, musl, UCRT), e o teste precisa ser o mesmo em todas.
 *
 * As rotas fatais (divisão por zero, ABS do mínimo, bg_passo_zero) ficam em
 * `tests/test_biblioteca.c`, que as exercita em subprocesso.
 *
 * Compilar: cc -std=c11 -Wall -Wextra -Wformat=2 -Werror -I include \
 *              tests/portabilidade/fronteira.c build/libbasicgol.a -lm
 */
#include <basicgol.h>

static INTEIRO verificacoes = 0;

/* Conta a verificação antes de avaliar, para o número final dizer quantas
 * asserções este arquivo tem (serve de alarme se alguém apagar uma). */
#define EXIGE(cond)                                                       \
    do {                                                                  \
        verificacoes++;                                                   \
        AFIRME((cond), "verificacao de fronteira falhou");                \
    } while (0)

INICIO
    /* ------------------------------------------------ texto: tamanho */
    EXIGE(bg_tamanho_texto(NULL) == 0);
    EXIGE(bg_tamanho_texto("") == 0);
    EXIGE(bg_tamanho_texto("abc") == 3);

    /* ------------------------------------------- texto: cópia exata */
    {
        char dst[4];

        dst[0] = 'x'; dst[1] = 'x'; dst[2] = 'x'; dst[3] = 'x';
        EXIGE(bg_copia_texto(dst, 4, "abc") == true);   /* 3 + NUL = 4: cabe */
        EXIGE(dst[0] == 'a' E dst[1] == 'b' E dst[2] == 'c' E dst[3] == '\0');
        EXIGE(bg_copia_texto(dst, 3, "abc") == false);  /* 1 byte a menos */
        EXIGE(dst[0] == '\0');
        EXIGE(bg_copia_texto(dst, 4, NULL) == true);
        EXIGE(dst[0] == '\0');
        EXIGE(bg_copia_texto(dst, 0, "abc") == false);
        EXIGE(bg_copia_texto(NULL, 4, "abc") == false);
        EXIGE(bg_copia_texto(dst, 4, dst) == true);     /* origem == destino */
    }

    /* ------------------------------------------- texto: junção exata */
    {
        char dst[7];
        char sobreposto[8] = "abcd";

        EXIGE(bg_junta_texto(dst, 7, "abc", "def") == true);  /* 6 + NUL */
        EXIGE(strcmp(dst, "abcdef") == 0);
        EXIGE(bg_junta_texto(dst, 6, "abc", "def") == false);
        EXIGE(dst[0] == '\0');
        EXIGE(bg_junta_texto(dst, 7, NULL, "def") == true);
        EXIGE(strcmp(dst, "def") == 0);
        EXIGE(bg_junta_texto(dst, 7, NULL, NULL) == true);
        EXIGE(strcmp(dst, "") == 0);
        /* destino sobreposto às próprias origens */
        EXIGE(bg_junta_texto(sobreposto, TAMANHO_DE(sobreposto), sobreposto, "xy") == true);
        EXIGE(strcmp(sobreposto, "abcdxy") == 0);
    }

    /* -------------------------------------- texto: acréscimo e trecho */
    {
        char dst[6] = "ab";
        char trecho[4];

        EXIGE(bg_acrescenta_texto(dst, 6, "cde") == true);   /* 5 + NUL */
        EXIGE(strcmp(dst, "abcde") == 0);
        EXIGE(bg_acrescenta_texto(dst, 6, "f") == false);
        EXIGE(dst[0] == '\0');
        EXIGE(bg_acrescenta_texto(dst, 6, NULL) == true);
        EXIGE(bg_acrescenta_texto(dst, 1, "") == true);      /* só cabe o NUL */
        EXIGE(dst[0] == '\0');

        EXIGE(bg_trecho_texto(trecho, 4, "abcdef", 2, 3) == true);
        EXIGE(strcmp(trecho, "cde") == 0);
        /* quantidade é limitada ao fim do texto: 4 bytes não cabem em 4+1? */
        EXIGE(bg_trecho_texto(trecho, 4, "abcdef", 2, 99) == false);
        EXIGE(trecho[0] == '\0');
        {
            char t5[5];

            EXIGE(bg_trecho_texto(t5, 5, "abcdef", 2, 99) == true);
            EXIGE(strcmp(t5, "cdef") == 0);
            EXIGE(bg_trecho_texto(t5, 5, "abc", 0, 99) == true);
            EXIGE(strcmp(t5, "abc") == 0);
        }
        EXIGE(bg_trecho_texto(trecho, 4, "abcdef", 6, 1) == true);   /* além do fim */
        EXIGE(strcmp(trecho, "") == 0);
        EXIGE(bg_trecho_texto(trecho, 3, "abcdef", 0, 3) == false);  /* não cabe */
        EXIGE(bg_trecho_texto(trecho, 4, NULL, 0, 1) == true);
        EXIGE(bg_trecho_texto(trecho, 4, "abcd", 0, 0) == true);
        EXIGE(strcmp(trecho, "") == 0);
    }

    /* ----------------------------------------------- texto: comparação */
    EXIGE(bg_compara_texto("abc", "abc") == 0);
    EXIGE(bg_compara_texto("abc", "abd") == -1);
    EXIGE(bg_compara_texto("abd", "abc") == 1);
    EXIGE(bg_compara_texto("", "a") == -1);
    EXIGE(bg_compara_texto(NULL, NULL) == 0);
    EXIGE(bg_compara_texto(NULL, "") == 0);
    EXIGE(bg_compara_texto(NULL, "a") == -1);
    EXIGE(bg_texto_vazio(NULL) == true);
    EXIGE(bg_texto_vazio("") == true);
    EXIGE(bg_texto_vazio(" ") == false);

    /* -------------------------------------------- texto: formatação */
    {
        char dst[8];

        EXIGE(bg_formata_texto(dst, 4, "%d", 123) == true);      /* 3 + NUL */
        EXIGE(strcmp(dst, "123") == 0);
        EXIGE(bg_formata_texto(dst, 3, "%d", 123) == false);     /* truncaria */
        EXIGE(dst[0] == '\0');
        EXIGE(bg_formata_texto(dst, 8, "%s", "abcdefg") == true);
        EXIGE(strcmp(dst, "abcdefg") == 0);
        EXIGE(bg_formata_texto(dst, 8, "%s", "abcdefgh") == false);
        EXIGE(dst[0] == '\0');
        EXIGE(bg_formata_texto(dst, 8, NULL) == false);
        EXIGE(bg_formata_texto(NULL, 8, "%d", 1) == false);
        EXIGE(bg_formata_texto(dst, 1, "%s", "") == true);       /* só o NUL */
        EXIGE(dst[0] == '\0');
    }

    /* --------------------------------------- conversões: inteiro/natural */
    {
        int i = 999;

        EXIGE(bg_texto_para_inteiro("42", &i) == true E i == 42);
        EXIGE(bg_texto_para_inteiro("  -42  ", &i) == true E i == -42);
        EXIGE(bg_texto_para_inteiro("+42", &i) == true E i == 42);
        EXIGE(bg_texto_para_inteiro("0", &i) == true E i == 0);
        EXIGE(bg_texto_para_inteiro("", &i) == false);
        EXIGE(bg_texto_para_inteiro("   ", &i) == false);
        EXIGE(bg_texto_para_inteiro("42x", &i) == false);
        EXIGE(bg_texto_para_inteiro("0x10", &i) == false);   /* base 10 */
        EXIGE(bg_texto_para_inteiro("1e3", &i) == false);
        i = 7;
        EXIGE(bg_texto_para_inteiro("nao", &i) == false E i == 7);
        EXIGE(bg_texto_para_inteiro(NULL, &i) == false);
        EXIGE(bg_texto_para_inteiro("1", NULL) == false);

        EXIGE(bg_texto_para_inteiro("2147483647", &i) == true E i == 2147483647);
        EXIGE(bg_texto_para_inteiro("-2147483648", &i) == true E i == -2147483647 - 1);
        EXIGE(bg_texto_para_inteiro("2147483648", &i) == false);     /* +1 */
        EXIGE(bg_texto_para_inteiro("-2147483649", &i) == false);    /* -1 */
        EXIGE(bg_texto_para_inteiro("99999999999999999999", &i) == false);
    }
    {
        unsigned u = 999u;

        EXIGE(bg_texto_para_natural("0", &u) == true E u == 0u);
        EXIGE(bg_texto_para_natural("4294967295", &u) == true E u == 4294967295u);
        EXIGE(bg_texto_para_natural("4294967296", &u) == false);     /* +1 */
        EXIGE(bg_texto_para_natural("-1", &u) == false);             /* sinal */
        EXIGE(bg_texto_para_natural(" 7 ", &u) == true E u == 7u);
        EXIGE(bg_texto_para_natural("7.0", &u) == false);
    }

    /* ------------------------------------------------ conversões: real */
    {
        double d = -1.0;

        EXIGE(bg_texto_para_real("3.5", &d) == true E d == 3.5);
        EXIGE(bg_texto_para_real(" 3.5 ", &d) == true E d == 3.5);
        EXIGE(bg_texto_para_real("-0.25", &d) == true E d == -0.25);
        EXIGE(bg_texto_para_real("abc", &d) == false);
        EXIGE(bg_texto_para_real("3,5", &d) == false);      /* locale C (D-17) */
        EXIGE(bg_texto_para_real("1e999", &d) == false);    /* estouro */
        EXIGE(bg_texto_para_real("1e-999", &d) == false);   /* underflow */
        EXIGE(bg_texto_para_real("", &d) == false);
        EXIGE(bg_texto_para_real(NULL, &d) == false);
    }

    /* ------------------------------------ conversões: real -> inteiro */
    {
        int i = 0;

        EXIGE(bg_numero_para_inteiro(3.7, &i) == true E i == 3);
        EXIGE(bg_numero_para_inteiro(-3.7, &i) == true E i == -3);   /* trunca */
        EXIGE(bg_numero_para_inteiro(2147483647.0, &i) == true E i == 2147483647);
        EXIGE(bg_numero_para_inteiro(2147483648.0, &i) == false);    /* +1 */
        EXIGE(bg_numero_para_inteiro(-2147483649.0, &i) == false);
        EXIGE(bg_numero_para_inteiro(1.0 / 0.0 * 0.0, &i) == false); /* NaN */
        EXIGE(bg_numero_para_inteiro(0.0, &i) == true E i == 0);
        EXIGE(bg_numero_para_inteiro(1.0, NULL) == false);
    }

    /* ---------------------------------------------- divisão e resto */
    EXIGE(bg_divide_i(7, 2, "t", 1) == 3);
    EXIGE(bg_divide_i(-7, 2, "t", 1) == -3);
    EXIGE(bg_divide_l(7L, 2L, "t", 1) == 3L);
    EXIGE(bg_divide_ll(-9LL, 2LL, "t", 1) == -4LL);
    EXIGE(bg_divide_u(7u, 2u, "t", 1) == 3u);
    EXIGE(bg_divide_ul(7ul, 2ul, "t", 1) == 3ul);
    EXIGE(bg_divide_ull(7ull, 2ull, "t", 1) == 3ull);
    EXIGE(bg_divide_f(1.0f, 4.0f, "t", 1) == 0.25f);
    EXIGE(bg_divide_d(1.0, 4.0, "t", 1) == 0.25);
    EXIGE(bg_divide_ld(1.0L, 4.0L, "t", 1) == 0.25L);
    EXIGE(bg_resto_i(7, 2, "t", 1) == 1);
    EXIGE(bg_resto_i(-7, 2, "t", 1) == -1);
    EXIGE(bg_resto_l(7L, -2L, "t", 1) == 1L);
    EXIGE(bg_resto_ll(-7LL, -2LL, "t", 1) == -1LL);
    EXIGE(bg_resto_u(7u, 2u, "t", 1) == 1u);
    EXIGE(bg_resto_ul(7ul, 2ul, "t", 1) == 1ul);
    EXIGE(bg_resto_ull(7ull, 2ull, "t", 1) == 1ull);

    /* ------------------------------------- ABS / MINIMO / MAXIMO */
    EXIGE(bg_abs_i(0) == 0);
    EXIGE(bg_abs_i(-5) == 5);
    EXIGE(bg_abs_i(5) == 5);
    EXIGE(bg_abs_i(INT_MAX) == INT_MAX);
    EXIGE(bg_abs_i(INT_MIN + 1) == INT_MAX);
    EXIGE(bg_abs_l(LONG_MIN + 1L) == LONG_MAX);
    EXIGE(bg_abs_ll(LLONG_MIN + 1LL) == LLONG_MAX);
    EXIGE(bg_abs_f(-2.5f) == 2.5f);
    EXIGE(bg_abs_d(-2.5) == 2.5);
    EXIGE(bg_min_i(1, 2) == 1 E bg_min_i(2, 1) == 1 E bg_min_i(3, 3) == 3);
    EXIGE(bg_max_i(1, 2) == 2 E bg_max_i(2, 1) == 2 E bg_max_i(3, 3) == 3);
    EXIGE(bg_min_l(-1L, 1L) == -1L E bg_max_l(-1L, 1L) == 1L);
    EXIGE(bg_min_d(-1.0, 1.0) == -1.0 E bg_max_d(-1.0, 1.0) == 1.0);
    EXIGE(bg_min_d(1.0, 1.0) == 1.0);
    /* macros de valor: tipo vem de _Generic, resultado continua inteiro */
    EXIGE(ABS(-4) == 4 E MINIMO(4, 2) == 2 E MAXIMO(4, 2) == 4);
    EXIGE(TAMANHO_DE(INTEIRO) == sizeof(int));
    EXIGE(BITS_DE(INTEIRO) == (INTEIRO) (sizeof(int) * 8));

    /* ------------------------------------------------- arredondamento */
    EXIGE(bg_round_l(0.4) == 0 E bg_round_l(0.5) == 1 E bg_round_l(-0.5) == -1);
    EXIGE(bg_round_l(2.5) == 3 E bg_round_l(-2.5) == -3);
    EXIGE(bg_round_l(1e15) == 1000000000000000LL);
    EXIGE(ARREDONDA(2.5) == 3 E PISO(2.9) == 2.0 E TETO(2.1) == 3.0);
    EXIGE(RAIZ(9.0) == 3.0 E POTENCIA(2.0, 10.0) == 1024.0);

    /* -------------------------------------------------- caractere ASCII */
    EXIGE(bg_e_digito('0') == true E bg_e_digito('9') == true);
    EXIGE(bg_e_digito('/') == false E bg_e_digito(':') == false);
    EXIGE(bg_e_digito((char) 0x80) == false);
    EXIGE(bg_e_letra('A') == true E bg_e_letra('z') == true);
    EXIGE(bg_e_letra('@') == false E bg_e_letra('[') == false);
    EXIGE(bg_caractere_maiusculo('a') == 'A');
    EXIGE(bg_caractere_maiusculo('Z') == 'Z');
    EXIGE(bg_caractere_maiusculo('0') == '0');
    EXIGE(bg_caractere_maiusculo((char) 0xFF) == (char) 0xFF);
    EXIGE(bg_caractere_minusculo('A') == 'a');
    EXIGE(bg_caractere_minusculo((char) 0xFF) == (char) 0xFF);
    EXIGE(E_DIGITO('5') == true E E_LETRA('q') == true);
    EXIGE(CARACTERE_MAIUSCULO('q') == 'Q' E CARACTERE_MINUSCULO('Q') == 'q');

    /* -------------------------------------------------------- sorteio */
    {
        unsigned valor = 0;
        int i;
        bool dentro = true;

        EXIGE(bg_sorteia_ate(0u, &valor) == false);          /* limite invalido */
        EXIGE(bg_sorteia_ate(1u, &valor) == true E valor == 0u);
        EXIGE(bg_sorteia_ate(10u, NULL) == false);
        EXIGE(((uintmax_t) RAND_MAX + 1u) > 10u);            /* premissa */
        EXIGE(bg_sorteia_ate((unsigned) RAND_MAX + 1u, &valor) == true);

        bg_inicia_sorteio(12345u);
        for (i = 0; i < 200; i++) {
            if (!bg_sorteia_ate(7u, &valor)) {
                dentro = false;
            } else if (valor >= 7u) {
                dentro = false;
            }
        }
        EXIGE(dentro == true);

        EXIGE(bg_sorteia(0) == 0);                           /* invalido */
        EXIGE(bg_sorteia(-3) == 0);
        EXIGE(bg_sorteia(1) == 0);
        EXIGE(bg_sorteia(5) >= 0 E bg_sorteia(5) < 5);
    }

    /* ------------------------------------------------------- console */
    {
        char dst[4];

        EXIGE(bg_escreva_texto(stdout, NULL) == false);
        EXIGE(bg_escreva_texto(NULL, "x") == false);
        EXIGE(bg_escreva_linha(stdout, NULL) == false);
        EXIGE(bg_escreva_texto(stdout, "fronteira: ok (") == true);
        EXIGE(bg_leia_linha(NULL, dst, 4) == false);
        EXIGE(bg_leia_linha(stdin, NULL, 4) == false);
        EXIGE(bg_leia_linha(stdin, dst, 0) == false);
        EXIGE(bg_leia_inteiro(NULL) == false);
        EXIGE(bg_leia_real(NULL) == false);
    }

    ESCREVA_FMT("%d verificacoes)\n", verificacoes);
FIM
