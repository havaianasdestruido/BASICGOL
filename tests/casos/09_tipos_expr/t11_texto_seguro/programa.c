// Texto seguro: toda operação reporta truncamento e nunca deixa lixo parcial
#include <basicgol.h>

INICIO
    TEXTO_FIXO(buf, 8);
    TEXTO_FIXO(dest, 4);

    // --- cópia: 7 caracteres + NUL cabem em 8; 8 não cabem
    ESCREVAL(COPIA_TEXTO(buf, TAMANHO_VETOR(buf), "abc"));
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();
    ESCREVAL(COPIA_TEXTO(buf, TAMANHO_VETOR(buf), "1234567"));
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();
    ESCREVAL(COPIA_TEXTO(buf, TAMANHO_VETOR(buf), "12345678"));
    ESCREVAL(TAMANHO_TEXTO(buf));      // 0: falha limpa o destino

    // --- junção de dois textos
    ESCREVAL(JUNTA_TEXTO(buf, TAMANHO_VETOR(buf), "ab", "cd"));
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();
    ESCREVAL(JUNTA_TEXTO(buf, TAMANHO_VETOR(buf), "abcd", "efgh"));
    ESCREVAL(TAMANHO_TEXTO(buf));      // 0

    // --- acréscimo ao que já existe (o strncat seguro)
    ESCREVAL(COPIA_TEXTO(buf, TAMANHO_VETOR(buf), "ab"));
    ESCREVAL(ACRESCENTA_TEXTO(buf, TAMANHO_VETOR(buf), "cd"));
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();
    ESCREVAL(ACRESCENTA_TEXTO(buf, TAMANHO_VETOR(buf), "efgh"));
    ESCREVAL(TAMANHO_TEXTO(buf));      // 0: 6 caracteres não caberiam

    // --- trecho: quantidade limitada ao que existe, início além do fim = vazio
    ESCREVAL(TRECHO_TEXTO(buf, TAMANHO_VETOR(buf), "abcdef", 2, 3));
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();
    ESCREVAL(TRECHO_TEXTO(buf, TAMANHO_VETOR(buf), "abcdef", 4, 99));
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();
    ESCREVAL(TRECHO_TEXTO(buf, TAMANHO_VETOR(buf), "abcdef", 99, 1));
    ESCREVAL(TAMANHO_TEXTO(buf));
    ESCREVAL(TRECHO_TEXTO(buf, TAMANHO_VETOR(buf), NULL, 0, 1));

    // --- formatação verificável (sem inspeção de formato pelo compilador)
    ESCREVAL(FORMATA_TEXTO(buf, TAMANHO_VETOR(buf), "%d-%s", 42, "xy"));
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();
    ESCREVAL(FORMATA_TEXTO(dest, TAMANHO_VETOR(dest), "%s", "longo"));
    ESCREVAL(TAMANHO_TEXTO(dest));
    TEXTO formato = "<%d>";
    ESCREVAL(FORMATA_TEXTO(dest, TAMANHO_VETOR(dest), formato, 7));
    ESCREVA_TEXTO(dest); ESCREVA_LINHA();

    // --- comparação normalizada e texto vazio (NULL conta como vazio)
    ESCREVAL(COMPARA_TEXTO("a", "b"));
    ESCREVAL(COMPARA_TEXTO("b", "a"));
    ESCREVAL(COMPARA_TEXTO("a", "a"));
    ESCREVAL(COMPARA_TEXTO(NULL, ""));
    ESCREVAL(TEXTO_VAZIO(""));
    ESCREVAL(TEXTO_VAZIO(NULL));
    ESCREVAL(TEXTO_VAZIO("x"));

    // --- origem NULL nunca é desreferenciada
    ESCREVAL(COPIA_TEXTO(buf, TAMANHO_VETOR(buf), NULL));
    ESCREVAL(TAMANHO_TEXTO(buf));

    // --- origem e destino podem ser o mesmo buffer (as cópias usam memmove)
    COPIA_TEXTO(buf, TAMANHO_VETOR(buf), "abcdef");
    ESCREVAL(TRECHO_TEXTO(buf, TAMANHO_VETOR(buf), buf, 2, 3));
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();          // cde
    ESCREVAL(ACRESCENTA_TEXTO(buf, TAMANHO_VETOR(buf), buf));  // "cde"+"cde"
    ESCREVA_TEXTO(buf); ESCREVA_LINHA();          // cdecde
    ESCREVAL(ACRESCENTA_TEXTO(buf, 6, buf));      // nao cabe em 6: falha
    ESCREVAL(TAMANHO_TEXTO(buf));
FIM
