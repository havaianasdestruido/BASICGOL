#include <basicgol.h>

// W-04: o corpo do PARA mexe na propria variavel de controle
INICIO
    INTEIRO i = 0;
    PARA (i, 0, 9) {
        SE i > 3 ENTAO
            PARE;
        FIM_SE
        i = i + 1;
    }
FIM
