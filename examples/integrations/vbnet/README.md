# BASICGOL + VB.NET (7 exemplos)

Os exemplos VB.NET usam P/Invoke (`DllImport`) para uma biblioteca nativa feita com fontes BASICGOL compilados em C11. `native/basicgol.c` define as sete funções C; `examples/ExampleNN.vb` exercita cada chamada.

## Compilar a biblioteca nativa

Execute a partir desta pasta (`examples/integrations/vbnet`).

Linux (GCC/Clang):

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fPIC -shared \
   -I ../../../include native/basicgol.c -o libbasicgol.so
```

macOS (Clang):

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -dynamiclib \
   -I ../../../include native/basicgol.c -o libbasicgol.dylib
```

Windows (Developer Command Prompt do Visual Studio):

```bat
cl /nologo /std:c11 /W4 /utf-8 /LD /I ..\..\..\include native\basicgol.c /link /OUT:basicgol.dll
```

O projeto copia a biblioteca ao diretório de saída do .NET. Use o .NET 8 SDK (ou um SDK mais recente com o targeting pack .NET 8):

```sh
dotnet run -- 01
dotnet run -- 07
```

## Exemplos

| ID | Interoperabilidade demonstrada |
|---|---|
| `01` | Inteiros e convenção de chamada C (`Cdecl`). |
| `02` | Retorno C `int64_t` mapeado para `Long` VB.NET. |
| `03` | Argumentos e retorno `Double`. |
| `04` | Array VB.NET passado como ponteiro C com tamanho explícito. |
| `05` | String VB.NET enviada como UTF-8; BASICGOL conta bytes. |
| `06` | `const char *` estático UTF-8 convertido para string. |
| `07` | Delegate VB.NET chamado de volta pelo lado BASICGOL. |

Mantenha a fronteira ABI simples: a chamada de callback é síncrona, e exceções/objetos gerenciados não devem atravessar a fronteira nativa.
