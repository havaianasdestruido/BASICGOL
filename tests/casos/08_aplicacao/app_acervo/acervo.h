/* acervo.h — tipos e API do pequeno sistema de acervo de livros.
 *
 * Este caso exercita a linguagem "de verdade": vários arquivos, registro,
 * enumeração, vetor de registro, texto fixo, estático, E/S formatada,
 * laços, ESCOLHA, conversões de texto e matemática. */
#ifndef ACERVO_H
#define ACERVO_H

#include <basicgol.h>

/* Constantes de compilação. Um enumerador é expressão constante de verdade
 * (serve como tamanho de vetor); um CONSTANTE viraria VLA — ver C-23/W-07. */
ENUMERACAO(Constantes)
    ACERVO_MAX = 6,
    TITULO_MAX = 40,
    AUTOR_MAX = 24
FIM_ENUMERACAO(Constantes);

ENUMERACAO(Situacao)
    DISPONIVEL,
    EMPRESTADO
FIM_ENUMERACAO(Situacao);

REGISTRO(Livro)
    INTEIRO id;
    TEXTO_FIXO(titulo, TITULO_MAX);
    TEXTO_FIXO(autor, AUTOR_MAX);
    INTEIRO ano;
    Situacao situacao;
FIM_REGISTRO(Livro);

EXTERNO PROCEDIMENTO cadastra(INTEIRO id, TEXTO titulo, TEXTO autor, INTEIRO ano);
EXTERNO FUNCAO INTEIRO busca(INTEIRO id);
EXTERNO FUNCAO LOGICO empresta(INTEIRO id);
EXTERNO FUNCAO LOGICO devolve(INTEIRO id);
EXTERNO PROCEDIMENTO ordena_por_ano(void);
EXTERNO FUNCAO INTEIRO quantidade(void);
EXTERNO FUNCAO INTEIRO emprestados(void);
EXTERNO FUNCAO REAL media_ano(void);
EXTERNO FUNCAO INTEIRO indice_raridade(REAL fator);
EXTERNO FUNCAO TEXTO nome_situacao(Situacao s);

/* Acessores: o vetor e o contador sao privados do modulo (ESTATICO); o resto do
 * programa so enxerga estes getters. */
EXTERNO FUNCAO INTEIRO acervo_id(INTEIRO i);
EXTERNO FUNCAO TEXTO acervo_titulo(INTEIRO i);
EXTERNO FUNCAO TEXTO acervo_autor(INTEIRO i);
EXTERNO FUNCAO INTEIRO acervo_ano(INTEIRO i);
EXTERNO FUNCAO Situacao acervo_situacao(INTEIRO i);
EXTERNO PROCEDIMENTO relatorio(REAL fator);

#endif
