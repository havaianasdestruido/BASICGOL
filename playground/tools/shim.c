/*
 * shim.c — ajustes de execução do playground, compilados junto com o seu
 * programa. Não faz parte da linguagem nem da biblioteca BASICGOL.
 *
 * 1. Terminal sem atraso. Sem isto, `printf` acumula a saída em buffer
 *    (stdout de pipe é completamente bufferizado) e um prompt como
 *    `ESCREVA("Seu nome: ")` só apareceria depois do `LEIA` — a tela ficaria
 *    parada justamente durante a leitura.
 *
 * 2. Arquivos temporários. O módulo `filesystem` da biblioteca usa `mkstemp`,
 *    que a wasi-libc usada pelo playground não tem. O sandbox do navegador não
 *    tem disco mesmo: a implementação abaixo falha com ENOSYS, que é o
 *    resultado honesto (a função da biblioteca devolve erro em vez de o
 *    programa nem ligar).
 *
 * 3. `long double` na impressão. No wasm32 o `long double` ocupa 16 bytes
 *    (como o x87), mas o printf da wasi-libc de 2019 só foi compilado para 8:
 *    pedir `%Lg` chama `long_double_not_supported()`, que aborta o programa. O
 *    valor, porém, tem só 64 bits de precisão (não os 80 do x87), então
 *    imprimir como `double` dá exatamente o mesmo texto. As funções abaixo
 *    substituem `printf`/`fprintf`/`sprintf`/`snprintf` e percorrem o formato
 *    convertendo cada argumento com o tipo certo — sem montar uma `va_list`
 *    nova (o `__builtin_va_arg` do clang não é atribuível). Cada conversão é
 *    formatada pelo `vsnprintf` **da libc**; só as quatro funções acima são
 *    substituídas, justamente para que essa chamada não volte para cá.
 */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

__attribute__((constructor)) static void basicgol_playground_inicia(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

int mkstemp(char *modelo)
{
    (void) modelo;
    errno = ENOSYS;
    return -1;
}

/* --------------------------------------------------------- saída em buffer */

struct saida {
    char *dados;
    size_t usado;
    size_t alocado;
    int falhou; /* 1 = ficou sem memória */
};

static void acrescentar(struct saida *s, const char *texto, size_t n)
{
    if (s->falhou) return;
    if (s->usado + n + 1 > s->alocado) {
        size_t novo = s->alocado ? s->alocado * 2 : 256;
        while (novo < s->usado + n + 1) novo *= 2;
        char *maior = (char *) realloc(s->dados, novo);
        if (!maior) { s->falhou = 1; return; }
        s->dados = maior;
        s->alocado = novo;
    }
    for (size_t i = 0; i < n; i++) s->dados[s->usado + i] = texto[i];
    s->usado += n;
    s->dados[s->usado] = '\0';
}

/** Escreve uma conversão no buffer de saída (crescendo se precisar). */
static void acrescentar_convertido(struct saida *s, const char *especificacao, ...)
{
    va_list argumentos;
    char pequeno[256];
    va_start(argumentos, especificacao);
    int n = vsnprintf(pequeno, sizeof(pequeno), especificacao, argumentos);
    va_end(argumentos);
    if (n < 0) { s->falhou = 1; return; }
    if ((size_t) n < sizeof(pequeno)) {
        acrescentar(s, pequeno, (size_t) n);
        return;
    }
    char *grande = (char *) malloc((size_t) n + 1);
    if (!grande) { s->falhou = 1; return; }
    va_start(argumentos, especificacao);
    vsnprintf(grande, (size_t) n + 1, especificacao, argumentos);
    va_end(argumentos);
    acrescentar(s, grande, (size_t) n);
    free(grande);
}

/* ----------------------------------------------------- percurso do formato */

static int eh_inteira(char c)
{
    return c == 'd' || c == 'i' || c == 'o' || c == 'u' || c == 'x' || c == 'X';
}

static int eh_real(char c)
{
    return c == 'e' || c == 'E' || c == 'f' || c == 'F' || c == 'g' || c == 'G'
        || c == 'a' || c == 'A';
}

/** Escreve um inteiro decimal em `destino` (para largura/precisão com `*`). */
static void escrever_inteiro(char *destino, size_t *usado, int valor)
{
    char digitos[12];
    int n = 0;
    int negativo = valor < 0;
    unsigned int v = negativo ? (unsigned int) (-valor) : (unsigned int) valor;
    do {
        digitos[n++] = (char) ('0' + v % 10u);
        v /= 10u;
    } while (v);
    if (negativo) destino[(*usado)++] = '-';
    while (n) destino[(*usado)++] = digitos[--n];
}

/**
 * Percorre `formato` e monta a saída em `s`, lendo os argumentos de `ap` cada
 * um com o tipo que o formato pede.
 */
static void formatar(struct saida *s, const char *formato, va_list *ap)
{
    const char *p = formato;
    while (*p && !s->falhou) {
        if (*p != '%') {
            const char *inicio = p;
            while (*p && *p != '%') p++;
            acrescentar(s, inicio, (size_t) (p - inicio));
            continue;
        }
        p++; /* '%' */
        if (*p == '%') {
            acrescentar(s, "%", 1);
            p++;
            continue;
        }

        /* Bandeiras. */
        char especificacao[64];
        size_t n = 0;
        especificacao[n++] = '%';
        while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0') {
            if (n < sizeof(especificacao) - 2) especificacao[n++] = *p;
            p++;
        }

        /* Largura: número literal ou `*` (que vem de um int). */
        if (*p == '*') {
            escrever_inteiro(especificacao, &n, va_arg(*ap, int));
            p++;
        } else {
            while (*p >= '0' && *p <= '9') {
                if (n < sizeof(especificacao) - 2) especificacao[n++] = *p;
                p++;
            }
        }

        /* Precisão. */
        if (*p == '.') {
            if (n < sizeof(especificacao) - 2) especificacao[n++] = *p;
            p++;
            if (*p == '*') {
                escrever_inteiro(especificacao, &n, va_arg(*ap, int));
                p++;
            } else {
                while (*p >= '0' && *p <= '9') {
                    if (n < sizeof(especificacao) - 2) especificacao[n++] = *p;
                    p++;
                }
            }
        }

        /* Modificador de comprimento. */
        int real_longo = 0;
        int tamanho_inteiro = 0; /* 0 = int, 1 = long long, 2 = curto */
        if (*p == 'h') {
            p++;
            if (*p == 'h') p++;
            tamanho_inteiro = 2;
        } else if (*p == 'l') {
            if (n < sizeof(especificacao) - 2) especificacao[n++] = *p;
            p++;
            if (*p == 'l') {
                if (n < sizeof(especificacao) - 2) especificacao[n++] = *p;
                p++;
                tamanho_inteiro = 1;
            }
        } else if (*p == 'j' || *p == 'z' || *p == 't') {
            if (n < sizeof(especificacao) - 2) especificacao[n++] = *p;
            char modificador = *p;
            p++;
            /* No wasm32 `size_t` e `ptrdiff_t` têm 4 bytes (igual a int); só
             * `intmax_t` (`j`) é de 8. */
            if (modificador == 'j') tamanho_inteiro = 1;
        } else if (*p == 'L') {
            /* O `L` sai do formato: o argumento vira `double`. */
            p++;
            real_longo = 1;
        }

        char conversao = *p ? *p++ : '\0';
        if (n < sizeof(especificacao) - 1) especificacao[n++] = conversao;
        especificacao[n] = '\0';

        if (eh_real(conversao)) {
            if (real_longo) {
                long double valor = va_arg(*ap, long double);
                acrescentar_convertido(s, especificacao, (double) valor);
            } else {
                double valor = va_arg(*ap, double);
                acrescentar_convertido(s, especificacao, valor);
            }
        } else if (eh_inteira(conversao)) {
            if (tamanho_inteiro == 1) {
                long long valor = va_arg(*ap, long long);
                acrescentar_convertido(s, especificacao, valor);
            } else {
                /* `h`/`hh` também chegam promovidos a int. */
                int valor = va_arg(*ap, int);
                acrescentar_convertido(s, especificacao, valor);
            }
        } else if (conversao == 'c') {
            int valor = va_arg(*ap, int);
            acrescentar_convertido(s, especificacao, valor);
        } else if (conversao == 's') {
            const char *valor = va_arg(*ap, const char *);
            acrescentar_convertido(s, especificacao, valor);
        } else if (conversao == 'p') {
            void *valor = va_arg(*ap, void *);
            acrescentar_convertido(s, especificacao, valor);
        } else if (conversao == 'Z') {
            /* `%zu` é C99; o printf do wasi-libc 2019 só entende `%z` antes de
             * `d/i/o/u/x/X`. Para `s` (tamanho), tratamos aqui mesmo — e o
             * exemplo `biblioteca_texto.c` do repositório usa `%zu`. */
            size_t valor = va_arg(*ap, size_t);
            especificacao[n - 1] = 'z';
            especificacao[n] = 'u';
            especificacao[n + 1] = '\0';
            acrescentar_convertido(s, especificacao, valor);
        } else if (conversao == 'n') {
            int *valor = va_arg(*ap, int *);
            if (valor) *valor = (int) s->usado;
        } else if (conversao) {
            /* Conversão que não conhecemos: mantém o texto do formato. */
            acrescentar(s, especificacao, n);
        }
    }
}

