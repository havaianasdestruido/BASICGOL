# 14 — Auditoria de compiladores e exemplos (17/11/2026)

Esta é uma execução nova, não uma alteração do relatório histórico da Fase 6.
Ambiente: Debian 12, x86-64, glibc 2.36, GCC 12.2.0. Só foi possível acessar
GitHub (repositórios), npm e PyPI; `apt-get update` não conseguiu conectar a
`deb.debian.org`, e downloads de releases GitHub redirecionaram para
`objects.githubusercontent.com`, também inacessível. Binários/fontes baixados
ficaram fora do repositório. **A solicitação de testar todos os compiladores não
foi concluída**: veja os impedimentos abaixo.

## Escopo e leitura dos números

Foram descobertos **32 arquivos `.c`**: 24 em `docs/exemplos` e oito em
`examples`. O novo `tests/auditar_exemplos.py` tenta M0 e M1; dois arquivos de
`docs/exemplos` são M1 por definição. Nos 29 arquivos comuns são 56 tentativas:
46 dos exemplos documentados e 10 dos cinco outros. O programa confere a saída
dourada de `docs/exemplos` (inclusive entrada, trechos e stderr); as quatro
cópias em `examples` usam os mesmos arquivos dourados. O BASIC-Z depende do
sistema e, se compilar, é verificado apenas por marcadores de saída.

Os outros três arquivos (`basicz-gui`, `bootloader`, `wasm`) têm alvo especial:
o script faz **seis sondagens no host**, gerando apenas objetos, sem afirmar
que Windows, boot ou WebAssembly foram compilados ou executados. As sondagens
falhas não entram nos 56 testes comuns. Cada linha dos resultados está em
[`relatorios/2026-11-17/`](relatorios/2026-11-17/); caminhos temporários e PIDs
e espaços ao fim das linhas foram normalizados. Os totais são verificações de perfil, não arquivos únicos.

| Compilador/configuração | Exemplos comuns (56) | Sondagens especiais (6) | Observações |
|---|---:|---:|---|
| GCC 12.2 | 54 passaram, 2 falharam | 0 compilaram, 6 falharam | 46/46 exemplos documentados passaram |
| LLVM `clang` 24.0.0git | 54 passaram, 2 falharam | 0 compilaram, 6 falharam | 46/46 documentados; suíte completa passou |
| `clang-cl` 24.0.0git, alvo **Linux** | 54 passaram, 2 falharam | 0 compilaram, 6 falharam | Compilação pelo driver `clang-cl`, ligação com GCC e biblioteca GCC; **não é teste Windows/MSVC** |
| TCC 0.9.28rc | 54 passaram, 2 falharam | 0 compilaram, 6 falharam | 46/46 documentados; suíte completa não passou |
| cproc + QBE | 50 passaram, 6 falharam | 0 compilaram, 6 falharam | C de exemplos compilado com cproc; transpilador e biblioteca foram construídos com GCC, pois o build completo com cproc falhou |
| 8cc | 8 passaram, 48 falharam | 0 compilaram, 6 falharam | Os objetos são produzidos pelo 8cc e ligados por GCC à biblioteca GCC; basicgolc também é GCC |

`-std=c11` foi usado em GCC, clang, TCC e cproc; no `clang-cl`,
`/std:c11 --target=x86_64-linux-gnu`. O 8cc foi chamado com `-c` sem essa
flag, que não é sua interface. Um resultado com um compilador não valida o
projeto inteiro se os componentes foram construídos com outro.

### Falhas observadas

* O exemplo `examples/advanced/basicz/basicz.c` falha em M0: `ENQUANTO` é
  escrito com `{` onde a macro exige `FACA`. Em M1, o transpilador também o
  rejeita. Isso explica as duas falhas comuns em GCC/clang/clang-cl/TCC.
* `basicz-gui` não possui `windows.h` no host; M1 também é rejeitado por
  `ENQUANTO` sem `FACA`. `bootloader` tem o mesmo problema de sintaxe e não
  há linker script no exemplo. `wasm_exemplo.c` chama `ESCREVAL` com dois
  argumentos, mas a macro aceita apenas um; isso falha na sondagem nativa M0
  e M1. Nenhuma dessas sondagens substitui o teste no alvo.
* cproc: `tipos` não compila por `__CHAR_BIT__` não definido; `utilidades`
  falha porque o backend informa não suportar `long double` (ambos perfis).
  Além disso, `make ... all` falhou em `src/biblioteca/matematica.c` por
  `__LONG_MAX__` não definido. Não se afirma que cproc passa o build completo.
* 8cc: quatro exemplos modulares (`biblioteca_caractere`, `biblioteca_consola`,
  `biblioteca_texto`, `biblioteca_utilidades`) compilaram e executaram em
  ambos os perfis com biblioteca/ligador GCC. Os demais erros estão no log:
  a maior parte encontra falha do parser em `/usr/include/math.h` do sistema.
  A opção `-ffreestanding` da sondagem de boot não é aceita pelo 8cc.

## Suíte completa

* `make CC=gcc BUILD=build-gcc test`: **307** verificações de linguagem,
  **230** verificações internas da biblioteca e **30** verificações do runner
  da biblioteca, aprovadas.
* Com o `clang` abaixo e `BUILD=build-clang`, os mesmos **307 + 230 + 30**
  passaram. Este é o driver LLVM `clang`, não `zig cc`.
