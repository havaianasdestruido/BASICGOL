# BASICGOL + C++ (7 exemplos)

O cabeçalho da linguagem `basicgol.h` é C11 e **não deve ser incluído em `.cpp`**. Cada `basicgol.c` abaixo é compilado como C11; o executável C++ chama as funções por uma API C com `extern "C"`. Os cabeçalhos modulares `include/basicgol/*.h` também podem ser usados diretamente de C++17, mas não são necessários nestes exemplos.

## Compilar e executar

Na raiz do repositório:

```sh
make -C examples/integrations/cpp test
```

O Makefile compila os executáveis em `build/integrations/cpp/`; `CC` e `CXX` podem ser sobrescritos. Cada pasta contém `basicgol.c`, `api.h` e `main.cpp`.

## Exemplos

| Exemplo | O que demonstra |
|---|---|
| `01_soma` | C++ chama uma função BASICGOL de inteiros. |
| `02_pontos` | Estrutura C compartilhada por ponteiro e cálculo BASICGOL. |
| `03_vetor` | `std::array` passa buffer e tamanho para uma função BASICGOL. |
| `04_callback` | BASICGOL invoca callback C++ com ligação C. |
| `05_cpp_chama_bg` | Função BASICGOL chama uma implementação C++ (`std::clamp`) via ABI C. |
| `06_texto_utf8` | `std::string` passa texto UTF-8; BASICGOL mede bytes, não caracteres. |
| `07_raii` | C++ administra com `std::unique_ptr` um buffer alocado/liberado do lado BASICGOL. |

O alvo `test` compila e executa os sete programas.
