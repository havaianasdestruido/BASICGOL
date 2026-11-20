# Changelog — BASICGOL

## Não publicado

### Added
- Workspace falsa com persistência (`basicgol/workspace.h` + `src/workspace/workspace.c`),
  módulo **opt-in e desligado por padrão** (só existe para quem define
  `BG_USAR_WORKSPACE` antes do `#include`). É um disco simulado em memória — pastas e
  arquivos — que grava um snapshot único (`BGWS1`, conteúdo em base64) de forma atômica e
  o recarrega depois. Dá para **enviar** arquivos do disco real ou de um buffer e
  **baixar** de volta, além de consultar, listar e remover. Limites: 4096 arquivos e
  64 MiB. Suíte própria (`tests/test_workspace.c`, `make test-workspace`, 130 checks)
  e exemplo `docs/exemplos/biblioteca_workspace.c`.
- `basicgol/filesystem.h` passa a integrar a lista de cabeçalhos autocontidos da
  suíte de portabilidade (com `basicgol/workspace.h`).
- Biblioteca 1.1.0 (era 1.0.0): acréscimo compatível.
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

### Changed
- `docs/09-biblioteca-padrao.md`: novo tópico 3.6 (workspace falsa), tabela de módulos
  com `filesystem` e o agregado, e o §4 documenta o opt-in ao lado de
  `BG_USAR_EXTENSOES`.
- `tests/portabilidade/cpp_inclusao.cpp` também exercita a workspace, provando que o
  cabeçalho de módulo continua usável de C++.
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
