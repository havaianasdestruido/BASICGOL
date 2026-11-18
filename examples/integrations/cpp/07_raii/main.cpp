#include "api.h"
#include <iostream>
#include <memory>

struct BgDeleter {
    void operator()(char *texto) const noexcept { bg_libera_texto(texto); }
};

int main()
{
    std::unique_ptr<char, BgDeleter> copia(bg_duplica_texto("gerenciado por RAII"));
    if (!copia) {
        std::cerr << "Falha ao alocar texto\n";
        return 1;
    }
    std::cout << "C++ libera o buffer BASICGOL automaticamente: " << copia.get() << '\n';
    return 0;
}
