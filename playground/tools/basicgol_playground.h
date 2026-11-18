/*
 * basicgol_playground.h — declarações que a libc do playground (wasi-libc
 * 2019, a que roda no navegador) não traz, mas a biblioteca BASICGOL usa.
 *
 * As definições estão em src/shim.c. O compilador recebe este arquivo com
 * `-include`, antes de qualquer fonte — assim `filesystem.c` compila sem o
 * aviso de "implicit declaration" e sem alterar o código do repositório.
 */
#ifndef BASICGOL_PLAYGROUND_H
#define BASICGOL_PLAYGROUND_H

int mkstemp(char *modelo);

#endif /* BASICGOL_PLAYGROUND_H */
