# Sete exemplos: cada fonte BASICGOL e compilada em C11; o cliente e compilado em C++17.
ROOT := ../../..
OUT := $(ROOT)/build/integrations/cpp
CC ?= cc
CXX ?= c++
CFLAGS ?= -O2
WARN_C := -std=c11 -Wall -Wextra -Wformat=2 -Wvla -Werror
WARN_CXX := -std=c++17 -Wall -Wextra -Werror
SAMPLES := 01_soma 02_pontos 03_vetor 04_callback 05_cpp_chama_bg 06_texto_utf8 07_raii

.PHONY: all test
all: $(addprefix $(OUT)/,$(SAMPLES))

$(OUT)/%: %/basicgol.c %/main.cpp %/api.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(WARN_C) -I $(ROOT)/include -I $* -c $*/basicgol.c -o $(@D)/$*-basicgol.o
	$(CXX) $(CXXFLAGS) $(WARN_CXX) -I $* $*/main.cpp $(@D)/$*-basicgol.o -lm -o $@

test: all
	@set -e; for sample in $(SAMPLES); do echo "--- $$sample"; $(OUT)/$$sample; done
