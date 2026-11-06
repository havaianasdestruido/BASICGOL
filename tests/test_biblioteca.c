/* Testes unitários da biblioteca padrão BASICGOL. */
#include <basicgol.h>

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned verificacoes;
static unsigned falhas;

#define VERIFICA(condicao) do {                                            \
    ++verificacoes;                                                        \
    if (!(condicao)) {                                                     \
        ++falhas;                                                          \
        (void) fprintf(stderr, "FALHA %s:%d: %s\n", __FILE__, __LINE__,  \
                       #condicao);                                         \
    }                                                                      \
} while (0)

static FILE *cria_arquivo_temporario(const char *conteudo)
{
    FILE *arquivo = tmpfile();

    if (arquivo == NULL) {
        return NULL;
    }
    if (fputs(conteudo, arquivo) < 0) {
        (void) fclose(arquivo);
        return NULL;
    }
    rewind(arquivo);
    return arquivo;
}

static void testa_console(void)
{
    char buffer[8] = "sobra";
    char lido[64];
    char uma_posicao[1] = "x";
    FILE *entrada = cria_arquivo_temporario("abc\n1234567\n12345678\n\ncrlf\r\nfinal");
    FILE *saida = tmpfile();
    FILE *limite = cria_arquivo_temporario("x\n\n");

    VERIFICA(entrada != NULL);
    VERIFICA(saida != NULL);
    VERIFICA(limite != NULL);
    if (entrada == NULL || saida == NULL || limite == NULL) {
        if (entrada != NULL) (void) fclose(entrada);
        if (saida != NULL) (void) fclose(saida);
        if (limite != NULL) (void) fclose(limite);
        return;
    }

    VERIFICA(bg_leia_linha(entrada, buffer, sizeof buffer));
    VERIFICA(strcmp(buffer, "abc") == 0);
    VERIFICA(bg_leia_linha(entrada, buffer, sizeof buffer));
    VERIFICA(strcmp(buffer, "1234567") == 0);
    VERIFICA(bg_leia_linha(entrada, buffer, 5) == false);
    VERIFICA(buffer[0] == '\0');
    VERIFICA(bg_leia_linha(entrada, buffer, sizeof buffer));
    VERIFICA(buffer[0] == '\0'); /* linha vazia é sucesso */
    VERIFICA(bg_leia_linha(entrada, buffer, sizeof buffer));
    VERIFICA(strcmp(buffer, "crlf") == 0);
    VERIFICA(bg_leia_linha(entrada, buffer, sizeof buffer));
    VERIFICA(strcmp(buffer, "final") == 0); /* última linha sem LF */
    VERIFICA(!bg_leia_linha(entrada, buffer, sizeof buffer));
    VERIFICA(buffer[0] == '\0');
    VERIFICA(!bg_leia_linha(NULL, buffer, sizeof buffer));
    VERIFICA(buffer[0] == '\0');
    (void) fclose(entrada);

    VERIFICA(!bg_leia_linha(limite, uma_posicao, 0));
    VERIFICA(uma_posicao[0] == 'x'); /* capacidade zero não autoriza escrita */
    VERIFICA(!bg_leia_linha(limite, uma_posicao, sizeof uma_posicao));
    VERIFICA(uma_posicao[0] == '\0'); /* nenhum byte cabe além do NUL */
    VERIFICA(bg_leia_linha(limite, uma_posicao, sizeof uma_posicao));
    VERIFICA(uma_posicao[0] == '\0'); /* linha vazia cabe até em capacidade 1 */
    VERIFICA(!bg_leia_linha(limite, NULL, 1));
    (void) fclose(limite);

    entrada = tmpfile();
    VERIFICA(entrada != NULL);
    if (entrada != NULL) {
        (void) fputc('a', entrada);
        (void) fputc('\0', entrada);
        (void) fputc('b', entrada);
        (void) fputc('\n', entrada);
        (void) fputs("ok\n", entrada);
        rewind(entrada);
        VERIFICA(!bg_leia_linha(entrada, buffer, sizeof buffer));
        VERIFICA(buffer[0] == '\0');
        VERIFICA(bg_leia_linha(entrada, buffer, sizeof buffer));
        VERIFICA(strcmp(buffer, "ok") == 0);
        (void) fclose(entrada);
    }

    VERIFICA(bg_escreva_texto(saida, "literal %s%n"));
    VERIFICA(bg_escreva_linha(saida, " linha"));
    VERIFICA(!bg_escreva_texto(saida, NULL));
    VERIFICA(!bg_escreva_linha(saida, NULL));
    VERIFICA(!bg_escreva_texto(NULL, "nao escreve"));
    rewind(saida);
    VERIFICA(fgets(lido, sizeof lido, saida) != NULL);
    VERIFICA(strcmp(lido, "literal %s%n linha\n") == 0);
    (void) fclose(saida);
}

