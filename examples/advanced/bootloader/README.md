# Bootloader "Olá Mundo!" em BASICGOL puro

Exemplo freestanding (sem libc) que exibe "Ola Mundo!" no buffer de texto VGA.

## Build (GCC cross, i686-elf)

```sh
i686-elf-gcc -ffreestanding -nostdlib -I ../../../include \
  bootloader.c -o bootloader.elf -T linker.ld
```

## Notas
- Requer toolchain freestanding (i686-elf-gcc) e um linker script.
- Sem biblioteca padrão; apenas macros BASICGOL e acesso direto ao hardware.
