# Tutorial rápido (Português) — BASICGOL

## 1. Hello, world

```c
#include <basicgol.h>

INICIO
    ESCREVAL("Ola, mundo!");
FIM
```

Compile com `cc -std=c11 -I include ola.c build/libbasicgol.a -lm -o ola`.

## 2. Variáveis e aritmética

```c
INICIO
    INTEIRO a = 10;
    INTEIRO b = 3;
    ESCREVAL("soma = %d", a + b);
FIM
```

## 3. Controle de fluxo

```c
INICIO
    INTEIRO i = 0;
    PARA (i, 1, 5) {
        SE i % 2 == 0 ENTAO
            ESCREVAL("%d par", i);
        SENAO
            ESCREVAL("%d impar", i);
        FIM_SE
    }
FIM
```

## 4. Funções

```c
FUNCAO INTEIRO dobro(INTEIRO x) {
    RETORNE x * 2;
}

INICIO
    ESCREVAL("dobro(7) = %d", dobro(7));
FIM
```

## 5. Biblioteca padrão

Inclua os módulos em `include/basicgol/` (matemática, texto, console, caracteres,
utilidades). Veja `docs/09-biblioteca-padrao.md`.

## Próximos passos

- Referência de palavras-chave: `docs/02-referencia.md`.
- Exemplos: `examples/beginner`, `examples/intermediate`, `examples/advanced`.
- Testes: `make test`.
