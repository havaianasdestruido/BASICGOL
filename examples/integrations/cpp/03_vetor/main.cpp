#include "api.h"
#include <array>
#include <iostream>

int main()
{
    const std::array<int, 5> valores{2, 4, 6, 8, 10};
    std::cout << "Soma do std::array em BASICGOL: "
              << bg_soma_vetor(valores.data(), static_cast<int>(valores.size()))
              << '\n';
    return 0;
}