static void testa_entrada_numerica(void)
{
    int inteiro = 71;
    double real = 8.25;
    FILE *entrada = cria_arquivo_temporario("-12\n3.125\n4x\n1e9999\n");

    VERIFICA(entrada != NULL);
    if (entrada == NULL) {
        return;
    }
    VERIFICA(bg_leia_inteiro_de(entrada, &inteiro));
    VERIFICA(inteiro == -12);
    VERIFICA(bg_leia_real_de(entrada, &real));
    VERIFICA(real == 3.125);
    inteiro = 71;
    VERIFICA(!bg_leia_inteiro_de(entrada, &inteiro));
    VERIFICA(inteiro == 71); /* saída não muda em falha */
    real = 8.25;
    VERIFICA(!bg_leia_real_de(entrada, &real));
    VERIFICA(real == 8.25);
    VERIFICA(!bg_leia_inteiro_de(entrada, &inteiro)); /* EOF */
    VERIFICA(!bg_leia_inteiro_de(NULL, &inteiro));
    VERIFICA(!bg_leia_inteiro_de(entrada, NULL));
    (void) fclose(entrada);
}

static void testa_texto(void)
{
    char buffer[32];
    char pequeno[4] = "xyz";
    char justo[5];
    char formato[32];
    int inteiro = 17;
    int inteiro_antes;
    unsigned natural = 9;
    unsigned natural_antes;
    double real = 2.0;
    double real_antes;
    char numero_grande[1024];
    size_t i;

    VERIFICA(bg_tamanho_texto(NULL) == 0);
    VERIFICA(bg_tamanho_texto("abc") == 3);
    VERIFICA(bg_copia_texto(buffer, sizeof buffer, "abc"));
    VERIFICA(strcmp(buffer, "abc") == 0);
    VERIFICA(bg_copia_texto(buffer, sizeof buffer, NULL));
    VERIFICA(buffer[0] == '\0');
    VERIFICA(bg_copia_texto(pequeno, sizeof pequeno, "abc")); /* capacidade exata */
    VERIFICA(strcmp(pequeno, "abc") == 0);
    VERIFICA(!bg_copia_texto(pequeno, sizeof pequeno, "quatro"));
    VERIFICA(pequeno[0] == '\0');
    VERIFICA(!bg_copia_texto(NULL, 1, "x"));
    VERIFICA(!bg_copia_texto(buffer, 0, "x"));

    VERIFICA(bg_junta_texto(buffer, sizeof buffer, "ab", "cd"));
    VERIFICA(strcmp(buffer, "abcd") == 0);
    VERIFICA(bg_junta_texto(justo, sizeof justo, "ab", "cd"));
    VERIFICA(strcmp(justo, "abcd") == 0); /* 4 bytes + NUL cabem exatamente */
    VERIFICA(bg_junta_texto(buffer, sizeof buffer, NULL, "x"));
    VERIFICA(strcmp(buffer, "x") == 0);
    VERIFICA(bg_copia_texto(buffer, sizeof buffer, "abcd"));
    VERIFICA(bg_junta_texto(buffer, sizeof buffer, buffer, buffer + 1));
    VERIFICA(strcmp(buffer, "abcdbcd") == 0); /* origens sobrepostas */
    VERIFICA(!bg_junta_texto(pequeno, sizeof pequeno, "ab", "cd"));
    VERIFICA(pequeno[0] == '\0');
    VERIFICA(!bg_junta_texto(NULL, 0, "a", "b"));

    VERIFICA(bg_copia_texto(buffer, sizeof buffer, "ab"));
    VERIFICA(bg_acrescenta_texto(buffer, sizeof buffer, "cd"));
    VERIFICA(strcmp(buffer, "abcd") == 0);
    VERIFICA(bg_copia_texto(buffer, sizeof buffer, "ab"));
    VERIFICA(bg_acrescenta_texto(buffer, 5, "cd"));
    VERIFICA(strcmp(buffer, "abcd") == 0); /* capacidade exata */
    VERIFICA(bg_acrescenta_texto(buffer, sizeof buffer, buffer));
    VERIFICA(strcmp(buffer, "abcdabcd") == 0);
    VERIFICA(!bg_acrescenta_texto(buffer, 8, "x"));
    VERIFICA(buffer[0] == '\0');
    VERIFICA(!bg_acrescenta_texto(NULL, 0, "x"));

    VERIFICA(bg_trecho_texto(buffer, sizeof buffer, "abcdef", 2, 3));
    VERIFICA(strcmp(buffer, "cde") == 0);
    VERIFICA(bg_trecho_texto(buffer, sizeof buffer, "abcdef", 4, SIZE_MAX));
    VERIFICA(strcmp(buffer, "ef") == 0);
    VERIFICA(bg_trecho_texto(buffer, sizeof buffer, "abc", SIZE_MAX, 1));
    VERIFICA(buffer[0] == '\0');
    VERIFICA(bg_trecho_texto(buffer, sizeof buffer, "abc", 1, 0));
    VERIFICA(buffer[0] == '\0');
    VERIFICA(bg_trecho_texto(justo, sizeof justo, "abc", 0, 3));
    VERIFICA(strcmp(justo, "abc") == 0); /* capacidade exata */
    VERIFICA(!bg_trecho_texto(pequeno, sizeof pequeno, "abcdef", 0, 4));
    VERIFICA(pequeno[0] == '\0');
    VERIFICA(bg_copia_texto(buffer, sizeof buffer, "abcdef"));
    VERIFICA(bg_trecho_texto(buffer, sizeof buffer, buffer, 2, 3));
    VERIFICA(strcmp(buffer, "cde") == 0);

    VERIFICA(bg_compara_texto("a", "b") == -1);
    VERIFICA(bg_compara_texto("b", "a") == 1);
    VERIFICA(bg_compara_texto("a", "a") == 0);
    VERIFICA(bg_compara_texto(NULL, "") == 0);
    VERIFICA(bg_texto_vazio(NULL));
    VERIFICA(bg_texto_vazio(""));
    VERIFICA(!bg_texto_vazio(" "));

    VERIFICA(bg_formata_texto(formato, sizeof formato, "%s-%d", "id", 42));
    VERIFICA(strcmp(formato, "id-42") == 0);
    VERIFICA(bg_formata_texto(pequeno, sizeof pequeno, "%d", 123));
    VERIFICA(strcmp(pequeno, "123") == 0); /* capacidade exata */
    VERIFICA(!bg_formata_texto(pequeno, sizeof pequeno, "%s", "longo"));
    VERIFICA(pequeno[0] == '\0');
    (void) strcpy(pequeno, "x");
    VERIFICA(!bg_formata_texto(pequeno, sizeof pequeno, NULL));
    VERIFICA(pequeno[0] == '\0');
    VERIFICA(!bg_formata_texto(NULL, sizeof formato, "%d", 1));
    VERIFICA(!bg_formata_texto(formato, 0, "%d", 1));

    errno = EDOM;
    (void) snprintf(formato, sizeof formato, " \t%d \n", INT_MIN);
    VERIFICA(bg_texto_para_inteiro(formato, &inteiro));
    VERIFICA(errno == EDOM);
    VERIFICA(inteiro == INT_MIN);
    (void) snprintf(formato, sizeof formato, "%d", INT_MAX);
    VERIFICA(bg_texto_para_inteiro(formato, &inteiro));
    VERIFICA(inteiro == INT_MAX);
    inteiro_antes = inteiro;
    VERIFICA(!bg_texto_para_inteiro("12abc", &inteiro));
    VERIFICA(inteiro == inteiro_antes);
    VERIFICA(!bg_texto_para_inteiro("", &inteiro));
    VERIFICA(!bg_texto_para_inteiro(NULL, &inteiro));
    VERIFICA(!bg_texto_para_inteiro("1", NULL));

    VERIFICA(bg_texto_para_natural("  +123 \t", &natural));
    VERIFICA(natural == 123U);
    (void) snprintf(formato, sizeof formato, "%u", UINT_MAX);
    VERIFICA(bg_texto_para_natural(formato, &natural));
    VERIFICA(natural == UINT_MAX);
    natural_antes = natural;
    VERIFICA(!bg_texto_para_natural("-1", &natural));
    VERIFICA(natural == natural_antes);
    VERIFICA(!bg_texto_para_natural("7x", &natural));
    VERIFICA(!bg_texto_para_natural(NULL, &natural));
    VERIFICA(!bg_texto_para_natural("1", NULL));

    for (i = 0; i + 1 < sizeof numero_grande; ++i) {
        numero_grande[i] = '9';
    }
    numero_grande[sizeof numero_grande - 1] = '\0';
    VERIFICA(!bg_texto_para_inteiro(numero_grande, &inteiro));
    VERIFICA(!bg_texto_para_natural(numero_grande, &natural));

    errno = EDOM;
    VERIFICA(bg_texto_para_real(" 3.5 \t", &real));
    VERIFICA(errno == EDOM);
    VERIFICA(real == 3.5);
    real_antes = real;
    VERIFICA(!bg_texto_para_real("2,5", &real));
    VERIFICA(real == real_antes);
    VERIFICA(!bg_texto_para_real("1e9999", &real));
    VERIFICA(!bg_texto_para_real("1e-9999", &real));
    VERIFICA(!bg_texto_para_real("3.5x", &real));
    VERIFICA(!bg_texto_para_real(NULL, &real));
    VERIFICA(!bg_texto_para_real("1", NULL));

    VERIFICA(bg_numero_para_inteiro(12.9, &inteiro));
    VERIFICA(inteiro == 12);
    VERIFICA(bg_numero_para_inteiro(-12.9, &inteiro));
    VERIFICA(inteiro == -12);
    inteiro_antes = inteiro;
    VERIFICA(!bg_numero_para_inteiro(NAN, &inteiro));
    VERIFICA(inteiro == inteiro_antes);
    VERIFICA(!bg_numero_para_inteiro(HUGE_VAL, &inteiro));
    VERIFICA(!bg_numero_para_inteiro(-HUGE_VAL, &inteiro));
    VERIFICA(!bg_numero_para_inteiro(1.0, NULL));
}