* TCC: `make CC=tcc BUILD=build-tcc test` falhou em **20 de 306** verificações
  de linguagem ([log](relatorios/2026-11-17/suite-tcc.txt)). Há divergências
  na representação das expansões, recusa documentada de `BG_USAR_EXTENSOES`,
  17 diagnósticos negativos cuja redação não corresponde às variantes aceitas,
  e um teste que depende de `-Wsizeof-pointer-div` mas compilou. Não se deve
  interpretar as diferenças de texto como aceitação dos programas inválidos.
  `make CC=tcc BUILD=build-tcc test-biblioteca`, executado separadamente,
  passou as **230 + 30** verificações. A suíte de exemplos passou **46/46**.
  As flags de aviso aceitas por TCC não garantem os mesmos diagnósticos do GCC.
* cproc e 8cc não tiveram suíte completa aprovada; a auditoria isolada dos
  exemplos não substitui a construção e os testes de todos os componentes.

## Obtenção e lacunas

| Item | Resultado concreto |
|---|---|
| `clang` / `clang-cl` | Obtido `@lasm/emscripten-linux-x64@6.0.9-lasm.1` do npm. O arquivo interno tem SHA-256 `e3f417bc9d5525c303a35fe8c4ab2d6229f8b45a44f2edd40b3c191fb0e67de7`; versão LLVM `510126255f89d693717c1ce7105b593f995f07c1`. `clang-cl` é um link para o mesmo executável, ativando o modo do driver. O alvo Windows padrão falhou em `stdio.h` ausente: não há SDK/UCRT MSVC. |
| TCC | `https://github.com/TinyCC/tinycc`, commit `f830a1697e159056b75de40a60464bc43ee670a3`; `./configure`, `make`, `make install`. |
| cproc / QBE | `https://github.com/michaelforney/cproc`, commit `d1c53ddf56571573a7025324c8dd5c6d547a4d1f`, e `https://github.com/omenos/mirror-qbe`, commit `180d2eb4157b706dc480bce50ff6d03a65ec334b`; construídos localmente. |
| 8cc | `https://github.com/rui314/8cc`, commit `b480958396f159d3794f0d4883172b21438a8597`; construído com GCC. |
| PCC | Fonte `https://github.com/PortableCC/pcc`, commit `8b92dab2899f1ba0b99e9e541e6be6c13b2a95bf`. O build falhou por falta de yacc; compilou-se o byacc, mas faltou também lex/flex para gerar `scan.c`. Nenhum exemplo foi testado com PCC. |
| Digital Mars | Obtido `https://github.com/DigitalMars/dmc`, commit `9478d25a677f70dbe4fc0ed317cc5a5e5050ef8b`, inclusive `bin/dmc.exe`; não executado, pois não há Wine/Windows. |
| Open64 | Fonte `https://github.com/open64-compiler/open64`, commit `8aeb892ebfb8eb735c45f0fe260403aabe3527b4`. `configure` falhou, entre outras dependências, por falta de Gradle, Java 8 e CMake. A documentação recomenda GCC 5–7; disponível é GCC 12. Nenhum exemplo foi testado. |
| Intel oneAPI `icx` | Não obtido nem testado; o repositório de pacotes do fornecedor não está entre os hosts acessíveis. |
| Wine | Tentativa de baixar release de `Kron4ek/Wine-Builds` redirecionou para host bloqueado; não obtido nem executado. Nenhum resultado Windows foi inferido. |
| Restantes candidatos da [§8 da matriz](11-matriz-compatibilidade.md#8-candidatos-conhecidos-não-obtidos-lista-coletada-nada-executado) | Não obtidos nem testados nesta execução; vários exigem ferramentas proprietárias, sistemas ou arquiteturas diferentes. Não se deve interpretar a lista como uma bateria concluída. |

## Reproduzir

Depois de instalar os compiladores e construir os componentes em diretórios
separados (o Makefile não rastreia mudança de compilador):

```sh
make CC=gcc BUILD=build-gcc test
make CC=clang BUILD=build-clang test
make CC=tcc BUILD=build-tcc test
make CC=tcc BUILD=build-tcc test-biblioteca
python3 tests/auditar_exemplos.py --cc clang --flag=-std=c11 \
  --basicgolc build-clang/basicgolc --library build-clang/libbasicgol.a
python3 tests/auditar_exemplos.py --cc cproc --flag=-std=c11 \
  --basicgolc build-gcc/basicgolc --library build-gcc/libbasicgol.a
python3 tests/auditar_exemplos.py --cc 8cc --linker gcc \
  --basicgolc build-gcc/basicgolc --library build-gcc/libbasicgol.a
python3 tests/auditar_exemplos.py --cc clang-cl \
  --flag=--target=x86_64-linux-gnu --flag=/std:c11 --linker gcc \
  --basicgolc build-gcc/basicgolc --library build-gcc/libbasicgol.a
```

O script retorna código 1 se houver falha, inclusive nas sondagens especiais;
`--object-only` não liga nem executa. Para cproc, QBE e `cproc-qbe` devem estar
no `PATH`. Para o alvo Windows de `clang-cl`, instalar SDK/CRT e ligador
apropriados e executar em Windows/Wine são trabalhos ainda pendentes.

## Perguntas frequentes

### Esta auditoria comprova suporte a todos os compiladores listados?

Não. A tabela registra o que foi executado e as limitações de cada configuração;
compiladores não obtidos ou sondagens sem execução no alvo não comprovam suporte.
Consulte os logs e a matriz de compatibilidade antes de tirar conclusões.

### Como repetir a suíte com GCC?

Com GCC e as dependências instalados, use um diretório de build separado:

```sh
make CC=gcc BUILD=build-gcc test
```
