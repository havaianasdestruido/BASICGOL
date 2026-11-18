#include <basicgol.h>
#include "api.h"

FUNCAO REAL bg_distancia(const bg_ponto *a, const bg_ponto *b)
{
    REAL dx = a->x - b->x;
    REAL dy = a->y - b->y;
    RETORNE RAIZ(dx * dx + dy * dy);
}
