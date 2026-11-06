# BASICGOL — Makefile
#
#   make                 compila o transpilador e a biblioteca estática
#   make biblioteca      compila build/libbasicgol.a
#   make test            roda a suíte da linguagem e os testes da biblioteca
#   make test-biblioteca roda os testes unitários dos módulos da biblioteca
#   make test-v          suíte completa, mostrando cada verificação aprovada
#   make test-portabilidade  matriz de compiladores, avisos, sanitizers e alvos
#   make test-sanitize   suíte completa sob ASan + UBSan (build-sanitize/)
#   make test-matriz     suíte completa com cada compilador encontrado
#   make fuzz            mutação determinística do transpilador
#   make test-checkout   roda os comandos do README a partir de um checkout limpo
#   make exemplos        compila os exemplos de docs/exemplos
#   make cobertura       regenera tests/COBERTURA.md
#   make limpar          remove build/ e os diretórios de build alternativos

CC       ?= cc
AR       ?= ar
CFLAGS   ?= -O2
WARN     := -std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror
INCLUDE  := include
BUILD    := build
FERRAMENTA := $(BUILD)/basicgolc
BIBLIOTECA := $(BUILD)/libbasicgol.a
TESTE_BIBLIOTECA := $(BUILD)/test-biblioteca
FONTES_BIBLIOTECA := $(sort $(wildcard src/biblioteca/*.c))
OBJETOS_BIBLIOTECA := $(patsubst src/biblioteca/%.c,$(BUILD)/biblioteca/%.o,$(FONTES_BIBLIOTECA))

.PHONY: all biblioteca test test-v test-biblioteca smoke smoke-v exemplos cobertura \
        limpar ajuda test-portabilidade test-portabilidade-v test-sanitize test-matriz fuzz \
        test-checkout

all: $(FERRAMENTA) $(BIBLIOTECA)

$(FERRAMENTA): tools/basicgolc.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(WARN) -o $@ tools/basicgolc.c

$(BUILD)/biblioteca/%.o: src/biblioteca/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(WARN) -I $(INCLUDE) -c -o $@ $<

$(BIBLIOTECA): $(OBJETOS_BIBLIOTECA)
	@mkdir -p $(BUILD)
	rm -f $@
	$(AR) rcs $@ $(OBJETOS_BIBLIOTECA)

biblioteca: $(BIBLIOTECA)

$(TESTE_BIBLIOTECA): tests/test_biblioteca.c $(BIBLIOTECA)
	$(CC) $(CFLAGS) $(WARN) -I $(INCLUDE) -o $@ $< $(BIBLIOTECA) -lm

# ------------------------------------------------------------------ testes

test: $(FERRAMENTA) $(BIBLIOTECA) $(TESTE_BIBLIOTECA)
	@T_BASICGOLC="$(FERRAMENTA)" T_LIBRARY="$(BIBLIOTECA)" ./tests/run.sh
	@./tests/run_biblioteca.sh "$(TESTE_BIBLIOTECA)"

test-v: $(FERRAMENTA) $(BIBLIOTECA) $(TESTE_BIBLIOTECA)
	@T_BASICGOLC="$(FERRAMENTA)" T_LIBRARY="$(BIBLIOTECA)" ./tests/run.sh -v
	@./tests/run_biblioteca.sh "$(TESTE_BIBLIOTECA)"

test-biblioteca: $(TESTE_BIBLIOTECA)
	@./tests/run_biblioteca.sh "$(TESTE_BIBLIOTECA)"

# smoke: suíte completa de compile-e-rodar (M0 + M1) e teste dos módulos.
smoke: test

smoke-v: test-v

# ------------------------------------------------- Fase 6: portabilidade
#
# Nada disto é obrigatório para usar a linguagem: são verificações de
# compilador, aviso e sanitizer. Ferramenta ausente é REPORTADA como ignorada.

test-portabilidade: $(FERRAMENTA) $(BIBLIOTECA)
	@T_BASICGOLC="$(FERRAMENTA)" T_LIBRARY="$(BIBLIOTECA)" \
	    ./tests/run_portabilidade.sh

test-portabilidade-v: $(FERRAMENTA) $(BIBLIOTECA)
	@T_BASICGOLC="$(FERRAMENTA)" T_LIBRARY="$(BIBLIOTECA)" \
	    ./tests/run_portabilidade.sh -v

# A suíte inteira sob ASan+UBSan, em um diretório de build separado para não
# misturar objetos instrumentados com os normais (o Makefile não rastreia
# mudança de flags; é por isso que existe BUILD).
SAN_FLAGS := -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all

test-sanitize:
	@command -v $(CC) > /dev/null 2>&1 || { echo "test-sanitize: $(CC) nao encontrado" >&2; exit 2; }
	@rm -rf build-sanitize
	@$(MAKE) --no-print-directory BUILD=build-sanitize \
	    CFLAGS="$(SAN_FLAGS)" \
	    T_CFLAGS="$(WARN) $(SAN_FLAGS)" \
	    ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=print_stacktrace=1 test

# A mesma suíte com cada compilador C que existir nesta máquina.
test-matriz:
	@./tests/run_matriz.sh

# Mutação determinística do transpilador: contrato = não derrubar, não travar.
fuzz: $(FERRAMENTA)
	@python3 tests/fuzz_basicgolc.py --basicgolc "$(FERRAMENTA)"

# Os comandos documentados funcionam a partir de um checkout limpo (sem build/)?
test-checkout:
	@./tests/run_checkout_limpo.sh

# ---------------------------------------------------------------- exemplos

exemplos: $(FERRAMENTA) $(BIBLIOTECA)
	@mkdir -p $(BUILD)/exemplos
	@for f in docs/exemplos/*.c; do \
	    b=$$(basename $$f .c); \
	    printf '%-30s ' "$$b"; \
	    if [ "$$b" = "para_contado" ] || [ "$$b" = "controle_para_acucar" ]; then \
	        $(FERRAMENTA) "$$f" -o $(BUILD)/exemplos/$$b.c 2>/dev/null && \
	        $(CC) $(WARN) -I $(INCLUDE) -o $(BUILD)/exemplos/$$b $(BUILD)/exemplos/$$b.c $(BIBLIOTECA) -lm 2>/dev/null && \
	        echo "ok (M1)"; \
	    else \
	        $(CC) $(WARN) -I $(INCLUDE) "$$f" -o $(BUILD)/exemplos/$$b $(BIBLIOTECA) -lm && echo "ok (M0)"; \
	    fi; \
	done

# ------------------------------------------------------------- cobertura

# A saída é determinística (sem data) para o CI comparar com o arquivo
# versionado; `make cobertura COBERTURA_ARGS=--data` gera uma versão datada.
cobertura:
	@python3 tools/cobertura.py $(COBERTURA_ARGS)

# --------------------------------------------------------------- limpeza

limpar:
	rm -rf $(BUILD) build-sanitize build-matriz

ajuda:
	@sed -n '2,16p' Makefile
