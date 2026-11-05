# Exemplos BASICGOL

Doze programas completos, escritos **apenas** com construtores definidos na
[especificação](../01-especificacao.md). Eles servem a três propósitos:

1. **Documentação executável** — cada exemplo demonstra um conjunto de construtores.
2. **Especificação por exemplos** — se um exemplo não compilar, a especificação está
   errada ou incompleta.
3. **Testes dourados** — a saída esperada de cada programa está registrada abaixo e é a
   base dos testes `T-execução` da Fase 2 ([critérios](../05-criterios-de-aceitacao.md)).

## Como compilar

Desde a Fase 2 o cabeçalho existe ([`include/basicgol.h`](../../include/basicgol.h)) e
todos os exemplos são compilados e executados pela suíte, nos dois perfis
(`make test`, suíte `10_exemplos`).

Perfil M0 (só precisa de um compilador C11):

```sh
cc -std=c11 -Wall -Wextra -Wformat=2 -x c ola_mundo.bg -o ola_mundo -lm
```

Com o transpilador (perfil M1 — todos os exemplos passam por aqui):

```sh
make                                    # constrói build/basicgolc
build/basicgolc para_contado.bg -o para_contado.c
cc -std=c11 -Wall -Wextra -Wformat=2 para_contado.c -o para_contado -lm -I ../../include
```

### Arquivos de ouro ao lado dos fontes

Cada exemplo tem, ao lado do `.bg`, os arquivos que a suíte consome:

| Sufixo | Papel |
|---|---|
| `.entrada.txt` | o que vai para o `stdin` (ausente = sem entrada) |
| `.esperado.txt` | a saída **completa** esperada em `stdout` |
| `.contem.txt` | linhas que precisam aparecer em `stdout` (para saída que depende do relógio) |
| `.esperado_stderr.txt` | linha que precisa aparecer em `stderr` |

Eles são a fonte de verdade da máquina; a seção "Saídas esperadas" abaixo continua sendo a
leitura humana, e pode mostrar trechos em vez do arquivo inteiro.

## Registro de validação (Fase 1)

Compilados com GCC 12.2, `-std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror`: **zero avisos**.

| Arquivo | Perfil | Construtores demonstrados | Compila | Executa |
|---|---|---|---|---|
| `ola_mundo.bg` | M0 | `INICIO`, `ESCREVAL`, `FIM` | ok | ok |
| `media_notas.bg` | M0 | `FUNCAO`, `TEXTO` como retorno, `SE`/`SENAO_SE`/`SENAO`, `LEIA_OK`, `ERRO`, `ESCREVA_FMT` | ok | ok |
| `tabuada.bg` | M0 | `PARA`, `PARA_PASSO`, laços aninhados, `SE` com `OU`, `ESCREVA_FMT` | ok | ok |
| `primos.bg` | M0 | `FUNCAO LOGICO`, `ENQUANTO`, `PARE`, `PROSSIGA`, `MOD`, retorno antecipado | ok | ok |
| `vetores.bg` | M0 | `VETOR`, `TAMANHO_VETOR`, `INTEIRO v[]` + tamanho, `PROCEDIMENTO` | ok | ok |
| `cadastro.bg` | M0 | `REGISTRO`/`FIM_REGISTRO`, vetor de registros, `TEXTO_FIXO`, `LEIA` + `LEIA_TEXTO` (C-20), `ESCREVA_FMT` com `%-20s` | ok | ok |
| `adivinha.bg` | M0 | `REPITA`/`ATE_QUE`, `SORTEIA`, `INICIA_SORTEIO`, `ESCOLHA`/`CASO` agrupados | ok | ok |
| `contador_vogais.bg` | M0 | `TEXTO_FIXO` em parâmetro como `CARACTERE destino[]`, `TAMANHO_TEXTO`, percurso de texto (entrada limpa) | ok | ok |
| `interop_c.bg` | M0 + C | `#include <time.h>`, `struct`, função estática em C, ponteiro, `for`/`printf` misturados | ok | ok |
| `fatorial_recursivo.bg` | M0 | recursão, `INTEIRO64`, `ESCREVA_FMT("%lld")` | ok | ok |
| `utilidades.bg` | M0 | catálogo de tipos, `ABS`/`MINIMO`/`MAXIMO`/`POTENCIA`/`RAIZ`/`ARREDONDA`/`PISO`/`TETO`, `COPIA_TEXTO`/`JUNTA_TEXTO`, `TEXTO_PARA_INTEIRO`/`TEXTO_PARA_REAL`, `ENUMERACAO`, `AFIRME`, `ESCREVA_ERRO`, `ESTATICO`, `VAZIO` | ok | ok |
| `para_contado.bg` | **M1** | `PARA … DE … ATE … PASSO … FACA` / `FIM_PARA` | ok | ok |

