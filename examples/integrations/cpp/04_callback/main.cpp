#include "api.h"
#include <iostream>

extern "C" int cpp_triplica(int valor)
{
    return valor * 3;
}

int main()
{
    std::cout << "Callback C++ via BASICGOL: "
              << bg_aplica_callback(14, cpp_triplica) << '\n';
    return 0;
}
