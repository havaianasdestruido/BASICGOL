# Atribuições — playground

O playground é uma página estática que **não carrega nada de fora**: todas as
ferramentas e todos os recursos de terceiros estão no repositório (ou são
baixados por um script e empacotados localmente). Esta página lista cada um
deles, de onde veio, sob que licença e onde está a licença completa.

Resumo por licença:

| Recurso | Versão/pacote | Licença | Texto da licença |
|---|---|---|---|
| clang (WebAssembly) | binji/wasm-clang, clang 8.0.1 | Apache-2.0 com exceção do LLVM | `playground/vendor/wasm-clang/LICENSE.llvm` |
| lld / wasm-ld | binji/wasm-clang (mesmo pacote) | Apache-2.0 com exceção do LLVM | idem |
| wasi-libc + libc++ | sysroot do binji/wasm-clang | Apache-2.0 com exceção do LLVM (musl: MIT) | idem |
| memfs (WASI em memória) | binji/wasm-clang | Apache-2.0 | `playground/vendor/wasm-clang/LICENSE` |
| Ace | 1.4.x | BSD-3-Clause | `playground/vendor/ace/LICENSE` |
| xterm.js + addon-fit | 5.5.0 | MIT | `playground/vendor/xterm/LICENSE` |
| Font Awesome Free | 5.6.3 (só os `path` dos ícones usados) | CC BY 4.0 (ícones), SIL OFL 1.1 (fontes), MIT (código) | `playground/vendor/fa/LICENSE.txt` |
| Montserrat | `@fontsource/montserrat` (400, 700, latin) | SIL Open Font License 1.1 | `playground/vendor/fonts/montserrat/LICENSE` |
| Ubuntu Mono | `@fontsource/ubuntu-mono` (400, 700, latin) | Ubuntu Font Licence 1.0 | `playground/vendor/fonts/ubuntu-mono/LICENSE` |

Os fontes do BASICGOL (cabeçalhos, `src/biblioteca/*.c`) e o próprio
playground são do projeto BASICGOL e seguem a licença do repositório (veja
`LICENSE` na raiz) — os recursos de terceiros abaixo continuam sob as licenças
deles.

---

## 1. clang, lld e o sysroot WASI (binji/wasm-clang)

`playground/tools/baixar-toolchain.sh` baixa
<https://github.com/binji/wasm-clang> (`master`, tarball do GitHub), de onde saem:

- `clang` e `lld` compilados para WebAssembly com WASI (`clang`, `lld`);
- `memfs/` — a implementação de sistema de arquivos em memória usada pelos dois;
- `sysroot.tar` — libc (wasi-libc/musl), libc++ e `crt1.o` em `usr/`;
- `lib/wasi/libclang_rt.builtins-wasm32.a` — builtins do compilador.

O pacote é montado por `playground/tools/empacotar-toolchain.js` em
`playground/publico/toolchain/wasi-cc.tar.gz` (com o `shim.c` do projeto e os
cabeçalhos do BASICGOL acrescentados). O diretório `.toolchain-build/` e o
tarball gerado não são versionados: rode os dois scripts para reconstruí-los.

As licenças ficam versionadas em `playground/vendor/wasm-clang/` (copiadas de
`.toolchain-build/`; o diretório baixado em si não é versionado).

Licenças: o projeto wasm-clang é Apache-2.0 (`LICENSE`); o LLVM/clang/lld é
Apache-2.0 com a exceção do LLVM (`LICENSE.llvm`); a wasi-libc distribui
musl (MIT), libc++ (Apache-2.0 com exceção do LLVM) e `libclang_rt`
(Apache-2.0 com exceção do LLVM). Nada disso foi modificado — o `shim.c`, o
`basicgol_playground.h` e os cabeçalhos do BASICGOL são acrescentados por fora,
dentro do tarball.

Se o binário do `clang` for redistribuído além do repositório, mantenha junto
`LICENSE` e `LICENSE.llvm`.

## 2. Ace (editor)

`playground/vendor/ace/` guarda `ace.js`, `mode-c_cpp.js` e
`theme-textmate.js` da versão 1.4.x (BSD-3-Clause). O tema visual
TextMate do cplayground foi refeito em CSS (`ativo/tema.css`, regras
`.ace-tm`), a partir das folhas de estilo do cplayground.

## 3. xterm.js (terminal)

`playground/vendor/xterm/` guarda `xterm.js`, `addon-fit.js` e `xterm.css` do
`@xterm/xterm` 5.5.0 e `@xterm/addon-fit` 0.10.0 (MIT). O tema de cores do
terminal vem do cplayground (`getTerminalColors()`).

## 4. Font Awesome Free (ícones)

A interface usa 23 ícones (`ativo/app.js`). Em vez de carregar o Font Awesome
inteiro, `playground/tools/extrair-icones.js` extrai o `viewBox` e o `path` de
cada um de `svg/solid/*.svg` do Font Awesome Free 5.6.3 para
`playground/vendor/fa/icones.json`. Os ícones são CC BY 4.0 (atribuição:
Font Awesome Free, <https://fontawesome.com>); a licença completa, com as
partes MIT e OFL, está em `playground/vendor/fa/LICENSE.txt`.

## 5. Fontes

- **Montserrat** (interface): `@fontsource/montserrat`, arquivos `latin`
  400/700 em `woff2`, SIL OFL 1.1.
- **Ubuntu Mono** (terminal): `@fontsource/ubuntu-mono`, `latin`
  400/700 (normal e itálico), Ubuntu Font Licence 1.0.

Ambas são carregadas por `@font-face` em `ativo/tema.css` e não vão à rede.

## 6. Inspiração: cplayground

A aparência e o comportamento da página seguem o **cplayground**, de Ryan
Eberhardt (<https://github.com/reberhardt7/cplayground>, MIT): topbar com
`settings` e `Run`, `sidebar` com linguagem/compilador/flags/arquivos de apoio,
painel de código e painel de terminal. Os nomes de classe (`topbar`,
`sidebar`, `code-container`, `program-pane`, `btn`/`main-text`/`shortcut`,
`show-split`/`show-code-only`/`show-term-only`) foram mantidos iguais de
propósito, para o CSS ser reconhecível para quem conhece o original.

O que é do BASICGOL e não existe no cplayground: os 22 exemplos prontos
(`docs/exemplos/`), o seletor de exemplos com a entrada de teste dourada, os
temas claro/escuro e o selo de status da toolchain.

---

## Como reconstruir os recursos de terceiros

```sh
# clang + lld + sysroot (baixa do GitHub, ~18 MB)
sh playground/tools/baixar-toolchain.sh
# empacota o tarball que a página carrega
node playground/tools/empacotar-toolchain.js
# (as licenças do wasm-clang já estão em playground/vendor/wasm-clang/)
# reconstrói o JSON de ícones a partir de um Font Awesome Free descompactado
node playground/tools/extrair-icones.js caminho/para/fontawesome-free
```

Ace, xterm.js e as fontes são estáticos e já estão versionados em
`playground/vendor/`.
