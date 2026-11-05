// ESCOLHA dentro de PARA. PARE sai só do ESCOLHA (o laço continua);
// PROSSIGA pula para o próximo i do laço, sem chegar ao código após o ESCOLHA.
#include <basicgol.h>

INICIO
    INTEIRO i = 0;
    INTEIRO total = 0;

    PARA (i, 0, 5) {
        ESCOLHA (i)
            CASO 0:
                PROSSIGA;            // i = 0: pula o +1000 abaixo
            CASO 1:
                total = total + 1;
                PARE;                // sai do ESCOLHA, não do PARA
            CASO 2:
            CASO 3:                  // rótulos vazios agrupados
                total = total + 10;
                PARE;
            CONTRARIO:
                total = total + 100;
        FIM_ESCOLHA
        total = total + 1000;        // i = 1..5
    }
    ESCREVAL(total);                 // 5221
FIM