static void testa_caracteres(void)
{
    VERIFICA(bg_e_digito('0'));
    VERIFICA(bg_e_digito('9'));
    VERIFICA(!bg_e_digito('/'));
    VERIFICA(!bg_e_digito('a'));
    VERIFICA(bg_e_letra('A'));
    VERIFICA(bg_e_letra('z'));
    VERIFICA(!bg_e_letra('0'));
    VERIFICA(!bg_e_letra((char) 127));
    VERIFICA(bg_caractere_maiusculo('q') == 'Q');
    VERIFICA(bg_caractere_maiusculo('Q') == 'Q');
    VERIFICA(bg_caractere_maiusculo('7') == '7');
    VERIFICA(bg_caractere_minusculo('Q') == 'q');
    VERIFICA(bg_caractere_minusculo('q') == 'q');
    VERIFICA(bg_caractere_minusculo('7') == '7');
    VERIFICA(E_DIGITO('5'));
    VERIFICA(E_LETRA('b'));
    VERIFICA(CARACTERE_MAIUSCULO('b') == 'B');
    VERIFICA(CARACTERE_MINUSCULO('B') == 'b');
}

static void testa_matematica(void)
{
    const char *arquivo = "teste.c";

    VERIFICA(bg_divide_i(-7, 2, arquivo, 1) == -3);
    VERIFICA(bg_divide_l(7L, 2L, arquivo, 1) == 3L);
    VERIFICA(bg_divide_ll(7LL, 2LL, arquivo, 1) == 3LL);
    VERIFICA(bg_divide_u(7U, 2U, arquivo, 1) == 3U);
    VERIFICA(bg_divide_ul(7UL, 2UL, arquivo, 1) == 3UL);
    VERIFICA(bg_divide_ull(7ULL, 2ULL, arquivo, 1) == 3ULL);
    VERIFICA(bg_divide_f(7.0f, 2.0f, arquivo, 1) == 3.5f);
    VERIFICA(bg_divide_d(7.0, 2.0, arquivo, 1) == 3.5);
    VERIFICA(bg_divide_ld(7.0L, 2.0L, arquivo, 1) == 3.5L);
    if (INT_MIN == -INT_MAX) {
        VERIFICA(bg_divide_i(INT_MIN, -1, arquivo, 1) == INT_MAX);
    }
    if (LONG_MIN == -LONG_MAX) {
        VERIFICA(bg_divide_l(LONG_MIN, -1L, arquivo, 1) == LONG_MAX);
    }
    if (LLONG_MIN == -LLONG_MAX) {
        VERIFICA(bg_divide_ll(LLONG_MIN, -1LL, arquivo, 1) == LLONG_MAX);
    }

    VERIFICA(bg_resto_i(-7, 3, arquivo, 1) == -1);
    VERIFICA(bg_resto_l(7L, 3L, arquivo, 1) == 1L);
    VERIFICA(bg_resto_ll(7LL, 3LL, arquivo, 1) == 1LL);
    VERIFICA(bg_resto_u(7U, 3U, arquivo, 1) == 1U);
    VERIFICA(bg_resto_ul(7UL, 3UL, arquivo, 1) == 1UL);
    VERIFICA(bg_resto_ull(7ULL, 3ULL, arquivo, 1) == 1ULL);
    if (INT_MIN == -INT_MAX) {
        VERIFICA(bg_resto_i(INT_MIN, -1, arquivo, 1) == 0);
    }
    if (LONG_MIN == -LONG_MAX) {
        VERIFICA(bg_resto_l(LONG_MIN, -1L, arquivo, 1) == 0L);
    }
    if (LLONG_MIN == -LLONG_MAX) {
        VERIFICA(bg_resto_ll(LLONG_MIN, -1LL, arquivo, 1) == 0LL);
    }

    VERIFICA(bg_abs_i(-7) == 7);
    VERIFICA(bg_abs_l(-7L) == 7L);
    VERIFICA(bg_abs_ll(-7LL) == 7LL);
    VERIFICA(bg_abs_f(-2.5f) == 2.5f);
    VERIFICA(bg_abs_d(-2.5) == 2.5);
    if (INT_MIN == -INT_MAX) {
        VERIFICA(bg_abs_i(INT_MIN) == INT_MAX);
    }
    if (LONG_MIN == -LONG_MAX) {
        VERIFICA(bg_abs_l(LONG_MIN) == LONG_MAX);
    }
    if (LLONG_MIN == -LLONG_MAX) {
        VERIFICA(bg_abs_ll(LLONG_MIN) == LLONG_MAX);
    }
    VERIFICA(signbit(bg_abs_d(-0.0)) == 0);
    VERIFICA(bg_min_i(3, 9) == 3);
    VERIFICA(bg_min_l(3L, 9L) == 3L);
    VERIFICA(bg_min_d(3.0, 9.0) == 3.0);
    VERIFICA(bg_max_i(3, 9) == 9);
    VERIFICA(bg_max_l(3L, 9L) == 9L);
    VERIFICA(bg_max_d(3.0, 9.0) == 9.0);
    VERIFICA(bg_round_l(2.5) == 3LL);
    VERIFICA(bg_round_l(-2.5) == -3LL);

    VERIFICA(ABS(-7) == 7);
    VERIFICA(MINIMO(3, 9) == 3);
    VERIFICA(MAXIMO(3, 9) == 9);
    VERIFICA(fabs(POTENCIA(2.0, 10.0) - 1024.0) < 0.0001);
    VERIFICA(RAIZ(144.0) == 12.0);
    VERIFICA(ARREDONDA(-2.5) == -3LL);
    VERIFICA(PISO(2.9) == 2.0);
    VERIFICA(TETO(2.1) == 3.0);
    VERIFICA(DIVIDE(-7, 2) == -3);
    VERIFICA(RESTO(-7, 3) == -1);
}

