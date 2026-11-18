# Changelog — BASICGOL

## Não publicado

### Added
- `playground/`: página estática que compila e executa BASICGOL **no navegador**
  (clang e wasm-ld em WebAssembly, sem servidor, sem enviar código para fora).
  Os 22 exemplos de `docs/exemplos/` estão no seletor, com a entrada de teste
  dourada; entrada interativa por JSPI quando o navegador tem
  (`WebAssembly.Suspending`).
- Verificação em três níveis, todos contra `docs/exemplos/*.esperado.txt`:
  `playground/tools/verificar.js` (toolchain, memfs reimplementado),
  `playground/tools/verificar-navegador.mjs` (o código da página importado como
  módulo ES) e `playground/tools/verificar-pagina.mjs` (a página num Chromium
  headless, via CDP).
- `playground/README.md` e `playground/ATRIBUICOES.md` (licenças de wasm-clang/
  LLVM, wasi-libc, Ace, xterm.js, Font Awesome, Montserrat e Ubuntu Mono).

### Fixed
- `playground/tools/shim.c`: `long double` (`BG_REAL_LONGO`) e `%zu` no `printf`
  da wasi-libc de 2019. As funções `v*` não são substituídas (recursão).

## 1.0.0 (2026-11-09)

### Added
- Phase 7 developer tooling and documentation: BUILDING.md, README.pt.md,
  tutorial-quickstart.pt.md, feature-matrix.md, limitations.md, examples/,
  RELEASE_CHECKLIST.md.
- Phase 8 release artifacts: CHANGELOG.md, VERSIONING.md, TEST_REPORT.md,
  api-reference.md.

### Core
- Language 1.1.0, library 1.0.0 (see README.md).
- M0 macro path and M1 transpiler path (`basicgolc`).
- Standard library modules: caractere, consola, matematica, texto, utilidades.
