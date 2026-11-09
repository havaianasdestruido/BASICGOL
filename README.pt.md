# BASICGOL (Português)

> BASICGOL — a legibilidade do BASIC e do Portugol, entregue como uma camada de macros C.

Linguagem didática cuja sintaxe usa palavras-chave em português (`SE`, `ENTAO`,
`ENQUANTO`, `ESCREVAL`, `REGISTRO` …) e cujo modelo de execução é **exatamente o
de C**: sem máquina virtual, coletor de lixo, interpretador ou compilador próprio
no caminho crítico.

## Início rápido

1. Pré-requisitos: compilador C11 (GCC/Clang/MSVC), `make`, `ar`.
2. Compile:

```sh
make
```

3. Programa mínimo (`ola.c`):

```c
#include <basicgol.h>

INICIO
    ESCREVAL("Ola, mundo!");
FIM
```

4. Compile e execute:

```sh
cc -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include ola.c \
   build/libbasicgol.a -lm -o ola
./ola
```

## Tutorial

Veja `docs/tutorial-quickstart.pt.md`.

## Testes

```sh
make test
```

## Documentação

- `docs/02-referencia.md` — referência de palavras-chave.
- `docs/feature-matrix.md` — matriz de funcionalidades.
- `docs/limitations.md` — limitações.
- `BUILDING.md` — instalação e compilação.
- `RELEASE_CHECKLIST.md` — checklist de release.
