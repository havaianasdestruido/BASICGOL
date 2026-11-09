# Matriz de funcionalidades — BASICGOL (BG)

| Categoria | Recurso | Status |
|---|---|---|
| Sintaxe | Palavras-chave em português (`SE`, `ENTAO`, `ENQUANTO`, `ESCREVAL`, `REGISTRO`, ...) | Disponível |
| Execução | Modelo idêntico ao de C (sem VM, GC, interpretador) | Disponível |
| Caminho M0 | Macros do cabeçalho `basicgol.h` | Disponível |
| Caminho M1 | Transpilador `basicgolc` (açúcar `PARA ... FACA`) | Disponível |
| Tipos | `INTEIRO`, `REAL`, `CARACTERE`, `LOGICO`, `CADEIA`, `VAZIO`, variantes de largura | Disponível |
| Controle de fluxo | `SE/SENAO/SENAO_SE`, `ENQUANTO`, `REPITA`, `PARA`, `ESCOLHA`, `PARE`, `PROSSIGA` | Disponível |
| Estruturas | `VETOR`, `REGISTRO`, `ENUMERACAO`, funções, recursão | Disponível |
| E/S | `ESCREVA*`, `LEIA*`, `ERRO`, `AFIRME` | Disponível |
| Multi-arquivo | `EXTERNO`/`ESTATICO`, módulos, C puro junto de BG | Disponível |
| Biblioteca padrão | caracteres, console, matemática, texto, utilidades | Disponível |
| Interop C | Qualquer código C pode ser chamado/definido | Disponível |
| IDE/syntax highlighting | Opcional, não incluído por padrão | Opcional |

Consulte `docs/02-referencia.md` para a referência completa de palavras-chave
e `docs/11-matriz-compatibilidade.md` para a matriz de compiladores.
