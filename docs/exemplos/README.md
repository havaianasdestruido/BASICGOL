# Exemplos BASICGOL

Vinte e quatro programas completos, escritos com construtores definidos na
[especificação](../01-especificacao.md), na
[referência de tipos e expressões](../06-tipos-e-expressoes.md) e na
[biblioteca padrão](../09-biblioteca-padrao.md). Os cinco exemplos `biblioteca_*.c`
demonstram, um por arquivo, os módulos de console, matemática, texto, caractere e
utilidades. Os programas servem a três propósitos:

1. **Documentação executável** — cada exemplo demonstra um conjunto de construtores.
2. **Especificação por exemplos** — se um exemplo não compilar, a especificação está
   errada ou incompleta.
3. **Testes dourados** — a saída esperada de cada programa está registrada abaixo e é a
   base dos testes `T-execução` da Fase 2 ([critérios](../05-criterios-de-aceitacao.md)).

## Como compilar

Desde a Fase 5, a sintaxe e os tipos continuam no cabeçalho
[`include/basicgol.h`](../../include/basicgol.h), e os helpers estão organizados em
[`include/basicgol/`](../../include/basicgol/) com implementação em
[`src/biblioteca/`](../../src/biblioteca/). Todos os exemplos são compilados e
executados pela suíte M0/M1 (`make test`, suíte `10_exemplos`).

Perfil M0:

```sh
make biblioteca
cc -std=c11 -Wall -Wextra -Wformat=2 -I ../../include ola_mundo.c \
   ../../build/libbasicgol.a -lm -o ola_mundo
```

Com o transpilador (perfil M1):

```sh
make
build/basicgolc para_contado.c -o para_contado.gerado.c
cc -std=c11 -Wall -Wextra -Wformat=2 -I ../../include para_contado.gerado.c \
   ../../build/libbasicgol.a -lm -o para_contado
```

### Arquivos de ouro ao lado dos fontes

Cada exemplo tem, ao lado do `.c`, os arquivos que a suíte consome:

| Sufixo | Papel |
|---|---|
| `.entrada.txt` | o que vai para o `stdin` (ausente = sem entrada) |
| `.esperado.txt` | a saída **completa** esperada em `stdout` |
| `.contem.txt` | linhas que precisam aparecer em `stdout` (para saída que depende do relógio) |
| `.esperado_stderr.txt` | linha que precisa aparecer em `stderr` |

Eles são a fonte de verdade da máquina; a seção "Saídas esperadas" abaixo continua sendo a
leitura humana, e pode mostrar trechos em vez do arquivo inteiro.

## Registro de validação (Fases 1–5)

Compilados com GCC 12.2, `-std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror`: **zero avisos**.

