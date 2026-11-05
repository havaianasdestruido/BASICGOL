# BASICGOL — Makefile
#
#   make            compila o transpilador (build/basicgolc)
#   make test       roda a suíte completa
#   make test-v     suíte completa, mostrando cada verificação aprovada
#   make exemplos   compila e roda os exemplos de docs/exemplos (caminho M0)
#   make cobertura  regenera tests/COBERTURA.md a partir da árvore de testes
#   make limpar     remove build/

CC       ?= cc
CFLAGS   ?= -O2
WARN     := -std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror
INCLUDE  := include
BUILD    := build
FERRAMENTA := $(BUILD)/basicgolc

.PHONY: all test test-v exemplos cobertura limpar ajuda

all: $(FERRAMENTA)

$(FERRAMENTA): tools/basicgolc.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(WARN) -o $@ tools/basicgolc.c

# ------------------------------------------------------------------ testes

test: $(FERRAMENTA)
	@./tests/run.sh

test-v: $(FERRAMENTA)
	@./tests/run.sh -v

# ---------------------------------------------------------------- exemplos

exemplos: $(FERRAMENTA)
	@mkdir -p $(BUILD)/exemplos
	@for f in docs/exemplos/*.bg; do \
	    b=$$(basename $$f .bg); \
	    printf '%-22s ' "$$b"; \
	    if [ "$$b" = "para_contado" ]; then \
	        $(FERRAMENTA) "$$f" -o $(BUILD)/exemplos/$$b.c 2>/dev/null && \
	        $(CC) $(WARN) -I $(INCLUDE) -o $(BUILD)/exemplos/$$b $(BUILD)/exemplos/$$b.c -lm 2>/dev/null && \
	        echo "ok (M1)"; \
	    else \
	        $(CC) $(WARN) -I $(INCLUDE) -x c "$$f" -o $(BUILD)/exemplos/$$b -lm && echo "ok (M0)"; \
	    fi; \
	done

# ------------------------------------------------------------- cobertura

cobertura:
	@python3 tools/cobertura.py

# --------------------------------------------------------------- limpeza

limpar:
	rm -rf $(BUILD)

ajuda:
	@sed -n '2,11p' Makefile