static void testa_utilidades(void)
{
    unsigned sorteado = 99U;
    unsigned primeiro;
    unsigned segundo;
    int aleatorio;
    unsigned universo = (unsigned) RAND_MAX + 1U;

    bg_inicia_sorteio(1234U);
    VERIFICA(bg_sorteia_ate(100U, &sorteado));
    VERIFICA(sorteado < 100U);
    primeiro = sorteado;
    bg_inicia_sorteio(1234U);
    VERIFICA(bg_sorteia_ate(100U, &sorteado));
    segundo = sorteado;
    VERIFICA(primeiro == segundo); /* semente explícita, sequência repetível */

    sorteado = 99U;
    VERIFICA(!bg_sorteia_ate(0U, &sorteado));
    VERIFICA(sorteado == 99U);
    VERIFICA(!bg_sorteia_ate(10U, NULL));
    VERIFICA(bg_sorteia_ate(universo, &sorteado));
    VERIFICA(sorteado <= (unsigned) RAND_MAX);
    VERIFICA(!bg_sorteia_ate(universo + 1U, &sorteado));
    VERIFICA(sorteado <= (unsigned) RAND_MAX);
    VERIFICA(bg_sorteia(1) == 0);
    aleatorio = bg_sorteia(100);
    VERIFICA(aleatorio >= 0 && aleatorio < 100);
    VERIFICA(bg_sorteia(0) == 0);
    VERIFICA(bg_sorteia(-3) == 0);
    VERIFICA(SORTEIA(1) == 0);
}

