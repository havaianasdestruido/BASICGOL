#include "api.h"
#include <iostream>
#include <string>

int main()
{
    const std::string palavra = u8"ação";
    std::cout << "Bytes UTF-8 de '" << palavra << "': "
              << bg_tamanho_em_bytes(palavra.c_str()) << '\n';
    return 0;
}
