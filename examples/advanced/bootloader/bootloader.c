// bootloader.c -- Bootloader freestanding exibindo "Ola Mundo!" em BASICGOL.
// Compilado como ELF/binario freestanding; nao usa libc.
#include <basicgol.h>

// Endereco do buffer de texto VGA (80x25, 16 cores).
#define VGA ((volatile unsigned short *)0xB8000)

FUNCAO VAZIO imprimir_vga(const char *s) {
    volatile unsigned short *p = VGA;
    ENQUANTO (*s) {
        *p = (unsigned short)(*s) | (0x0F << 8);
        p++;
        s++;
    } FIM_ENQUANTO
}

// Ponto de entrada freestanding.
INICIO
    imprimir_vga("Ola Mundo!");
    // Loop infinito.
    ENQUANTO (1) { } FIM_ENQUANTO
FIM