static int testa_stdin_compativel(void)
{
    char linha[64];
    int inteiro = 0;
    double real = 0.0;

    if (scanf("%d", &inteiro) != 1 || inteiro != 42
        || !bg_leia_texto(linha, sizeof linha)
        || strcmp(linha, "linha de texto") != 0
        || !bg_leia_inteiro(&inteiro) || inteiro != -7
        || !bg_leia_real(&real) || real != 3.5) {
        (void) fprintf(stderr, "falha nos wrappers de stdin\n");
        return 1;
    }
    (void) puts("stdin: ok");
    return 0;
}

static int testa_stdin_invalido(void)
{
    int inteiro = 71;
    double real = 8.25;

    if (bg_leia_inteiro(&inteiro) || inteiro != 71
        || bg_leia_real(&real) || real != 8.25) {
        (void) fprintf(stderr, "falha nos wrappers numericos invalidos\n");
        return 1;
    }
    (void) puts("stdin invalido: ok");
    return 0;
}

static int testa_stdin_texto_longo(void)
{
    char pequeno[4];
    char seguinte[8];

    if (bg_leia_texto(pequeno, sizeof pequeno)
        || pequeno[0] != '\0'
        || !bg_leia_texto(seguinte, sizeof seguinte)
        || strcmp(seguinte, "ok") != 0) {
        (void) fprintf(stderr, "falha no descarte de linha longa\n");
        return 1;
    }
    (void) puts("linha longa: ok");
    return 0;
}

