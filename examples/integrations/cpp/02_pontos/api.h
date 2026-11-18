#ifndef EXEMPLO_02_API_H
#define EXEMPLO_02_API_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct bg_ponto {
    double x;
    double y;
} bg_ponto;
double bg_distancia(const bg_ponto *a, const bg_ponto *b);
#ifdef __cplusplus
}
#endif
#endif
