#include "api.h"
#include <algorithm>
#include <iostream>

extern "C" int cpp_limita(int valor, int minimo, int maximo)
{
    return std::clamp(valor, minimo, maximo);
}

int main()
{
    std::cout << "BASICGOL chama std::clamp: "
              << bg_limita_via_cpp(42, 0, 10) << '\n';
    return 0;
}