/* --------------------------------------------------- funções da biblioteca */

int printf(const char *formato, ...)
{
    va_list argumentos;
    va_start(argumentos, formato);
    struct saida s = {0};
    formatar(&s, formato, &argumentos);
    va_end(argumentos);
    int escritos = -1;
    if (!s.falhou) {
        escritos = (int) s.usado;
        if (s.usado) fwrite(s.dados, 1, s.usado, stdout);
    }
    free(s.dados);
    return escritos;
}

int fprintf(FILE *arquivo, const char *formato, ...)
{
    va_list argumentos;
    va_start(argumentos, formato);
    struct saida s = {0};
    formatar(&s, formato, &argumentos);
    va_end(argumentos);
    int escritos = -1;
    if (!s.falhou) {
        escritos = (int) s.usado;
        if (s.usado) fwrite(s.dados, 1, s.usado, arquivo);
    }
    free(s.dados);
    return escritos;
}

int sprintf(char *alvo, const char *formato, ...)
{
    va_list argumentos;
    va_start(argumentos, formato);
    struct saida s = {0};
    formatar(&s, formato, &argumentos);
    va_end(argumentos);
    if (s.falhou) { free(s.dados); return -1; }
    size_t i = 0;
    for (; i < s.usado; i++) alvo[i] = s.dados[i];
    alvo[i] = '\0';
    int escritos = (int) s.usado;
    free(s.dados);
    return escritos;
}

int snprintf(char *alvo, size_t tamanho, const char *formato, ...)
{
    va_list argumentos;
    va_start(argumentos, formato);
    struct saida s = {0};
    formatar(&s, formato, &argumentos);
    va_end(argumentos);
    if (s.falhou) { free(s.dados); return -1; }
    if (tamanho) {
        size_t i = 0;
        for (; i + 1 < tamanho && i < s.usado; i++) alvo[i] = s.dados[i];
        alvo[i] = '\0';
    }
    int escritos = (int) s.usado; /* como o padrão: o que *caber* ia escrever */
    free(s.dados);
    return escritos;
}