Todos os 12 exemplos compilam sem avisos com `-std=c11 -Wall -Wextra -Wformat=2 -Wvla
-Werror` e produzem exatamente a saída registrada — nos dois perfis, exceto
`para_contado.bg`, que é M1 puro por desenho. É o que a suíte verifica a cada `make test`.

A validação também forçou três ajustes de contrato, registrados em
[04 §6](../04-restricoes-e-decisoes.md): o contrato de `LEIA_TEXTO` (C-20, corrigido duas
vezes até ficar correto), a exigência de conversão de texto sem sobra (RB-7) e a
descoberta de que `CONSTANTE` não serve como tamanho de vetor (C-23) — `cadastro.bg`
nasceu com um VLA silencioso até `-Wvla` ser ligado.

## Saídas esperadas (leitura humana)

As saídas abaixo são reais, com entradas indicadas; em alguns casos mostram só o trecho
que importa. A comparação automática é feita contra os arquivos `.esperado.txt` /
`.contem.txt` descritos acima (e não contra este documento, para não haver duas verdades).
Espaços em branco fazem parte do teste.

### `ola_mundo.bg`
```
Ola, mundo!
```

### `media_notas.bg` — entrada `8.5`, `6`
```
Nota 1: Nota 2: Media: 7.25  Situacao: Aprovado
```

### `tabuada.bg` — entrada `3` (primeiras linhas)
```
3 x  1 =   3
3 x  2 =   6
...
3 x 10 =  30
```

### `primos.bg` — entrada `30`
```
Primos ate qual valor? 2 3 5 7 11 13 17 19 23 29 

Total de primos: 10
Primeiro multiplo de 7 acima de 100: 105
```

### `vetores.bg` — sem entrada
```
Vetor:    3  10   6   2   9   5   1   8
Soma:   44
Media:  5.50
Maior:  10
```

### `cadastro.bg` — entrada `2 / 101 / Ana Silva / 9.5 / 102 / Bia Souza / 6.5 / 0`
```
MATR.  NOME                   NOTA
101    Ana Silva              9.50
102    Bia Souza              6.50
Media da turma: 8.00
Buscar matricula (0 encerra): Fim do cadastro.
```
(com `102` como primeira busca: `Encontrado: Bia Souza  nota 6.50`)

### `adivinha.bg` — semente fixa, alvo 38, entradas `50`, `25`, `38`
```
Adivinhe o numero entre 1 e 100
Seu palpite: Muito alto!
Seu palpite: Muito baixo!
Seu palpite: Acertou!
Tentativas: 3
Muito bom.
```

### `contador_vogais.bg` — entrada `Programacao em Portugol`
```
Caracteres: 23
Vogais: 9
Mascarada: Pr*gr*m*c** *m P*rt*g*l
```

### `interop_c.bg` — sem entrada
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

### `fatorial_recursivo.bg` — sem entrada (primeiras linhas)
```
fatorial( 0) = 1
fatorial( 1) = 1
...
fib( 0) = 0
```

### `utilidades.bg` — sem entrada
Saída completa determinística; trechos que fazem parte do contrato:
```
CONCEITO_B vale 11
JUNTA_TEXTO em buffer pequeno coube? 0
TEXTO_PARA_INTEIRO rejeitou "4x2" (correto)
AFIRME passou: contrato interno satisfeito.
```
(mais a linha `(esta linha vai para stderr)` no fluxo de erro)

### `para_contado.bg` — sem entrada
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