| Arquivo | Perfil | Construtores demonstrados | Compila | Executa |
|---|---|---|---|---|
| `ola_mundo.c` | M0 | `INICIO`, `ESCREVAL`, `FIM` | ok | ok |
| `media_notas.c` | M0 | `FUNCAO`, `TEXTO` como retorno, `SE`/`SENAO_SE`/`SENAO`, `LEIA_OK`, `ERRO`, `ESCREVA_FMT` | ok | ok |
| `tabuada.c` | M0 | `PARA`, `PARA_PASSO`, laços aninhados, `SE` com `OU`, `ESCREVA_FMT` | ok | ok |
| `primos.c` | M0 | `FUNCAO LOGICO`, `ENQUANTO`, `PARE`, `PROSSIGA`, `MOD`, retorno antecipado | ok | ok |
| `vetores.c` | M0 | `VETOR`, `TAMANHO_VETOR`, `INTEIRO v[]` + tamanho, `PROCEDIMENTO` | ok | ok |
| `cadastro.c` | M0 | `REGISTRO`/`FIM_REGISTRO`, vetor de registros, `TEXTO_FIXO`, `LEIA` + `LEIA_TEXTO` (C-20), `ESCREVA_FMT` com `%-20s` | ok | ok |
| `adivinha.c` | M0 | `REPITA`/`ATE_QUE`, `SORTEIA`, `INICIA_SORTEIO`, `ESCOLHA`/`CASO` agrupados | ok | ok |
| `contador_vogais.c` | M0 | `TEXTO_FIXO` em parâmetro como `CARACTERE destino[]`, `TAMANHO_TEXTO`, percurso de texto (entrada limpa) | ok | ok |
| `interop_c.c` | M0 + C | `#include <time.h>`, `struct`, função estática em C, ponteiro, `for`/`printf` misturados | ok | ok |
| `fatorial_recursivo.c` | M0 | recursão, `INTEIRO64`, `ESCREVA_FMT("%lld")` | ok | ok |
| `utilidades.c` | M0 | catálogo de tipos, `ABS`/`MINIMO`/`MAXIMO`/`POTENCIA`/`RAIZ`/`ARREDONDA`/`PISO`/`TETO`, `COPIA_TEXTO`/`JUNTA_TEXTO`, `TEXTO_PARA_INTEIRO`/`TEXTO_PARA_REAL`, `ENUMERACAO`, `AFIRME`, `ESCREVA_ERRO`, `ESTATICO`, `VAZIO` | ok | ok |
| `para_contado.c` | **M1** | `PARA … DE … ATE … PASSO … FACA` / `FIM_PARA` | ok | ok |
| `tipos.c` | M0 | **um valor de cada tipo** (Fase 4): `LOGICO`, `CARACTERE`, inteiros nativos e de largura exata, naturais, reais, `TEXTO`/`TEXTO_FIXO`, `VAZIO`, `TAMANHO_DE`/`BITS_DE`, `OU_EXCLUSIVO`, `LOGICO_DE`, `ACRESCENTA_TEXTO` | ok | ok |
| `expressoes.c` | M0 | **semântica de expressões** (Fase 4): precedência, atribuição × comparação, valor de verdade, curto-circuito, conversões verificadas, `DIVIDE`/`RESTO`, texto seguro | ok | ok |
| `controle_condicoes.c` | M0 | **controle de fluxo** (Fase 3): `SE` aninhado, `SENAO_SE`, `E`/`OU`, `FUNCAO TEXTO`, `PARA` com valores negativos; sem `dangling else` (cada `SE` tem seu `FIM_SE`) | ok | ok |
| `controle_lacos.c` | M0 | **laços aninhados** (Fase 3): `PROSSIGA` no laço interno, `ENQUANTO` com contador (Collatz), `REPITA`/`ATE_QUE`, `PARE` com bandeira para sair de dois laços | ok | ok |
| `controle_acumuladores.c` | M0 | **contadores e acumuladores** (Fase 3): soma, produto, contagem condicional, maior/menor, média `REAL`, laço vazio, `PARA_PASSO` decrescente | ok | ok |
| `controle_entrada.c` | M0 | **laços dirigidos por entrada** (Fase 3): validação com `REPITA` + `LEIA_INTEIRO`, sentinela com `ENQUANTO`, rodada com zero iterações, `PROCEDIMENTO` | ok | ok |
| `controle_para_acucar.c` | **M1** | açúcar `PARA i DE … ATE … PASSO … FACA` / `FIM_PARA` aninhado, `ATE` no `REPITA`, zero iterações | ok | ok |
| `biblioteca_consola.c` | M0 + M1 | `bg_leia_linha`, escrita literal, limite de buffer e linha do terminal | ok | ok |
| `biblioteca_matematica.c` | M0 + M1 | `ABS`, `MINIMO`, `MAXIMO`, `POTENCIA`, `RAIZ`, `DIVIDE`, `RESTO`, `PISO`, `TETO`, `ARREDONDA` | ok | ok |
| `biblioteca_texto.c` | M0 + M1 | cópia, acréscimo, trecho, comprimento e conversão de texto para inteiro | ok | ok |
| `biblioteca_caractere.c` | M0 + M1 | classificação ASCII e conversão de caixa | ok | ok |
| `biblioteca_utilidades.c` | M0 + M1 | semente explícita, `bg_sorteia_ate` e intervalo limitado de `rand()` | ok | ok |

Os 24 exemplos compilam sem avisos com `-std=c11 -Wall -Wextra -Wformat=2 -Wvla
-Werror` e produzem exatamente a saída registrada — nos dois perfis, exceto
`para_contado.c` e `controle_para_acucar.c`, que são M1 puros por desenho. Os cinco
programas `biblioteca_*.c` exercitam os headers modulares e são ligados à biblioteca
estática. A suíte verifica os dois caminhos a cada `make test`.

A validação também forçou três ajustes de contrato, registrados em
[04 §6](../04-restricoes-e-decisoes.md): o contrato de `LEIA_TEXTO` (C-20, corrigido duas
vezes até ficar correto), a exigência de conversão de texto sem sobra (RB-7) e a
descoberta de que `CONSTANTE` não serve como tamanho de vetor (C-23) — `cadastro.c`
nasceu com um VLA silencioso até `-Wvla` ser ligado.

## Saídas esperadas (leitura humana)

As saídas abaixo são reais, com entradas indicadas; em alguns casos mostram só o trecho
que importa. A comparação automática é feita contra os arquivos `.esperado.txt` /
`.contem.txt` descritos acima (e não contra este documento, para não haver duas verdades).
Espaços em branco fazem parte do teste.

### `ola_mundo.c`
```
Ola, mundo!
```

### `media_notas.c` — entrada `8.5`, `6`
```
Nota 1: Nota 2: Media: 7.25  Situacao: Aprovado
```

### `tabuada.c` — entrada `3` (primeiras linhas)
```
3 x  1 =   3
3 x  2 =   6
...
3 x 10 =  30
```