static int executa_caso_fatal(const char *caso)
{
    if (strcmp(caso, "erro") == 0) {
        bg_erro("teste.c", 37, "falha esperada");
    } else if (strcmp(caso, "passo") == 0) {
        (void) bg_passo_zero("teste.c", 38);
    } else if (strcmp(caso, "divide-zero") == 0) {
        (void) bg_divide_i(1, 0, "teste.c", 39);
    } else if (strcmp(caso, "divide-estouro") == 0) {
        if (INT_MIN == -INT_MAX) return 3;
        (void) bg_divide_i(INT_MIN, -1, "teste.c", 40);
    } else if (strcmp(caso, "divide-estouro-l") == 0) {
        if (LONG_MIN == -LONG_MAX) return 3;
        (void) bg_divide_l(LONG_MIN, -1L, "teste.c", 56);
    } else if (strcmp(caso, "divide-estouro-ll") == 0) {
        if (LLONG_MIN == -LLONG_MAX) return 3;
        (void) bg_divide_ll(LLONG_MIN, -1LL, "teste.c", 57);
    } else if (strcmp(caso, "divide-zero-l") == 0) {
        (void) bg_divide_l(1L, 0L, "teste.c", 43);
    } else if (strcmp(caso, "divide-zero-ll") == 0) {
        (void) bg_divide_ll(1LL, 0LL, "teste.c", 44);
    } else if (strcmp(caso, "divide-zero-u") == 0) {
        (void) bg_divide_u(1U, 0U, "teste.c", 45);
    } else if (strcmp(caso, "divide-zero-ul") == 0) {
        (void) bg_divide_ul(1UL, 0UL, "teste.c", 46);
    } else if (strcmp(caso, "divide-zero-ull") == 0) {
        (void) bg_divide_ull(1ULL, 0ULL, "teste.c", 47);
    } else if (strcmp(caso, "divide-zero-f") == 0) {
        (void) bg_divide_f(1.0F, 0.0F, "teste.c", 48);
    } else if (strcmp(caso, "divide-zero-d") == 0) {
        (void) bg_divide_d(1.0, 0.0, "teste.c", 49);
    } else if (strcmp(caso, "divide-zero-ld") == 0) {
        (void) bg_divide_ld(1.0L, 0.0L, "teste.c", 50);
    } else if (strcmp(caso, "resto-zero") == 0) {
        (void) bg_resto_i(1, 0, "teste.c", 41);
    } else if (strcmp(caso, "resto-estouro") == 0) {
        if (INT_MIN == -INT_MAX) return 3;
        (void) bg_resto_i(INT_MIN, -1, "teste.c", 42);
    } else if (strcmp(caso, "resto-estouro-l") == 0) {
        if (LONG_MIN == -LONG_MAX) return 3;
        (void) bg_resto_l(LONG_MIN, -1L, "teste.c", 58);
    } else if (strcmp(caso, "resto-estouro-ll") == 0) {
        if (LLONG_MIN == -LLONG_MAX) return 3;
        (void) bg_resto_ll(LLONG_MIN, -1LL, "teste.c", 59);
    } else if (strcmp(caso, "resto-zero-l") == 0) {
        (void) bg_resto_l(1L, 0L, "teste.c", 51);
    } else if (strcmp(caso, "resto-zero-ll") == 0) {
        (void) bg_resto_ll(1LL, 0LL, "teste.c", 52);
    } else if (strcmp(caso, "resto-zero-u") == 0) {
        (void) bg_resto_u(1U, 0U, "teste.c", 53);
    } else if (strcmp(caso, "resto-zero-ul") == 0) {
        (void) bg_resto_ul(1UL, 0UL, "teste.c", 54);
    } else if (strcmp(caso, "resto-zero-ull") == 0) {
        (void) bg_resto_ull(1ULL, 0ULL, "teste.c", 55);
    } else if (strcmp(caso, "abs-minimo") == 0) {
        if (INT_MIN == -INT_MAX) {
            return 3; /* nesta representação, a magnitude ainda cabe em int */
        }
        (void) bg_abs_i(INT_MIN);
    } else if (strcmp(caso, "abs-minimo-l") == 0) {
        if (LONG_MIN == -LONG_MAX) {
            return 3;
        }
        (void) bg_abs_l(LONG_MIN);
    } else if (strcmp(caso, "abs-minimo-ll") == 0) {
        if (LLONG_MIN == -LLONG_MAX) {
            return 3;
        }
        (void) bg_abs_ll(LLONG_MIN);
    } else {
        return 2;
    }
    return 0; /* um caso fatal não deve chegar aqui */
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--stdin") == 0) {
        return testa_stdin_compativel();
    }
    if (argc == 2 && strcmp(argv[1], "--stdin-invalido") == 0) {
        return testa_stdin_invalido();
    }
    if (argc == 2 && strcmp(argv[1], "--stdin-texto-longo") == 0) {
        return testa_stdin_texto_longo();
    }
    if (argc == 2) {
        return executa_caso_fatal(argv[1]);
    }

    testa_console();
    testa_entrada_numerica();
    testa_texto();
    testa_caracteres();
    testa_matematica();
    testa_utilidades();

    if (falhas != 0) {
        (void) fprintf(stderr, "%u falhas em %u verificacoes\n", falhas, verificacoes);
        return 1;
    }
    (void) printf("biblioteca: %u verificacoes aprovadas\n", verificacoes);
    return 0;
}
