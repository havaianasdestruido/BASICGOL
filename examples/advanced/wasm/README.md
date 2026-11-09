# BASICGOL + WebAssembly

Exemplo de compilação de um programa BASICGOL para WebAssembly usando Emscripten.

## Pré-requisitos
- Emscripten SDK (emcc) instalado e ativado.
- make

## Build

```sh
emcc -std=c11 -I ../../../include wasm_exemplo.c -o wasm_exemplo.html
# ou
emcc -std=c11 -I ../../../include wasm_exemplo.c -o wasm_exemplo.js
```

## Executar

```sh
emrun wasm_exemplo.html
# ou sirva os arquivos com um servidor HTTP local
```

## Notas
- A biblioteca padrão (libbasicgol) deve ser compilada para WASM separadamente
  se for utilizada (emcc src/biblioteca/*.c -I ../../../include).
- Funcionalidades dependentes de sistema (arquivos, rede) exigem APIs do Emscripten
  (FS, sockets) e não estão cobertas neste exemplo.
