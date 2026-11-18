# Exemplos de interoperabilidade BASICGOL

Foram adicionados **7 exemplos por combinação** (35 no total), dentro do intervalo solicitado. Cada pasta explica a ABI e como compilar os exemplos.

| Combinação | Exemplos | Como funciona | Guia |
|---|---:|---|---|
| BASICGOL + C++ | 7 | Fontes BASICGOL compilados como C11; o C++ chama e, em alguns casos, fornece funções por ABI C. | [cpp/README.md](cpp/README.md) |
| BASICGOL + C# | 7 | Biblioteca nativa C11 consumida por P/Invoke (`DllImport`). | [csharp/README.md](csharp/README.md) |
| BASICGOL + Assembly | 7 | Programa BASICGOL chama funções Assembly GNU pela ABI C. | [assembly/README.md](assembly/README.md) |
| BASICGOL + VB.NET | 7 | Biblioteca nativa C11 consumida por P/Invoke (`DllImport`). | [vbnet/README.md](vbnet/README.md) |
| BASICGOL + JavaScript (Emscripten) | 7 | Funções BASICGOL/C11 compiladas para WebAssembly e chamadas pelo JavaScript no navegador. | [emscripten/README.md](emscripten/README.md) |

## Escolha a fronteira correta

BASICGOL é uma camada de macros sobre C11, não um runtime que permita incluir o cabeçalho da linguagem em qualquer compilador.

- **C++:** `include/basicgol.h` continua sendo somente C. Compile cada `.c` BASICGOL com um compilador C11 e exponha uma API C declarada com `extern "C"` no lado C++.
- **C# e VB.NET:** compile o C11/BASICGOL em uma biblioteca compartilhada (`.so`, `.dylib` ou `.dll`) e use P/Invoke com tipos ABI simples. Não passe exceções nem objetos gerenciados pela fronteira nativa.
- **Assembly:** as rotinas são exemplos de chamada C e dependem da plataforma/ABI indicada no guia Assembly.
- **JavaScript:** Emscripten compila os fontes C11/BASICGOL para WebAssembly; o host JavaScript chama os símbolos exportados.

Os diretórios de saída ficam sob `build/` (ou `emscripten/build/`) e não são versionados. C++ e Assembly têm alvos `make test`; C# e VB.NET requerem um SDK .NET e uma biblioteca nativa; Emscripten requer `emcc` e um servidor HTTP local.
