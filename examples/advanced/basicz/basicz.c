// basicz.c -- BASIC-Z: a console rewrite inspired by CPU-Z.
// Backend gathers CPU, cache, memory, board and graphics info.
// Front-end renders a tabbed text UI reminiscent of CPU-Z.
// Build: cc -std=c11 -I ../../../../include basicz.c ... (see README)
#include <basicgol.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

FUNCAO VAZIO imprimir_separador() {
    ESCREVAL("----------------------------------------------------------------");
}

FUNCAO VAZIO aba_cpu() {
    ESCREVAL("[ CPU ]");
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    ESCREVA_FMT("  Processors (logical): %lu\n", si.dwNumberOfProcessors);
    ESCREVA_FMT("  Page size: %lu bytes\n", si.dwPageSize);
#else
    FILE *f = fopen("/proc/cpuinfo", "r");
    char linha[256];
    int mostrado = 0;
    SE f != NULL ENTAO
        ENQUANTO (fgets(linha, sizeof(linha), f) != NULL) {
            SE (strncmp(linha, "model name", 10) == 0 && !mostrado) ENTAO
                ESCREVA_FMT("  %s", linha);
                mostrado = 1;
            FIM_SE
            SE (strncmp(linha, "cpu cores", 9) == 0) ENTAO
                ESCREVA_FMT("  %s", linha);
            FIM_SE
            SE (strncmp(linha, "processor", 9) == 0) ENTAO
                // conta apenas o primeiro
            FIM_SE
        }
        fclose(f);
    SENAO
        ESCREVAL("  (nao foi possivel ler /proc/cpuinfo)");
    FIM_SE
#endif
    imprimir_separador();
}

FUNCAO VAZIO aba_memoria() {
    ESCREVAL("[ Memory ]");
#ifdef _WIN32
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    SE GlobalMemoryStatusEx(&ms) ENTAO
        ESCREVA_FMT("  Total physical: %llu MB\n", ms.ullTotalPhys / (1024*1024));
        ESCREVA_FMT("  Available:      %llu MB\n", ms.ullAvailPhys / (1024*1024));
    FIM_SE
#else
    FILE *f = fopen("/proc/meminfo", "r");
    char linha[256];
    SE f != NULL ENTAO
        ENQUANTO (fgets(linha, sizeof(linha), f) != NULL) {
            SE (strncmp(linha, "MemTotal", 8) == 0 || strncmp(linha, "MemAvailable", 12) == 0) ENTAO
                ESCREVA_FMT("  %s", linha);
            FIM_SE
        }
        fclose(f);
    SENAO
        ESCREVAL("  (nao foi possivel ler /proc/meminfo)");
    FIM_SE
#endif
    imprimir_separador();
}

FUNCAO VAZIO aba_cache() {
    ESCREVAL("[ Caches ]");
#ifdef _WIN32
    ESCREVAL("  (cache info via CPUID nao implementado nesta versao)");
#else
    FILE *f = fopen("/proc/cpuinfo", "r");
    char linha[256];
    SE f != NULL ENTAO
        ENQUANTO (fgets(linha, sizeof(linha), f) != NULL) {
            SE (strncmp(linha, "cache size", 10) == 0) ENTAO
                ESCREVA_FMT("  %s", linha);
            FIM_SE
        }
        fclose(f);
    FIM_SE
#endif
    imprimir_separador();
}

FUNCAO VAZIO aba_mainboard() {
    ESCREVAL("[ Mainboard ]");
#ifdef _WIN32
    ESCREVAL("  (board info via WMI nao implementado nesta versao)");
#else
    ESCREVAL("  (consulte /sys/class/dmi/id/ quando disponivel)");
#endif
    imprimir_separador();
}

FUNCAO VAZIO aba_graficos() {
    ESCREVAL("[ Graphics ]");
    ESCREVAL("  (deteccao de GPU nao implementada nesta versao)");
    imprimir_separador();
}

FUNCAO VAZIO aba_bench() {
    ESCREVAL("[ Bench ]");
    ESCREVAL("  (benchmark sintetico nao implementado nesta versao)");
    imprimir_separador();
}

FUNCAO VAZIO aba_about() {
    ESCREVAL("[ About ]");
    ESCREVAL("  BASIC-Z 1.0 -- inspirado em CPU-Z");
    ESCREVAL("  Escrito em BASICGOL (camada de macros C)");
    imprimir_separador();
}

INICIO
    ESCREVAL("  ____  _   _ ____  ____  _____    ____ ");
    ESCREVAL(" | __ )| | | / ___||  _ \\|__  /   |__  |");
    ESCREVAL(" |  _ \\| | | \\___ \\| |_) | / /      / / ");
    ESCREVAL(" | |_) | |_| |___) |  _ < / /_     / /_ ");
    ESCREVAL(" |____/ \\___/|____/|_| \\_\\____|   |____|");
    ESCREVAL("");
    ESCREVAL("BASIC-Z -- CPU-Z style system information (console)");
    ESCREVAL("");

    aba_cpu();
    aba_cache();
    aba_mainboard();
    aba_memoria();
    aba_graficos();
    aba_bench();
    aba_about();

    ESCREVAL("Concluido.");
FIM
