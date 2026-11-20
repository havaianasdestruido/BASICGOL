# Playground BASICGOL — compila e roda no navegador

Uma página estática onde você escreve BASICGOL e aperta **Run**: não há
servidor. O clang e o wasm-ld compilados para WebAssembly (do
[binji/wasm-clang](https://github.com/binji/wasm-clang)) rodam na própria aba,
num sistema de arquivos em memória, e o programa que sai de lá roda no mesmo
lugar. Nada do seu código é enviado a lugar nenhum.

```
playground/
├── ativo/                     a página (é isso que se publica)
│   ├── index.html             topbar, sidebar, editor, terminal
│   ├── tema.css               aparência (o tema do cplayground, em CSS vars)
│   ├── app.js                 amarração: botões, exemplos, tema, execução
│   ├── ferramentas.js         clang/lld no navegador (memfs + WASI + shim)
│   ├── armazenamento.js       o programa WASI: argv, env, fd_read/write, memfs
│   ├── terminal.js            xterm.js: entrada, roteiro, cores
│   └── exemplos.js            os 22 exemplos (gerado de docs/exemplos/)
├── tools/                     scripts de manutenção e verificação
├── vendor/                    Ace, xterm.js, fontes e ícones (local)
├── publico/toolchain/         wasi-cc.tar.gz (gerado; não versionado)
└── ATRIBUICOES.md             licenças de tudo que é de terceiros
```

## Rodar

```sh
# 1) baixa o clang/lld em WebAssembly e monta o pacote (~18 MB baixados)
sh playground/tools/baixar-toolchain.sh
node playground/tools/empacotar-toolchain.js

# 2) serve a pasta (módulos ES não funcionam em file://)
python3 -m http.server 8000 --bind 0.0.0.0
# abra http://localhost:8000/playground/ativo/index.html
```

A primeira carga leva alguns segundos (baixar e desempacotar o pacote); depois
disso, cada Run custa ~2 s para compilar e ligar, e o programa começa na hora.

## Verificação

Os três níveis rodam a mesma coisa de pontos de vista diferentes, e todos
comparam com `docs/exemplos/*.esperado.txt`:

```sh
# a biblioteca e o transpilador da raiz (GCC/Clang nativos)
make test

# a toolchain do playground, com um memfs reimplementado em CommonJS
node playground/tools/verificar.js --dourado

# o código da página, importado como módulo ES (o mesmo que a aba carrega)
node playground/tools/verificar-navegador.mjs --dourado

# a página de verdade, num Chromium headless (precisa de um Chromium)
BASICGOL_CHROMIUM=/usr/bin/chromium node playground/tools/verificar-pagina.mjs
```

`verificar-pagina.mjs` é o teste de fumaça do que só a página prova: que o
terminal mostra a saída, que a entrada interativa espera o Enter (JSPI), que o
seletor de exemplos carrega a entrada de teste dourada e que não há erro de
console. Ele mesmo serve a pasta, então não precisa de servidor à mão.

Resultado atual: **22/22 exemplos idênticos** ao dourado nos três níveis, e as
sete verificações da página passando (compilação ~1,9 s).

## O que roda no navegador

O pacote `wasi-cc.tar.gz` tem:

| Peça | O que é |
|---|---|
| `clang`, `lld` | LLVM 8.0.1 compilado para wasm32-wasi |
| `memfs` | implementação WASI de sistema de arquivos em memória |
| `sysroot.tar` | libc (musl/wasi-libc), libc++ e `crt1.o` |
| `lib/wasi/libclang_rt.builtins-wasm32.a` | builtins |
| `include/`, `src/` | os cabeçalhos e a biblioteca do BASICGOL |
| `src/shim.c` | conserta o que a wasi-libc de 2019 não tem (ver abaixo) |

`ferramentas.js` monta o disco, roda `clang -cc1 -emit-obj` para cada arquivo
(`main.cc`, `shim.c`, `src/biblioteca/*.c`) e liga com `wasm-ld`, produzindo um
`programa.wasm` de ~86 KB. `armazenamento.js` executa esse wasm com os imports
`wasi_unstable`/`wasi_snapshot_preview1` apontando para o terminal.

### O shim (`tools/shim.c`)

A wasi-libc dessa toolchain é de 2019: não tem `mkstemp`, e o `printf` dela não
formata `long double`. Como `src/biblioteca/utilidades.c` usa as duas coisas, o
`shim.c` é compilado junto com a biblioteca e:

- responde `ENOSYS` para `mkstemp` (o resto do módulo continua funcionando);
- reescreve a família não-`v` de `printf`/`sprintf`/`snprintf`/`fprintf`: o
  `long double` (`BG_REAL_LONGO`) é passado ao `vsnprintf` da libc como
  `double`, e `%zu`/`%zd` viram `%u`/`%d` com um `size_t`/`ptrdiff_t` explícito
  (no wasm32, `size_t` e `long` são 32 bits; só `j` é 64).

As funções `v*` **não** são substituídas: o formatador do shim chama o
`vsnprintf` da libc, então substituí-lo seria recursão infinita.

### Entrada do usuário

`LEIA_TEXTO`/`LEIA_INTEIRO` chamam `fd_read` no stdin. Onde o navegador tem
**JSPI** (`WebAssembly.Suspending`/`promising`, Chrome 137+), o import é
embrulhado e o programa realmente **espera o Enter**, retomando na mesma
instrução. Onde não tem, a entrada é digitada antes do Run e lida como um
fluxo — a página avisa no terminal qual dos dois modos está valendo.

## Aparência

A página segue o [cplayground](https://github.com/reberhardt7/cplayground) de
Ryan Eberhardt (MIT): os mesmos nomes de classe (`topbar`, `sidebar`,
`code-container`, `program-pane`, `btn`/`main-text`/`shortcut`,
`show-split`/`show-code-only`/`show-term-only`), a mesma topbar (engrenagem +
Run) e o mesmo terminal (Ubuntu Mono 16px). O tema do Ace (TextMate) foi
portado para CSS em `tema.css`, com as cores em variáveis — daí os temas
Padrão, Monokai, Zenburn, Nord e Dracula. A primeira visita usa Padrão; a escolha
no seletor fica salva em um cookie por um ano. `?theme=nord`, por exemplo,
substitui a preferência só para aquela página.

O que é do BASICGOL: os 22 exemplos de `docs/exemplos/` no seletor da sidebar,
com a entrada de teste dourada já no terminal, e as dicas de erro (por
exemplo, lembrar que `PARA i DE 1 ATE 10` é do caminho M1).

## Manutenção

Depois de mexer em `include/`, `src/` ou no `shim.c`, repita o passo 1 acima
(empacotar) e rode as verificações. `exemplos.js` é gerado:

```sh
node playground/tools/gerar-exemplos.js     # docs/exemplos/*.c → ativo/exemplos.js
node playground/tools/extrair-icones.js DIR # Font Awesome Free → vendor/fa/icones.json
```

Licenças de tudo que é de terceiros: [ATRIBUICOES.md](ATRIBUICOES.md).