### `primos.c` — entrada `30`
```
Primos ate qual valor? 2 3 5 7 11 13 17 19 23 29 

Total de primos: 10
Primeiro multiplo de 7 acima de 100: 105
```

### `vetores.c` — sem entrada
```
Vetor:    3  10   6   2   9   5   1   8
Soma:   44
Media:  5.50
Maior:  10
```

### `cadastro.c` — entrada `2 / 101 / Ana Silva / 9.5 / 102 / Bia Souza / 6.5 / 0`
```
MATR.  NOME                   NOTA
101    Ana Silva              9.50
102    Bia Souza              6.50
Media da turma: 8.00
Buscar matricula (0 encerra): Fim do cadastro.
```
(com `102` como primeira busca: `Encontrado: Bia Souza  nota 6.50`)

### `adivinha.c` — semente fixa, alvo 38 (glibc) ou 55 (MSVCRT/UCRT)

A entrada fixa `50, 25, 38, 44, 52, 54, 55` cobre os dois alvos: `rand()` com a
mesma semente só reproduz a sequência na mesma implementação da biblioteca C.
No segundo caso, são sete palpites: `Muito baixo!` seis vezes e `Acertou!`;
a saída termina com `Tentativas: 7` e `Dentro do esperado.` e está registrada
em `adivinha.alternativo.txt`. Saída no glibc:
```
Adivinhe o numero entre 1 e 100
Seu palpite: Muito alto!
Seu palpite: Muito baixo!
Seu palpite: Acertou!
Tentativas: 3
Muito bom.
```

### `contador_vogais.c` — entrada `Programacao em Portugol`
```
Caracteres: 23
Vogais: 9
Mascarada: Pr*gr*m*c** *m P*rt*g*l
```

### `interop_c.c` — sem entrada
```
Aluno: Ana  nota: 9.5
Distancia entre (0,0) e (3,4): 5.0
[C] iteracao 1
...
[BG] iteracao 1
...
Letras em 'basico': 6
Ano do sistema: <ano corrente>
```

### `fatorial_recursivo.c` — sem entrada (primeiras linhas)
```
fatorial( 0) = 1
fatorial( 1) = 1
...
fib( 0) = 0
```

### `utilidades.c` — sem entrada
Saída completa determinística; trechos que fazem parte do contrato:
```
CONCEITO_B vale 11
JUNTA_TEXTO em buffer pequeno coube? 0
TEXTO_PARA_INTEIRO rejeitou "4x2" (correto)
AFIRME passou: contrato interno satisfeito.
```
(mais a linha `(esta linha vai para stderr)` no fluxo de erro)

### `tipos.c` — sem entrada (trecho)
```
LOGICO        1 0
INTEIRO8/16/32/64: 100 30000 2000000000 9000000000
  largura exata: 8/16/32/64 bits
NATURAL       60000  (sem sinal; mesma largura de INTEIRO? 1)
  NATURAL8 250 + 10 = 4
  (modulo 256, comportamento definido)
REAL       3.1415926536
  0.1 + 0.2 == 0.3 ? 0   |diferenca| < EPSILON_REAL ? 1
TEXTO         "BASICGOL"
  copia + acrescimo coube? 1
  TEXTO_FIXO     "tipo texto"  tamanho 10
```
(a saída completa está em `tipos.esperado.txt`)

### `expressoes.c` — sem entrada (trecho)
```
  2 + 3 * 4        = 14
  (1 E 0) OU 1     = 1
  NAO 0 E 0        = 0
  -7 / 2           = -3
  (INTEIRO) 3.9     = 3
  (NATURAL8) 300    = 44
  TEXTO_PARA_INTEIRO("4x2")   = 0  (n continua 42)
  DIVIDE(-7, 2)     = -3
  acrescentar demais         = 0  -> tamanho 0
  TRECHO_TEXTO("BASICGOL", 5, 3) = 1  -> "GOL"
```
(a saída completa está em `expressoes.esperado.txt`)

### `para_contado.c` — sem entrada
```
Contagem crescente:
1 2 3 4 5 
Contagem decrescente:
5 4 3 2 1 
Pares de 0 a 10:
0 2 4 6 8 10 
Mesma coisa sem acucar:
0 2 4 6 8 10 
```

## Regras para novos exemplos

1. O cabeçalho do arquivo declara **nome, propósito, perfil (M0 ou M0+M1) e comando de
   compilação**.
2. Nada de construtor fora da especificação; se precisar de C, marque com comentário
   (`// C nativo`) — é permitido, mas o leitor deve saber.
3. Todo exemplo é determinístico: sementes fixas, sem leitura de relógio em resultados
   comparados, sem dependência de `locale`.
4. Toda variável é inicializada na declaração.
5. O exemplo compila sem avisos com `-Wall -Wextra -Wformat=2`.
6. Se o exemplo exercita um contrato sutil (C-20, truncamento, avaliação única), ele
   **deve** verificar o resultado, não apenas chamar a função.
