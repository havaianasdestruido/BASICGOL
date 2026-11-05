
// registro e passado por valor: a copia nao altera o original
#include <basicgol.h>

REGISTRO(Conta)
    INTEIRO saldo;
FIM_REGISTRO(Conta);

PROCEDIMENTO deposita(Conta c, INTEIRO valor) {
    c.saldo = c.saldo + valor;
    ESCREVAL(c.saldo);
}

INICIO
    Conta minha;
    minha.saldo = 100;
    deposita(minha, 50);
    ESCREVAL(minha.saldo);      // continua 100
FIM
