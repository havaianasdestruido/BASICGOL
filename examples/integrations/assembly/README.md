# BASICGOL + Assembly (7 exemplos)

Os programas BASICGOL declaram funções C comuns; cada implementação correspondente está em Assembly GNU (`.S`). O compilador C faz o link final e as chamadas seguem a ABI C.

> **Alvo destes fontes:** Linux x86-64, ABI System V e sintaxe GNU Assembly AT&T. Não são exemplos portáveis para ARM, Windows x64/MASM ou outras ABIs.

## Compilar e executar

Na raiz do repositório, com GCC/Clang e GNU assembler disponíveis:

```sh
make -C examples/integrations/assembly test
```

Os executáveis vão para `build/integrations/assembly/`. Cada pasta contém `programa.c` (BASICGOL) e `operacao.S` (Assembly).

| Exemplo | Operação implementada em Assembly |
|---|---|
| `01_soma` | Soma de inteiros. |
| `02_maior` | Maior de dois inteiros com sinal. |
| `03_multiplica` | Multiplicação inteira. |
| `04_absoluto` | Valor absoluto para valores diferentes de `INT_MIN`. |
| `05_media` | Média de dois `double` via registradores SSE2. |
| `06_soma_vetor` | Percorre um vetor C de inteiros e soma os elementos. |
| `07_mdc` | Algoritmo de Euclides para entradas positivas. |

Estes exemplos mostram a interface de chamada, não validação de argumentos. Em particular, MDC espera operandos positivos e a rotina de valor absoluto não define `abs(INT_MIN)`.
