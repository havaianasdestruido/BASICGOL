/* cpp_inclusao.cpp — os cabeçalhos de módulo são usáveis de C++.
 *
 * Os módulos declaram `extern "C"`, então prometem ligação de C; até a Fase 6
 * isso não valia, porque `consola.h` usava `_Noreturn`, que não existe em C++.
 * Este teste compila como C++, liga com build/libbasicgol.a e roda.
 *
 * O cabeçalho da LINGUAGEM (`basicgol.h`) continua sendo só C (D-15): ele usa
 * `_Generic`, e o runner confere que a inclusão em C++ falha com mensagem —
 * o teste de compatibilidade não pode fingir sucesso.
 *
 * Compilar: c++ -std=c++17 -Wall -Wextra -Werror -I include \
 *              tests/portabilidade/cpp_inclusao.cpp build/libbasicgol.a -lm
 */
#include <basicgol/biblioteca.h>

#include <cstdio>
#include <cstring>

int main()
{
    char destino[8] = {0};
    double valor = 0.0;
    unsigned sorteado = 99u;

    if (!bg_e_digito('5') || bg_e_letra('5')) {
        return 1;
    }
    if (!bg_copia_texto(destino, sizeof destino, "cpp")) {
        return 1;
    }
    if (std::strcmp(destino, "cpp") != 0) {
        return 1;
    }
    if (bg_divide_i(9, 4, "cpp", 1) != 2) {
        return 1;
    }
    if (!bg_texto_para_real("2.5", &valor) || valor != 2.5) {
        return 1;
    }
    if (!bg_sorteia_ate(1u, &sorteado) || sorteado != 0u) {
        return 1;
    }
    if (bg_tamanho_texto("abcd") != 4u) {
        return 1;
    }
    std::printf("cpp: ok\n");
    return 0;
}
