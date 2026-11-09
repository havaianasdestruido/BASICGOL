# Building BASICGOL

This document covers installing prerequisites, building the compiler (`basicgolc`),
building the static library, running the test suite, and troubleshooting.

## Prerequisites

- A C11 compiler: GCC, Clang, or MSVC. MinGW-w64 and `zig cc` are also used in CI.
- `make` (POSIX) **or** the equivalent compiler invocation shown below.
- `ar` (to build `libbasicgol.a`).
- `bash`, `diff`, `mktemp` (for the test runners).
- Python 3 (only for regenerating coverage/docs).

No runtime dependencies. The language compiles to plain C.

## Repository layout

```
include/           Public C headers (basicgol.h + basicgol/*.h)
src/biblioteca/    Standard library implementation (.c)
tools/basicgolc.c  Transpiler (M1 path)
tests/             Language + library test suites
docs/              Specification, reference, compatibility matrix
examples/          Beginner / intermediate / advanced programs
website/           Docusaurus site (optional)
```

## Build

```sh
make                  # builds build/basicgolc and build/libbasicgol.a
make biblioteca       # only the static library
make test             # language suite + library tests
make test-v           # verbose (lists every check)
make exemplos         # compiles docs/exemplos
make limpar           # clean
```

### Windows (MSVC)

Open a "Developer Command Prompt" and run:

```bat
if not exist build mkdir build
cl /std:c11 /W3 /Fe:build\basicgolc.exe tools\basicgolc.c
cl /std:c11 /W3 /I include /c src\biblioteca\*.c
lib /OUT:build\libbasicgol.lib src\biblioteca\*.obj
```

Then compile a program:

```bat
cl /std:c11 /W3 /I include programa.c build\libbasicgol.lib
```

### Manual invocation (any compiler)

```sh
cc -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include programa.c \
   build/libbasicgol.a -lm -o programa
```

Transpiler (M1) path, when using `PARA i DE ... ATE ... FACA` sugar:

```sh
build/basicgolc programa.c -o programa.gerado.c
cc -std=c11 -Wall -Wextra -Wformat=2 -Wvla -I include programa.gerado.c \
   build/libbasicgol.a -lm -o programa
```

## Include paths

- Add `-I include` so `#include <basicgol.h>` and `#include <basicgol/...>` resolve.
- Link `build/libbasicgol.a` (or the `.lib` on Windows) plus `-lm` on platforms where libm is separate.

## Tests

```sh
make test
```

Runs `tests/run.sh` (language M0/M1 equivalence) and `tests/run_biblioteca.sh`
(library unit tests). See `tests/README.md` for case conventions.

## Troubleshooting

- **"basicgol.h not found"**: missing `-I include`.
- **Undefined references to math functions**: add `-lm` (Linux/glibc).
- **M0 vs M1 mismatch**: a construct not reducible to macros; check `docs/04-restricoes-e-decisoes.md`.
- **Windows `strcpy`/`fopen` deprecation warnings**: harmless; define `_CRT_SECURE_NO_WARNINGS` to silence.
- **ASan/UBSan**: `make test-sanitize` (GCC/Clang only).
