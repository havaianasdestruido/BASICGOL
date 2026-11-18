# GNU Assembly x86-64 (ABI System V); BASICGOL continua sendo compilado como C11.
ROOT := ../../..
OUT := $(ROOT)/build/integrations/assembly
CC ?= cc
CFLAGS ?= -O2
WARN_C := -std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror
SAMPLES := 01_soma 02_maior 03_multiplica 04_absoluto 05_media 06_soma_vetor 07_mdc

.PHONY: all test
all: $(addprefix $(OUT)/,$(SAMPLES))

$(OUT)/%: %/programa.c %/operacao.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(WARN_C) -I $(ROOT)/include -c $*/programa.c -o $(@D)/$*-programa.o
	$(CC) $(CFLAGS) -c $*/operacao.S -o $(@D)/$*-operacao.o
	$(CC) $(@D)/$*-programa.o $(@D)/$*-operacao.o -lm -o $@

test: all
	@set -e; for sample in $(SAMPLES); do echo "--- $$sample"; $(OUT)/$$sample; done
