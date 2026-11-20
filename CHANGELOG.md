# Changelog — BASICGOL

## Não publicado

### Added
- `playground/`: página estática que compila e executa BASICGOL **no navegador**
  (clang e wasm-ld em WebAssembly, sem servidor, sem enviar código para fora).
  Os 22 exemplos de `docs/exemplos/` estão no seletor, com a entrada de teste
  dourada; entrada interativa por JSPI quando o navegador tem
  (`WebAssembly.Suspending`).
- Depurador opcional no playground, **desligado por padrão** (interruptor na
  barra lateral, ou `?debug=1`). Ligado, o Run vira Debug: o programa para na
  primeira linha, anda de passo em passo, respeita pontos de parada marcados no
  número da linha e mostra as variáveis visíveis com o tipo do BASICGOL. Não
  usa servidor nem DWARF: `playground/ativo/depurador.js` instrumenta a fonte
  antes de compilar e `playground/tools/basicgol_depurador.h` conversa com a
  página pelo descritor 3 (a mesma pausa do JSPI). A numeração das linhas não
  muda, a saída do programa não muda, e fora da página o descritor 3 responde
  `EBADF` — o `.wasm` roda igual em qualquer runtime WASI.
- Verificação em três níveis, todos contra `docs/exemplos/*.esperado.txt`:
  `playground/tools/verificar.js` (toolchain, memfs reimplementado),
  `playground/tools/verificar-navegador.mjs` (o código da página importado como
  módulo ES) e `playground/tools/verificar-pagina.mjs` (a página num Chromium
  headless, via CDP). `playground/tools/verificar-depurador.mjs` roda os 21
  exemplos de novo com o depurador ligado e exige saída byte a byte idêntica.
- `playground/README.md` e `playground/ATRIBUICOES.md` (licenças de wasm-clang/
  LLVM, wasi-libc, Ace, xterm.js, Font Awesome, Montserrat e Ubuntu Mono).

### Changed
- `README.md`: enxugado para o essencial (logo centralizada, início rápido, os dois
  caminhos M0/M1, estado de compatibilidade, estrutura do repositório, documentação,
  testes e as limitações do dia a dia). Relatórios de fase, detalhes por compilador e
  instruções de MSVC/MinGW continuam nos documentos de `docs/`.
- `docs/logo.png` (e `website/static/img/logo.png`, mesma arte) para a logo do README.
- Gerador do site: o bloco centralizado do README (`<h1 align="center">`, `<p
  align="center">`) deixa de ser escapado, tem as linhas reunidas e as URLs relativas
  resolvidas para o commit; imagens públicas e externas viram links crus.

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
