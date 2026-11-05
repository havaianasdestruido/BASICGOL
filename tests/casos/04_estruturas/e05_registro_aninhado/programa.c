
#include <basicgol.h>

REGISTRO(Endereco)
    TEXTO rua;
    INTEIRO numero;
FIM_REGISTRO(Endereco);

REGISTRO(Pessoa)
    TEXTO nome;
    Endereco onde;
FIM_REGISTRO(Pessoa);

INICIO
    Pessoa p;
    p.nome = "Ana";
    p.onde.rua = "Rua das Flores";
    p.onde.numero = 42;

    ESCREVA_TEXTO(p.nome);
    ESCREVA(" mora em ");
    ESCREVA_TEXTO(p.onde.rua);
    ESCREVA(", ");
    ESCREVAL(p.onde.numero);
FIM
