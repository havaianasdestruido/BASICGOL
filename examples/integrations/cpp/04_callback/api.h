#ifndef EXEMPLO_04_API_H
#define EXEMPLO_04_API_H
#ifdef __cplusplus
extern "C" {
#endif
typedef int (*bg_callback)(int valor);
int bg_aplica_callback(int valor, bg_callback callback);
#ifdef __cplusplus
}
#endif
#endif
