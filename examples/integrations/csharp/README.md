# BASICGOL + C# (7 exemplos)

Os sete exemplos usam P/Invoke (`DllImport`) para chamar funções BASICGOL compiladas como biblioteca nativa C. O código BASICGOL fica em `native/basicgol.c`; os exemplos C# ficam em `examples/ExampleNN.cs` e são selecionados pelo argumento `01` a `07`.

## Compilar a biblioteca nativa

Execute a partir desta pasta (`examples/integrations/csharp`).

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

O projeto copia a biblioteca ao diretório de saída do .NET durante o build. Use o .NET 8 SDK (ou um SDK mais recente com o targeting pack .NET 8):

```sh
dotnet run -- 01
dotnet run -- 07
```

## Exemplos

| ID | Interoperabilidade demonstrada |
|---|---|
| `01` | Inteiros e `CallingConvention.Cdecl`. |
| `02` | Retorno C `int64_t` para `long` C#. |
| `03` | Argumentos e retorno `double`. |
| `04` | Array gerenciado como ponteiro C + tamanho. |
| `05` | String C# enviada como UTF-8; BASICGOL conta bytes. |
| `06` | `const char *` estático UTF-8 convertido de volta para string. |
| `07` | Delegate C# passado como callback C; delegate mantido vivo durante a chamada. |

A biblioteca usa uma ABI C simples: não passe exceções .NET para C nem atravesse a fronteira nativa com exceções/objetos gerenciados. A função de callback é síncrona.
