# BASICGOL — Makefile
#
#   make                 compila o transpilador e a biblioteca estática
#   make biblioteca      compila build/libbasicgol.a
#   make test            roda a suíte da linguagem e os testes da biblioteca
#   make test-biblioteca roda os testes unitários dos módulos da biblioteca
#   make test-v          suíte completa, mostrando cada verificação aprovada
#   make exemplos        compila os exemplos de docs/exemplos
#   make cobertura       regenera tests/COBERTURA.md
#   make limpar          remove build/

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

.PHONY: all biblioteca test test-v test-biblioteca smoke smoke-v exemplos cobertura limpar ajuda

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
	@T_LIBRARY="$(BIBLIOTECA)" ./tests/run.sh
	@./tests/run_biblioteca.sh "$(TESTE_BIBLIOTECA)"

test-v: $(FERRAMENTA) $(BIBLIOTECA) $(TESTE_BIBLIOTECA)
	@T_LIBRARY="$(BIBLIOTECA)" ./tests/run.sh -v
	@./tests/run_biblioteca.sh "$(TESTE_BIBLIOTECA)"

test-biblioteca: $(TESTE_BIBLIOTECA)
	@./tests/run_biblioteca.sh "$(TESTE_BIBLIOTECA)"

# smoke: suíte completa de compile-e-rodar (M0 + M1) e teste dos módulos.
smoke: test

smoke-v: test-v

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

cobertura:
	@python3 tools/cobertura.py

# --------------------------------------------------------------- limpeza

limpar:
	rm -rf $(BUILD)

ajuda:
	@sed -n '2,12p' Makefile
