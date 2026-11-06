/* entrada.c — falhas de entrada, contratos de leitura e recuperação.
 *
 * Parte da suíte de portabilidade da Fase 6 (tests/run_portabilidade.sh).
 * A entrada é fixa (entrada.entrada.txt) e a saída é comparada linha a linha
 * com entrada.esperado.txt — este caso testa o CONTRATO de leitura, incluindo
 * o que acontece quando a leitura falha.
 *
 * Compilar: cc -std=c11 -Wall -Wextra -Wformat=2 -Werror -I include \
 *              tests/portabilidade/entrada.c build/libbasicgol.a -lm
 */
#include <basicgol.h>

INICIO
    INTEIRO n = -1;
    REAL r = -1.0;
    CARACTERE c = '?';
    TEXTO_FIXO(linha, 32);
    TEXTO_FIXO(curto, 4);
    LOGICO ok;

    /* 1. número válido: LEIA_OK devolve verdadeiro */
    ok = LEIA_OK(n);
    ESCREVA_FMT("1 ok=%d n=%d\n", (int) ok, n);

    /* 2. LEIA_TEXTO depois de LEIA consome o \n pendente e lê a linha seguinte */
    ok = LEIA_TEXTO(linha, TAMANHO_DE(linha));
    ESCREVA_FMT("2 ok=%d linha=[%s]\n", (int) ok, linha);

    /* 3. texto onde se esperava número: falha e o destino NÃO muda */
    n = 123;
    ok = LEIA_OK(n);
    ESCREVA_FMT("3 ok=%d n=%d\n", (int) ok, n);

    /* 4. recuperação: a linha inválida continua no fluxo e é consumida aqui */
    ok = LEIA_TEXTO(linha, TAMANHO_DE(linha));
    ESCREVA_FMT("4 ok=%d linha=[%s]\n", (int) ok, linha);

    /* 5. LEIA (scanf) lê o número e deixa o resto da linha no fluxo */
    n = -1;
    LEIA(n);
    ESCREVA_FMT("5 n=%d\n", n);

    /* 6. o resto da linha é lido como texto, com o espaço à frente */
    ok = LEIA_TEXTO(linha, TAMANHO_DE(linha));
    ESCREVA_FMT("6 ok=%d linha=[%s]\n", (int) ok, linha);

    /* 7. LEIA_REAL */
    ok = LEIA_REAL(r);
    ESCREVA_FMT("7 ok=%d r=%g\n", (int) ok, r);

    /* 8. LEIA de CARACTERE depois de uma leitura numérica (i15) */
    LEIA(c);
    ESCREVA_FMT("8 c=%c\n", c);

    /* 9. LEIA_INTEIRO com lixo depois dos dígitos: recusa a linha inteira */
    n = 77;
    ok = LEIA_INTEIRO(n);
    ESCREVA_FMT("9 ok=%d n=%d\n", (int) ok, n);

    /* 10. LEIA_TEXTO com buffer pequeno: falha, destino vazio, resto consumido */
    ok = LEIA_TEXTO(curto, TAMANHO_DE(curto));
    ESCREVA_FMT("10 ok=%d curto=[%s]\n", (int) ok, curto);

    /* 11. e o fluxo continua sincronizado na linha seguinte */
    ok = LEIA_TEXTO(linha, TAMANHO_DE(linha));
    ESCREVA_FMT("11 ok=%d linha=[%s]\n", (int) ok, linha);

    /* 12. EOF: falha em vez de repetir a última leitura */
    n = 5;
    ok = LEIA_TEXTO(linha, TAMANHO_DE(linha));
    ESCREVA_FMT("12 ok=%d linha=[%s]\n", (int) ok, linha);
    ok = LEIA_INTEIRO(n);
    ESCREVA_FMT("13 ok=%d n=%d\n", (int) ok, n);
    LEIA(n);
    ESCREVA_FMT("14 n=%d\n", n);

    ESCREVAL("entrada: ok");
FIM
