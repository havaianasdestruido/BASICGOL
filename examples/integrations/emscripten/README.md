# BASICGOL + JavaScript + Emscripten (7 exemplos)

Cada arquivo `basicgol.c` define uma função BASICGOL exportada para WebAssembly. Emscripten gera um módulo ES (`.js` + `.wasm`); `host.js` importa o módulo escolhido e chama a função exportada. Os sete casos usam a ABI C de valores numéricos simples.

## Requisitos e execução

1. Instale/ative o Emscripten SDK (`emcc`) e tenha Python 3.
2. Na raiz do repositório, compile os sete módulos:

   ```sh
   ./examples/integrations/emscripten/build.sh
   ```

3. Sirva esta pasta por HTTP (WebAssembly não deve ser aberto via `file://`):

   ```sh
   cd examples/integrations/emscripten
   python3 -m http.server 8000
   ```

4. Abra `http://localhost:8000` e selecione um exemplo. Também é possível usar `emrun index.html`.

O `build.sh` guarda os artefatos gerados em `build/`, que não fazem parte dos fontes versionados. O script usa `--no-entry` porque os exemplos exportam funções C e não precisam de uma função `main`.

## Exemplos

| Pasta | Função chamada pelo JavaScript |
|---|---|
| `01_soma` | `bg_soma(int, int)` |
| `02_fatorial` | `bg_fatorial(int)` |
| `03_media` | `bg_media3(double, double, double)` |
| `04_maior` | `bg_maior(int, int)` |
| `05_primo` | `bg_e_primo(int)`; inteiro `1`/`0` como resultado C |
| `06_fahrenheit` | `bg_fahrenheit_para_celsius(double)` |
| `07_limita` | `bg_limita(int, int, int)` |

O binding JavaScript usa `Module._nome_da_funcao`; por isso, o nome C aparece em `-sEXPORTED_FUNCTIONS` no build. Ao adicionar uma função, exporte-a explicitamente e documente os tipos e a propriedade dos dados na ABI C.
