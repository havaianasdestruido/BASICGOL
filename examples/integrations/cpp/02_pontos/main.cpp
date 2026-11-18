#include "api.h"
#include <iomanip>
#include <iostream>

int main()
{
    const bg_ponto origem{0.0, 0.0};
    const bg_ponto destino{3.0, 4.0};
    std::cout << std::fixed << std::setprecision(1)
              << "Distancia calculada em BASICGOL: "
              << bg_distancia(&origem, &destino) << '\n';
    return 0;
}
