# Suíte de testes — BASICGOL

```sh
make test         # tudo
make test-v       # tudo, listando cada verificação aprovada
./tests/run.sh 03 # só uma suíte (prefixo do nome)
```

A suíte de linguagem usa `bash`, um compilador C11, `build/basicgolc` e
`build/libbasicgol.a` (construídos por `make`). Os testes unitários da biblioteca
são `tests/test_biblioteca.c`, executados por `tests/run_biblioteca.sh`. O conjunto
completo também precisa de `ar`, `mktemp`, `diff` e Python 3 para a cobertura. Os
runners criam diretórios temporários e os apagam ao terminar.

```sh
make test-biblioteca  # somente biblioteca modular
make test             # linguagem M0/M1 + biblioteca
```

## A ideia central: um caso, dois caminhos

Todo caso de **execução** roda duas vezes — caminho **M0** (as macros de
`include/basicgol.h`) e caminho **M1** (`build/basicgolc` traduz e compila o C emitido) —
e só passa se **os dois** produzirem exatamente a saída dourada. A equivalência M0 × M1
(AC-32) não é um teste solto: ela é uma propriedade de cada um dos casos.

## Layout

```
tests/
├── run.sh        # as suítes 01…11 (cada uma é um bloco if; 09_tipos_expr é
│                 # dirigida pela árvore de casos, como 02…08)
├── lib.sh        # compila_m0, compila_m1, compila_objetos, roda, difere, caso_*
├── expansoes.txt # tabela de ouro das expansões (AC-22), comparada com `cc -dM -E`
├── COBERTURA.md  # matriz de rastreabilidade (gerada por `make cobertura`)
└── casos/
    ├── 02_basico/           tipos, operadores, texto, utilidades
    ├── 03_controle/         SE/ENQUANTO/REPITA/PARA/ESCOLHA e desvios
    ├── 04_estruturas/       VETOR, REGISTRO, ENUMERACAO, funções, recursão
    ├── 05_entrada_saida/    ESCREVA*, LEIA*, ERRO/AFIRME, truncamento
    ├── 06_negativos/        erros do compilador C (esperado_erro.txt)
    ├── 07_multiarquivo/     módulos, EXTERNO/ESTATICO, C puro junto de BG
    ├── 08_aplicacao/        sistema de acervo (3 módulos + cabeçalho)
    ├── 09_tipos_expr/       tipos, conversões, precedência, divisão verificada,
    │                        texto seguro, entrada numérica, extensões opcionais
    ├── 12_negativos_m1/     diagnósticos do transpilador (esperado_erro.txt)
    └── 13_avisos/           um caso por aviso W-01…W-07
```

## Convenção de arquivos de um caso

| Arquivo | Papel |
|---|---|
| `programa.c`, `*.c`, `*.h` | fontes do caso — a fonte BG é `.c` (todos compilados juntos) |
| `entrada.txt` | o que vai para o `stdin` (ausente = `/dev/null`) |
| `esperado.txt` | saída esperada em `stdout` (obrigatório em caso de execução) |
| `esperado_rc.txt` | código de saída esperado (padrão `0`) |
| `esperado_stderr.txt` | trecho que precisa aparecer em `stderr` |
| `esperado_erro.txt` | mensagens de erro **aceitas** (casos negativos) — ver abaixo |
| `cflags.txt` | flags extras só deste caso (ex.: `-Wno-implicit-fallthrough`) |
| `objetos` | marcador: compilar cada fonte com `-c` e ligar depois (multi-arquivo real) |
| `sem_m0` / `sem_m1` | marcador: pular um dos caminhos |

Um caso só precisa declarar o que é diferente do padrão — a ausência de `entrada.txt`,
por exemplo, significa "sem entrada".

## Casos negativos e o texto do compilador

`esperado_erro.txt` lista, **uma por linha**, as mensagens aceitas para o caso: linha vazia é
ignorada e linha que começa com `#` é comentário. O caso passa se o programa **não** compilar
(e, no caminho M1, se o `basicgolc` recusar) e se **pelo menos uma** das linhas aparecer no
diagnóstico — comparação de texto fixo, sem expressão regular.

Isso é de propósito. O texto das mensagens do caminho M0 é do **compilador**, não da
linguagem: GCC e Clang redigem o mesmo erro de formas diferentes, e o próprio GCC muda a
redação entre versões. Por isso cada caso guarda as variantes (normalmente uma do GCC e uma
do Clang) em vez de exigir uma frase única:

```
# GCC: "expected expression before '=' token"; Clang: "expected expression".
expected expression
```

Duas salvaguardas completam isso, para que a suíte não dependa da máquina:

* `tests/run.sh` fixa `LC_ALL=C` — com locale UTF-8 o GCC troca `'` por `‘`, e um locale
  traduzido mudaria a mensagem inteira. A biblioteca em si nunca chama `setlocale` (D-17).
* `tests/lib.sh` normaliza as aspas tipográficas (`‘ ’ “ ”` → `' "`) antes de comparar, então
  a comparação também vale quando alguém roda o runner fora do CI com outro locale.
* Os runners comparam a saída de console sem `\r`: o CRT do Windows pode traduzir
  `\n` para `\r\n`, enquanto os arquivos de saída esperada permanecem com LF.

Os diagnósticos do **basicgolc** (`12_negativos_m1`) usam o mesmo formato — ali o texto é
nosso e vem da especificação (§4.3), então basta uma linha por caso.

## Como adicionar um caso

1. Crie `tests/casos/<suíte>/<nome>/programa.c`.
2. Rode o programa uma vez e confira **à mão** a saída (nunca aceite a saída só porque o
   programa a produziu: o teste existe para pegar erro, não para congelar erro).
3. Salve como `esperado.txt` e rode `./tests/run.sh <suíte>`.
4. Rode `make cobertura` para atualizar a matriz — ela lista o caso automaticamente.

Se o caso documenta uma decisão da especificação, cite o `C-xx`/`D-xx` no comentário do
fonte: quem lê o teste depois entende **por que** ele existe.
